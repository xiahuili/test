#include <linux/types.h>
#include <asm/types.h>
#include <inttypes.h>
#include <sys/file.h>
#include <sys/user.h>
#include <signal.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "route.h"


#define MAX_WAIT_TIME  (10)
#define EXECUTE_MAX_NUM  (7)

HASH_COLL_TABLE  *g_ptArbitratHashLink[HASH_MAX_ITEM];
HASH_CACHE_TABLE *g_ptHashCache[HASH_MAX_ITEM];
u_int g_tCorrectValue = 1;
u_int g_tPrintDebug = 0;
pthread_rwlock_t rwlock;

u_int Item_HashFind(HASH_KEY* ptKey, int* pIndex);
u_int  Item_Send_CacheAdd(HASH_COLL_TABLE* pstHashInfo, bool Flag);

#if _RD("读取配置文件")

/************************************************************************************
	name          ：GetCorrect_StringValue()
	function      ：获取配置文件纠正模式值
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int GetCorrect_StringValue(FILE* fp, char* SectionName, char* KeyName, char* ptvalue, uint8_t len)
{
	char TitleContent[128] = { 0 };
	char KeyContent[128] = { 0 };
	u_int KeyContLen = 0;
	u_int ValuePositionFlag = 0;
	char KeyValue[128] = { 0 };

	if ((SectionName == NULL) || (KeyName == NULL) || (fp == NULL))
	{
		printf("input parameter(s) is NULL!\n");
		return 1;
	}

	while (feof(fp) == 0)
	{
		memset(TitleContent, 0, 128);
		/*匹配段名*/
		if (fgets(TitleContent, 128, fp) == NULL)
		{
			printf("fgets  is null\n");
		}


		if (TitleContent[0] == ';')
		{
			continue;
		}


		if (strncasecmp(SectionName, TitleContent, strlen(SectionName)) == 0)
		{
			while (feof(fp) == 0)
			{
				memset(KeyContent, 0, 128);
				//memset(KeyContentBak, 0, sizeof(KeyContentBak));

				fgets(KeyContent, 128, fp);

				if (KeyContent[0] == ';')
				{
					continue;
				}

				if (strncasecmp(KeyName, KeyContent, strlen(KeyName)) == 0)
				{
					KeyContLen = strlen(KeyContent);
					for (ValuePositionFlag = strlen(KeyName); ValuePositionFlag <= KeyContLen; ValuePositionFlag++)
					{
						if (KeyContent[ValuePositionFlag] == ' ')
						{
							continue;
						}

						if (KeyContent[ValuePositionFlag] == '=')
						{
							break;
						}
					}

					ValuePositionFlag = ValuePositionFlag + 1;
				}

				strcpy(KeyValue, KeyContent + ValuePositionFlag);

				/*去掉多余的符号*/
				for (ValuePositionFlag = 0; ValuePositionFlag < strlen(KeyValue); ValuePositionFlag++)
				{
					if (KeyValue[ValuePositionFlag] == '\r' || KeyValue[ValuePositionFlag] == '\n' || KeyValue[ValuePositionFlag] == '\0')
					{
						KeyValue[ValuePositionFlag] = '\0';
						break;
					}
				}

				strncpy(ptvalue, KeyValue, len - 1);
				break;
			}
		}


		break;
	}
	return 0;
}

/************************************************************************************
	name          ：GetCorrect_IntValue()
	function      ：获取配置文件纠正模式值
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int GetCorrect_IntValue(FILE * fp, char* SectionName, char* KeyName, uint8_t * value, uint8_t len)
{
	char KeyValue[128] = { 0 };
	if ((SectionName == NULL) || (KeyName == NULL) || (fp == NULL))
	{
		printf("input parameter(s) is NULL!\n");
		return 1;
	}

	GetCorrect_StringValue(fp, SectionName, KeyName, KeyValue, len);

	if ((KeyValue[0] == '\0') || (KeyValue[0] == ';'))
	{
		return 0;
	}
	else
	{
		*value = atoi(KeyValue);
	}

	return 0;
}

/************************************************************************************
	name          ：GetCorrectValue()
	function      ：获取配置文件纠正模式值
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int GetCorrectValue(int* ptValue)
{
	u_int Ret = 0;
	uint8_t CorrectValue = 0;
	FILE* fp;
	char* ptHomePath = NULL;
	char stWholePath[128] = { 0 };
	char s1[128] = { "[correct]" };
	char s2[128] = { "correct_mode" };

	//读取配置文件中裁决装置模式
	ptHomePath = (char*)getenv("HOME");
	snprintf(stWholePath, sizeof(stWholePath) * 128, "%s/route/%s", ptHomePath, "correct.cfg");
	fp = fopen(stWholePath, "r");

	Ret = GetCorrect_IntValue(fp, s1, s2, &CorrectValue, 128);
	if (Ret)
	{
		printf("GetCorrect_IntValue  error\n");
		return Ret;
	}

	*ptValue = CorrectValue;

	return 0;

}

#endif

#if _RD("打印")

/************************************************************************************
	name          ：RouteItem_LinkSrh()
	function      ：hash链表打印
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void RouteItem_LinkSrh(int *dstIp, int mask)
{
	u_int ret = 0;
	u_int index = 0;
	u_int collindex = 0;
	HASH_KEY   stKey;
	HASH_COLL_TABLE *ptItem = NULL;

	//stKey.DstIp = dstIp;
	memset(&stKey,0,sizeof(HASH_KEY));

	memcpy(stKey.DstIp, dstIp,4);
	stKey.Mask = mask;
	ret = Item_HashFind(&stKey, &index);

	ptItem = g_ptArbitratHashLink[index-1];
	while (ptItem)
	{
		printf("mask      =:%d\n", ptItem->HashKey.Mask);
		if (ptItem->HashItm.DstLen == 4)
		{
			printf("dstAddr   =:0x%08x\n", ptItem->HashKey.DstIp[0]);
			printf("NextHop   =:0x%08x\n", ptItem->HashItm.NextHop[0]);
		}
		
		if (ptItem->HashItm.DstLen == 16)
		{
			printf("dstAddr   =:0x%08x 0x%08x 0x%08x 0x%08x\n", ptItem->HashKey.DstIp[0], ptItem->HashKey.DstIp[1], ptItem->HashKey.DstIp[2], ptItem->HashKey.DstIp[3]);
			printf("NextHop   =:0x%08x 0x%08x 0x%08x 0x%08x\n", ptItem->HashItm.NextHop[0], ptItem->HashItm.NextHop[1], ptItem->HashItm.NextHop[2], ptItem->HashItm.NextHop[3]);
		}
		printf("ifid      =:%d\n", ptItem->HashItm.InterfaceIndex);
		printf("MngIp     =:0x%08x\n", ptItem->HashItm.MngIp);	
		printf("OpType    =:%d\n", ptItem->HashItm.OpType);

		ptItem = ptItem->pNext;

	}
	printf("--------------------------------------\n");

}


/************************************************************************************
	name          RouteItem_CacheSrh()
	function      ：hash缓存打印
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void RouteItem_CacheSrh(int *dstIp, int mask)
{
	u_int ret = 0;
	u_int index = 0;
	u_int collindex = 0;
	HASH_KEY   stKey;
	HASH_CACHE_TABLE *ptItem = NULL;

	//stKey.DstIp = dstIp;
	memset(&stKey,0,sizeof(HASH_KEY));

	memcpy(stKey.DstIp, dstIp,4);
	stKey.Mask = mask;
	ret = Item_HashFind(&stKey, &index);

	ptItem = g_ptHashCache[index-1];
	while (ptItem)
	{
		printf("mask      =:%d\n", ptItem->HashKey.Mask);
		if (ptItem->HashItm.DstLen == 4)
		{
			printf("dstAddr   =:0x%08x\n", ptItem->HashKey.DstIp[0]);
			printf("NextHop   =:0x%08x\n", ptItem->HashItm.NextHop[0]);
		}
		
		if (ptItem->HashItm.DstLen == 16)
		{
			printf("dstAddr   =:0x%08x 0x%08x 0x%08x 0x%08x\n", ptItem->HashKey.DstIp[0], ptItem->HashKey.DstIp[1], ptItem->HashKey.DstIp[2], ptItem->HashKey.DstIp[3]);
			printf("NextHop   =:0x%08x 0x%08x 0x%08x 0x%08x\n", ptItem->HashItm.NextHop[0], ptItem->HashItm.NextHop[1], ptItem->HashItm.NextHop[2], ptItem->HashItm.NextHop[3]);
		}
		printf("ifid      =:%d\n", ptItem->HashItm.InterfaceIndex);
		printf("OpType    =:%d\n", ptItem->HashItm.OpType);

		ptItem = ptItem->pNext;

	}
	printf("----------------cache end----------------------\n");

}

/************************************************************************************
	name          ：Ttem_NormalRouterPrintf()
	function      ：正常路由信息打印
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void Ttem_NormalRouterPrintf(HASH_COLL_TABLE *ptNormalRouter)
{

	printf("----------------------the normal router------------------\n");
	printf("DstIp =              :0x%08x\n", ptNormalRouter->HashKey.DstIp[0]);
	printf("Mask  =              :%d\n", ptNormalRouter->HashKey.Mask);
	printf("MngIp =              :0x%08x\n", ptNormalRouter->HashItm.MngIp);
	printf("OpType =             :%d\n", ptNormalRouter->HashItm.OpType);
	printf("NextHop =            :0x%08x\n", ptNormalRouter->HashItm.NextHop[0]);
	printf("InterfaceIndex =     :%d\n", ptNormalRouter->HashItm.InterfaceIndex);
	printf("----------------------the normal end---------------------\n");

}

/************************************************************************************
	name          ：Ttem_AbnormalRouterPrintf()
	function      ：异常路由信息打印
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void Ttem_AbnormalRouterPrintf(HASH_COLL_TABLE stAbnormalRouter[], int num)
{
	u_int loop = 0;

	printf("----------------------the abnormal router-----------------\n");
	for (loop = 0; loop < num; loop++)
	{
		printf("DstIp =           :0x%08x\n", stAbnormalRouter[loop].HashKey.DstIp[0]);
		printf("Mask =            :%d\n", stAbnormalRouter[loop].HashKey.Mask);
		printf("MngIp =           :0x%08x\n", stAbnormalRouter[loop].HashItm.MngIp);
		printf("OpType =          :%d\n", stAbnormalRouter[loop].HashItm.OpType);
		printf("NextHop =         :0x%08x\n", stAbnormalRouter[loop].HashItm.NextHop[0]);
		printf("InterfaceIndex =  :%d\n", stAbnormalRouter[loop].HashItm.InterfaceIndex);
	}

	printf("----------------------the abnormal end-------------------\n");
}

#endif

#if _RD("hash")

/************************************************************************************
	name          ：Item_HashInit()
	function      ：hash表初始化
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_HashInit()
{
	u_int i = 0;
	
	for (i = 0; i < HASH_MAX_ITEM; i++)
	{
		g_ptArbitratHashLink[i] = NULL;
		g_ptHashCache[i] = NULL;
	}

	return 0;
}


/************************************************************************************
	name          ：HashComputer()
	function      ：裁决队列hash值计算
	inparameter   ：ptKey
	outparameter  ：index
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int HashComputer(HASH_KEY* ptKey)
{
	u_int ret = 0;
	u_int len = 0;
	u_int i = 0;
	u_int index = 0;
	uint8_t key[20] = {0};
#if 0
	memcpy(key, ptKey, sizeof(HASH_KEY));

	len = sizeof(key);
	index = key[0];
	for (i = 1; i < len; ++i)
	{
		index *= 1103515245 + key[i];
	}

	index >>= 27;
	index &= (HASH_MAX_ITEM - 1);
#endif

	index = ptKey->DstIp[0] + ptKey->DstIp[1] + ptKey->DstIp[2] + ptKey->DstIp[3] + ptKey->Mask;
	index ^= (index >> 16);
	index ^= (index >> 8);
	index ^= (index >> 3);
	index &= (HASH_MAX_ITEM - 1);

	return index;
}

/************************************************************************************
	name          ：Item_HashFind()
	function      ：裁决队列hash查找
	inparameter   ：ptKey
	outparameter  ：pIndex
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_HashFind(HASH_KEY* ptKey, int* pIndex)
{
	u_int ret = 0;
	u_int Index = 0;

	Index = HashComputer(ptKey);

	if (Index <= 0)
	{
		printf("HashComputer failed  \n");
		return 1;
	}

	if (NULL != g_ptArbitratHashLink[Index])
	{
		//printf("index has been used\n");
	}
	else
	{
		//printf("index has not been used\n");
	}

	*pIndex = Index;

	return 0;
}


/************************************************************************************
	name          ：Item_HashAdd()
	function      ：裁决队列hash添加
	inparameter   ：ptHashInfo
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int  Item_HashAdd(HASH_COLL_TABLE *ptHashInfo, int CollIndex)
{
	u_int ret = 0;
	u_int index = 0;
	u_int DstAddr[4] = { 0 };
	u_int DstMask = 0;
	
	HASH_COLL_TABLE *ptHashLink = NULL;
	HASH_COLL_TABLE* ptCurrHashLink = NULL;
	HASH_COLL_TABLE  *ptItemCache = NULL;            
	HASH_KEY   *ptItemKey = NULL;

	ptHashLink = g_ptArbitratHashLink[CollIndex-1];

	ptItemCache = (HASH_COLL_TABLE *)malloc(sizeof(HASH_COLL_TABLE));
	if (NULL == ptItemCache)
	{
		printf("ptItemCache  malloc failed\n");
		return 1;
	}
	memcpy(ptItemCache, ptHashInfo, sizeof(HASH_COLL_TABLE));
	if (NULL == g_ptArbitratHashLink[CollIndex-1])
	{
		ptHashLink = ptItemCache;
		ptHashLink->pNext = NULL;
		g_ptArbitratHashLink[CollIndex-1] = ptHashLink;	
	}
	else
	{
		while (ptHashLink)
		{
			memcpy(DstAddr, ptHashLink->HashKey.DstIp,sizeof(DstAddr));
			DstMask = ptHashLink->HashKey.Mask;
			if ((0 == memcmp(ptHashInfo->HashKey.DstIp, DstAddr, sizeof(DstAddr))) &&
				(ptHashInfo->HashKey.Mask == DstMask) &&
				(ptHashInfo->HashItm.MngIp == ptHashLink->HashItm.MngIp))
			{
				memcpy(ptHashLink, ptHashInfo,sizeof(HASH_COLL_TABLE));
				//RouteItem_LinkSrh(ptHashInfo->HashKey.DstIp, ptHashInfo->HashKey.Mask); 
				return 0;
			}
			else
			{
				ptCurrHashLink = ptHashLink;
				ptHashLink = ptHashLink->pNext;
			}

		}
	
		ptCurrHashLink->pNext = ptItemCache;
		ptHashLink = ptItemCache;
		ptHashLink->pNext = NULL;
	}
	
	
	RouteItem_LinkSrh(ptHashInfo->HashKey.DstIp, ptHashInfo->HashKey.Mask);

	return 0;

}

/************************************************************************************
	name          ：Item_HashDel()
	function      ：裁决队列hash删除
	inparameter   ：ptHashDelInfo
	outparameter  ：ptNewHead
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int  Item_HashDel(HASH_COLL_TABLE *ptHashDelInfo, HASH_COLL_TABLE **ptNewHead,int Index)
{
	HASH_COLL_TABLE* ptHashInfo = NULL;
	HASH_COLL_TABLE* ptHead = NULL;
	HASH_COLL_TABLE* ptHashPre = NULL;

	ptHead = g_ptArbitratHashLink[Index];
	ptHashInfo = ptHead->pNext;
	ptHashPre = ptHead;
	
	/*删除链表中相同的元素*/
	while (ptHashInfo)
	{
		if ((0 == memcmp(ptHashInfo->HashKey.DstIp, ptHashDelInfo->HashKey.DstIp,4 * sizeof(u_int))) &&
			(ptHashInfo->HashKey.Mask == ptHashDelInfo->HashKey.Mask))
		{
			ptHashPre->pNext = ptHashInfo->pNext;
			free(ptHashInfo);
			ptHashInfo = ptHashPre->pNext;
		}
		else
		{
			ptHashPre = ptHashInfo;
			ptHashInfo = ptHashInfo->pNext;
		}
	}

	if (0 == memcmp(ptHashDelInfo, g_ptArbitratHashLink[Index],sizeof(HASH_COLL_TABLE)))
	{
		*ptNewHead = ptHead->pNext;		
		free(ptHashDelInfo);
		g_ptArbitratHashLink[Index] = ptHead->pNext;
	}
	
	return 0;
}

#endif

#if _RD("缓存hash")

/************************************************************************************
	name          ：Item_CacheHashFind()
	function      ：hash查找
	inparameter   ：ptDelHashInfo
	outparameter  ：ptCurrentItem
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_CacheHashFind(HASH_CACHE_TABLE* ptDelHashInfo, HASH_CACHE_TABLE* ptCurrentItem,u_int *pIndex, u_int* ptArgOut,u_int* ptItemArgOut)
{
	u_int Index = 0;
	HASH_CACHE_TABLE* ptHead = NULL;
	HASH_CACHE_TABLE* ptLink = NULL;

	Index = HashComputer(&ptDelHashInfo->HashKey);
	if (NULL == g_ptHashCache[Index-1])
	{
		*ptArgOut = HASH_INDEX_NOT_FIND;
	}
	else
	{
		ptLink = g_ptHashCache[Index-1];

		while (ptLink)
		{
			if (0 == memcmp(ptDelHashInfo, ptLink, sizeof(HASH_CACHE_TABLE)))
			{
				*ptItemArgOut = HASH_INDEX_FIND;
			}
			else if ((0 == memcmp(ptLink->HashKey.DstIp, ptDelHashInfo->HashKey.DstIp, 4 * sizeof(u_int))) &&
				(ptLink->HashKey.Mask == ptDelHashInfo->HashKey.Mask) &&
				ptLink->HashItm.NormalFlag == 1 &&
				(ptLink->HashItm.OpType == ptDelHashInfo->HashItm.OpType))
			{
				memcpy(ptCurrentItem, ptLink, sizeof(HASH_CACHE_TABLE));
				*ptArgOut = HASH_INDEX_FIND;
			}
			ptLink = ptLink->pNext;

		}	
	}

	*pIndex = Index;
	return 0;
}

/************************************************************************************
	name          ：Item_CacheHashAdd()
	function      ：添加路由回收缓存
	inparameter   ：ptHashInfo
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int  Item_CacheHashAdd(HASH_CACHE_TABLE* ptHashInfo, int CollIndex,bool Flag)
{
	u_int ret = 0;
	u_int index = 0;
	u_int DstAddr[4] = { 0 };
	u_int DstMask = 0;

	HASH_CACHE_TABLE* ptHashLink = NULL;
	HASH_CACHE_TABLE* ptItemCache = NULL;
	HASH_CACHE_TABLE* ptCurrHashLink = NULL;
	HASH_KEY* ptItemKey = NULL;

	ptHashLink = g_ptHashCache[CollIndex-1];
	
	ptItemCache = (HASH_CACHE_TABLE*)malloc(sizeof(HASH_CACHE_TABLE));
	if (NULL == ptItemCache)
	{
		printf("ptItemCache  malloc failed\n");
		return 1;
	}
	memcpy(ptItemCache, ptHashInfo, sizeof(HASH_CACHE_TABLE));
	if (NULL == g_ptHashCache[CollIndex-1])
	{
		ptHashLink = ptItemCache;
		ptHashLink->pNext = NULL;
		g_ptHashCache[CollIndex-1] = ptHashLink;
	}
	else
	{
		while (ptHashLink)
		{
			if (false == Flag)
			{	
				if (0 == memcmp(ptHashInfo, ptHashLink, sizeof(HASH_CACHE_TABLE)))
				{					
					return 0;
				}
				else
				{
					ptCurrHashLink = ptHashLink;
					ptHashLink = ptHashLink->pNext;
				}
			}
			else
			{
				memcpy(DstAddr, ptHashLink->HashKey.DstIp, sizeof(DstAddr));
				DstMask = ptHashLink->HashKey.Mask;
				if ((0 == memcmp(ptHashInfo->HashKey.DstIp, DstAddr, sizeof(DstAddr))) &&
					(ptHashInfo->HashKey.Mask == DstMask))
				{
					memcpy(ptHashLink, ptHashInfo, sizeof(HASH_CACHE_TABLE));
					return 0;
				}
				else
				{
					ptCurrHashLink = ptHashLink;
					ptHashLink = ptHashLink->pNext;
				}
			}

		}

		ptCurrHashLink->pNext = ptItemCache;
		ptHashLink = ptItemCache;
		ptHashLink->pNext = NULL;
	}

    //RouteItem_CacheSrh(ptHashInfo->HashKey.DstIp, ptHashInfo->HashKey.Mask);

	return 0;

}

/************************************************************************************
	name          ：Item_CacheHashDel()
	function      ：删除路由回收缓存
	inparameter   ：ptHashDelInfo
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_CacheHashDel(HASH_CACHE_TABLE* ptHashDelInfo,int CollIndex)
{
	HASH_CACHE_TABLE* ptHashInfo = NULL;
	HASH_CACHE_TABLE* ptHead = NULL;
	HASH_CACHE_TABLE* ptHashPre = NULL;

	ptHead = g_ptHashCache[CollIndex-1];
	ptHashInfo = ptHead->pNext;
	ptHashPre = ptHead;

	/*删除链表中相同的元素*/
	while (ptHashInfo)
	{		
		if (0 == memcmp(ptHashInfo, ptHashDelInfo,sizeof(HASH_CACHE_TABLE)))
		{
			ptHashPre->pNext = ptHashInfo->pNext;
			free(ptHashInfo);
			ptHashInfo = ptHashPre->pNext;
		}
		else
		{
			ptHashPre = ptHashInfo;
			ptHashInfo = ptHashInfo->pNext;
		}
	}

	if (0 == memcpy(ptHashDelInfo, g_ptHashCache[CollIndex-1],sizeof(HASH_CACHE_TABLE)))
	{
		free(ptHashDelInfo);
		g_ptHashCache[CollIndex-1] = ptHead->pNext;
	}

    RouteItem_CacheSrh(ptHashDelInfo->HashKey.DstIp, ptHashDelInfo->HashKey.Mask);
	return 0;
}

/************************************************************************************
	name          ：Item_CacheAdd()
	function      ：添加路由回收缓存
	inparameter   ：pstHashInfo
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_CacheAdd(HASH_CACHE_TABLE* pstHashInfo,bool Flag)
{
	u_int ret = 0;
	u_int collindex = 0;
	u_int stArgOut = HASH_INDEX_NOT_FIND;
	u_int stItemResult = HASH_INDEX_NOT_FIND;
	HASH_CACHE_TABLE stItemHashFind;

	memset(&stItemHashFind,0,sizeof(HASH_CACHE_TABLE));

	ret = Item_CacheHashFind(&pstHashInfo->HashKey,&stItemHashFind, &collindex,&stArgOut,&stItemResult);
	if (0 != ret)
	{
		printf("hash find error:  ret=   :%d   collindex=  :%d\n", ret, collindex);
		return ret;
	}
	else if ((0 == ret) && (collindex > 0))
	{
		ret = Item_CacheHashAdd(pstHashInfo, collindex,Flag);
		if (ret)
		{
			printf("Item_HashAdd failed \n");
			return ret;
		}

	}

	return 0;
}

#endif

#if _RD("裁决队列添加")

/************************************************************************************
	name          ：Item_QueueAdd()
	function      ：待裁决队列添加
	inparameter   ：ptHashInfo
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_QueueAdd(HASH_COLL_TABLE *ptHashInfo)
{
	u_int ret = 0;
	u_int collindex = 0;

#if 0
	void* arg = NULL;
	u_int i = 0;
	arg = (void*)ptHashInfo;

	for (i = 0; i < sizeof(HASH_COLL_TABLE); i++)
	{
		if (i == 0)
		{ 
			printf("0x%08x\n", *(u_int*)arg);
		}		
		else
		{
			if (i % 4 == 0)
			{
				printf("0x%08x\n", *(u_int*)(arg + i));
			}
		}

	}
#endif	
	//printf("dsip =   0x%08x\n", ptHashInfo->HashKey.DstIp[0]);
	//printf("mask =   %d\n", ptHashInfo->HashKey.Mask);

    /*添加待裁决队列*/
	ret = Item_HashFind(&(ptHashInfo->HashKey),&collindex);
	if (0 != ret)
	{
		printf("hash find error:  ret=   :%d   collindex=  :%d\n",ret,collindex);
		return ret;
	}	
	else if ((0 == ret) && (collindex >0))
	{				
		ret = Item_HashAdd(ptHashInfo, collindex);
		if (ret)
		{
			printf("Item_HashAdd failed \n");
			return ret;
		}		
			
	}
	
	return 0;
}


/************************************************************************************
	name          ：Itm_RouteMsgParse()
	function      ：解析netlink报文
	inparameter   ：buffer
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Itm_RouteMsgParse(char *buffer, int RouFrom,struct timeval *pTime)
{
	u_int ret = 0;
	u_int index;
	u_int table;
	u_int mask = 0;
	u_int i = 0;
	u_int len = 0;
	u_int collindex = 0;
	u_int timetamp = 0;
	u_int rtLen = 0;
	uint16_t ActionType = 0;
	u_int dest[4] = {0};
	u_int gate[4] = {0};
	bool IsAddFlag = false;
	unsigned short Dstlen = 0;
	struct nlmsghdr *h;
	fpm_msg_hdr_t  *pmsghead;
	void *buf = NULL;
	struct rtmsg *rtm = NULL;
	struct rtattr *rtAttr = NULL;
	struct rtattr *tb[RTA_MAX + 1];
	HASH_COLL_TABLE   ItemLink;
	HASH_COLL_TABLE   ItemCache = {0};
	
	memset(&tb, 0, sizeof tb);
	memset(&ItemLink,0,sizeof(HASH_COLL_TABLE));
	pmsghead = (fpm_msg_hdr_t *)buffer;
	buf = (void *)buffer;
	h = (void*)buf + 4;

	/*写锁定*/
	pthread_rwlock_wrlock(&rwlock);

	while(h->nlmsg_len)
	{
		len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));
		if (len < 0)
		{
			return -1;
		}
			
		switch (ntohs(h->nlmsg_type))
		{
		    case 0x1800:
			    ActionType = ROUTER_ADD;
			    break;

		    case 0x1900:
			    ActionType = ROUTER_DEL;
			    break;

		    default:
			    ActionType = ROUTER_UPDATE;
			    break;
		}
		
		rtm = NLMSG_DATA(h);
		rtAttr = RTM_RTA(rtm);
		rtLen = RTM_PAYLOAD(h);
		for (; RTA_OK(rtAttr, len); rtAttr = RTA_NEXT(rtAttr, len))
		{		
			Dstlen = *((unsigned short *)((void *)rtm + sizeof(struct rtmsg))) - 4;
			switch (rtAttr->rta_type)
			{
			    case RTA_DST:	
					if (Dstlen == 4)
					{
						dest[0] = *((int *)RTA_DATA(rtAttr));
					}

					if (Dstlen == 16)
					{
						memcpy(dest, RTA_DATA(rtAttr),sizeof(dest));
					}					
				    break;

				case RTA_GATEWAY:
					if (Dstlen == 4)
					{
						gate[0] = *((int *)RTA_DATA(rtAttr));
					}
					if (Dstlen == 16)
					{
						memcpy(gate, RTA_DATA(rtAttr), sizeof(gate));
					}					
				    break;

				case RTA_OIF:
					index = *((int *)RTA_DATA(rtAttr));
				    break;

			}
		}
		
				
		if (0 != dest[0])
		{
			ItemLink.HashKey.Mask = rtm->rtm_dst_len;
			if (Dstlen == 4)
			{
				ItemLink.HashKey.DstIp[0] = ntohl(dest[0]);
				ItemLink.HashItm.NextHop[0] =gate[0];
			}

			if (Dstlen == 16)
			{
				memcpy(ItemLink.HashKey.DstIp, dest,sizeof(dest));
				memcpy(ItemLink.HashItm.NextHop, gate, sizeof(gate));
			}

			ItemLink.HashItm.InterfaceIndex = index;
			ItemLink.HashItm.MngIp = RouFrom;
			ItemLink.HashItm.OpType = ActionType;
			ItemLink.HashItm.DstLen = Dstlen;
			ItemLink.HashItm.Time.TimeSec = pTime->tv_sec;
			ItemLink.HashItm.Time.TimeUSec = pTime->tv_usec;

			/*下发到转发面，添加回收缓存*/
			if (1 == g_tCorrectValue)
			{
				ret = Item_Send_CacheAdd(&ItemLink, IsAddFlag);

				ROUTER_ERROR_LOG("Itm_RouteMsgParse---->Item_Send_CacheAdd failed  ret =:%d\n", ret);
			}

			/*添加待裁决队列*/
			ret = Item_QueueAdd(&ItemLink);
			ROUTER_ERROR_LOG("Itm_RouteMsgParse---->Item_QueueAdd failed  ret =:%d\n", ret);

		}

		buf += h->nlmsg_len + 4;
		h = (void*)buf + 4;

	}

	/*写解锁*/
	pthread_rwlock_unlock(&rwlock);

	return 0;
}

/************************************************************************************
	name          ：Itm_RevMsgQueAdd()
	function      ：读取netlink报文
	inparameter   ：ArgIn
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void Itm_RevMsgQueAdd(void *ArgIn)
{
	u_int retval = 0;
	u_int Cli_soc = 0;
	u_int Length = 0;
	u_int MngIp = 0;
	char data[1024] = "";
	unsigned int t[1024] = { 0 };
	int i = 0;
	struct timeval  Time = { 0 };
	CLIENT_INFO  *ptClientInfo = NULL;

	struct sockaddr_in  ServerAddr;
	struct sockaddr_in  ClientAddr;


	ptClientInfo = (CLIENT_INFO *)ArgIn;

	while (1)
	{
		gettimeofday(&Time, NULL);

		memset(data, 0, 1024);
		
		/*读写数据*/

		retval = read(ptClientInfo->ClientSocket, data, sizeof(data) - 1);
		if (retval < 0)
		{
			printf("read error\n");
			return;
		}

		if (g_tPrintDebug)
		{
			for (i = 0; i < 256; i++)
			{
				t[i] = *(unsigned int*)(data + i * 4);
				if (i != 0)
				{
					if (i % 4 == 0)
					{
						printf("0x%08x 0x%08x 0x%08x 0x%08x\n", ntohl(t[i - 4]), ntohl(t[i - 3]), ntohl(t[i - 2]), ntohl(t[i - 1]));
					}
				}
			}
		}

		
		retval = Itm_RouteMsgParse(data, ptClientInfo->ManageIp, &Time);
		if (retval)
		{
			printf(" GetDstaddr  failed");
			return;
		}

	}
}

/************************************************************************************
	name          ：Itm_ArbitraQueAdd()
	function      ：建立socket
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void Itm_ArbitraQueAdd()
{
	u_int retval = 0;
	u_int Ser_soc = 0;
	u_int Cli_soc = 0;
	u_int Length = 0;
	u_int MngIp = 0;
	char data[1024] = "";
	unsigned int t[1024] = { 0 };
	int i = 0;
	struct timeval  Time = { 0 };
	pthread_t  RevMsg;
	CLIENT_INFO  Clientpthread = {0};
	
	struct sockaddr_in  ServerAddr;
	struct sockaddr_in  ClientAddr;
	
	memset(&ServerAddr, 0, sizeof(struct sockaddr_in));
	memset(&ClientAddr, 0, sizeof(struct sockaddr_in));
	
	/*建立socket*/
	Ser_soc = socket(AF_INET, SOCK_STREAM, 0);
	if (Ser_soc < 0)
	{
		printf("socket create error\n");
		return;
	}
	
	/*绑定ip地址和端口号*/
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(2620);
	ServerAddr.sin_addr.s_addr = inet_addr("192.168.3.7");

	int reuse = 1;
	if (-1 == setsockopt(Ser_soc, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
	{
		printf("setsockopt error\n");
		return;
	}

	Length = sizeof(struct sockaddr_in);
	retval = bind(Ser_soc, (struct sockaddr*) & ServerAddr, Length);
	if (retval < 0)
	{
		printf("bind error\n");
		return;
	}

	/*开始监听*/
	retval = listen(Ser_soc, 5);
	if (retval < 0)
	{
		printf("liset error \n");
		return;
	}
	
	while (1)
	{
		Cli_soc = accept(Ser_soc, (void*)& ClientAddr, &Length);
		if (Cli_soc < 0)
		{
			printf("accept error \n");
			return;
		}
		
		printf("client address: 0x%08x\n", ClientAddr.sin_addr.s_addr);
		printf("client sin_port: %d\n", ClientAddr.sin_port);

		MngIp = ntohl(ClientAddr.sin_addr.s_addr);
		Clientpthread.ClientSocket = Cli_soc;
		Clientpthread.ManageIp = MngIp;

		retval = pthread_create(&RevMsg, NULL, (void*)&Itm_RevMsgQueAdd, (void *)&Clientpthread);
		if (retval)
		{
			printf("pthread_create RevMsg failed \n");
		}

			
	}

	//pthread_join(RevMsg, NULL);
	
	//close(Ser_soc);
	//close(Cli_soc);
	//pthread_exit(0);
	return;
}

#endif

#if _RD("进行裁决")

/************************************************************************************
	name          ：Item_SendToForward()
	function      ：路由下发到转发面
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_SendToForward(ROUTER_ITM* ptForItem)
{
	return 0;
}
/************************************************************************************
	name          ：Item_Send_CacheAdd()
	function      ：添加路由回收缓存
	inparameter   ：pstHashInfo
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int  Item_Send_CacheAdd(HASH_COLL_TABLE* pstHashInfo, bool Flag)
{
	u_int ret = 0;
	ROUTER_ITM stForwardItem;
	HASH_CACHE_TABLE stCacheItm;

	memset(&stForwardItem, 0, sizeof(ROUTER_ITM));
	memset(&stCacheItm, 0, sizeof(HASH_CACHE_TABLE));

	/*下发到转发面*/
	stForwardItem.Mask = pstHashInfo->HashKey.Mask;
	memcpy(stForwardItem.DstIp, pstHashInfo->HashKey.DstIp, 4 * sizeof(u_int));

	stForwardItem.DstLen = pstHashInfo->HashItm.DstLen;
	stForwardItem.InterfaceIndex = pstHashInfo->HashItm.InterfaceIndex;
	stForwardItem.OpType = pstHashInfo->HashItm.OpType;
	memcpy(stForwardItem.NextHop, pstHashInfo->HashItm.NextHop, 4 * sizeof(u_int));

	ret = Item_SendToForward(&stForwardItem);
	ROUTER_ERROR_LOG("Item_Send_CacheAdd---->Item_SendToForward failed  ret =:%d\n", ret);

	/*添加路由回收缓存*/
	if ((ROUTER_DEL == pstHashInfo->HashItm.OpType) || (ROUTER_UPDATE == pstHashInfo->HashItm.OpType))
	{
		stCacheItm.HashKey.Mask = pstHashInfo->HashKey.Mask;
		memcpy(stCacheItm.HashKey.DstIp, pstHashInfo->HashKey.DstIp, 4 * sizeof(u_int));

		stCacheItm.HashItm.DstLen = pstHashInfo->HashItm.DstLen;
		stCacheItm.HashItm.InterfaceIndex = pstHashInfo->HashItm.InterfaceIndex;
		stCacheItm.HashItm.OpType = pstHashInfo->HashItm.OpType;
		memcpy(stCacheItm.HashItm.NextHop, pstHashInfo->HashItm.NextHop, 4 * sizeof(u_int));

		ret = Item_CacheAdd(&stCacheItm, Flag);
		ROUTER_ERROR_LOG("Item_Send_CacheAdd---->Item_CacheAdd failed  ret =:%d\n", ret);
	}

	return 0;
}

/************************************************************************************
	name          ：Item_SendAndRecovery()
	function      ：下发路由与恢复删除缓存
	inparameter   ：ptNormalItem,stAbnormalItem
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_SendAndRecovery(HASH_COLL_TABLE* ptNormalItem, HASH_COLL_TABLE stAbnormalItem[], int num, bool Flag)
{
	u_int ret = 0;
	u_int loop = 0;
	u_int CollIndex = 0;
	u_int FindResult = HASH_INDEX_NOT_FIND;
	u_int ItemFind = HASH_INDEX_NOT_FIND;
	ROUTER_ITM  stForwardItem;
	HASH_CACHE_TABLE stCacheItm;
	HASH_CACHE_TABLE stCacheItmPre;
	HASH_CACHE_TABLE stNorCacheItm;

	memset(&stForwardItem, 0, sizeof(ROUTER_ITM));
	memset(&stCacheItm, 0, sizeof(HASH_CACHE_TABLE));
	memset(&stCacheItmPre, 0, sizeof(HASH_CACHE_TABLE));
	memset(&stNorCacheItm, 0, sizeof(HASH_CACHE_TABLE));

	/*添加路由回收缓存*/
	if (1 == g_tCorrectValue)
	{
		for (loop = 0; loop < num; loop++)
		{
			if ((ROUTER_DEL == stAbnormalItem[loop].HashItm.OpType) || (ROUTER_UPDATE == stAbnormalItem[loop].HashItm.OpType))
			{
				ret = Item_CacheHashFind(&stAbnormalItem[loop], &stCacheItmPre, &CollIndex, &FindResult,&ItemFind);
				if ((ret == 0) && (FindResult == HASH_INDEX_FIND))
				{
					/*将stCacheItemPre 下发到转发面*/
					stForwardItem.Mask = stCacheItmPre.HashKey.Mask;
					memcpy(stForwardItem.DstIp, stCacheItmPre.HashKey.DstIp, 4 * sizeof(u_int));

					stForwardItem.DstLen = stCacheItmPre.HashItm.DstLen;
					stForwardItem.InterfaceIndex = stCacheItmPre.HashItm.InterfaceIndex;
					stForwardItem.OpType = stCacheItmPre.HashItm.OpType;
					memcpy(stForwardItem.NextHop, stCacheItmPre.HashItm.NextHop, 4 * sizeof(u_int));

					ret = Item_SendToForward(&stForwardItem);
					ROUTER_ERROR_LOG("Item_SendAndRecovery---->Item_SendToForward failed  ret =:%d\n", ret);
				}
			
				
				/*删除缓存*/
				if (ItemFind == HASH_INDEX_FIND)
				{
					stCacheItm.HashKey.Mask = stAbnormalItem[loop].HashKey.Mask;
					memcpy(stCacheItm.HashKey.DstIp, stAbnormalItem[loop].HashKey.DstIp, 4 * sizeof(u_int));

					stCacheItm.HashItm.DstLen = stAbnormalItem[loop].HashItm.DstLen;
					stCacheItm.HashItm.InterfaceIndex = stAbnormalItem[loop].HashItm.InterfaceIndex;
					stCacheItm.HashItm.OpType = stAbnormalItem[loop].HashItm.OpType;
					memcpy(stCacheItm.HashItm.NextHop, stAbnormalItem[loop].HashItm.NextHop, 4 * sizeof(u_int));

					ret = Item_CacheHashDel(&stCacheItm, CollIndex);
					ROUTER_ERROR_LOG("Item_SendAndRecovery---->Item_CacheHashDel failed  ret =:%d\n", ret);
				}
			}
		}
		if (NULL != ptNormalItem)
		{
			/*正常的覆盖前次的路由*/
			if ((ROUTER_DEL == ptNormalItem->HashItm.OpType) || (ROUTER_UPDATE == ptNormalItem->HashItm.OpType))
			{
				stNorCacheItm.HashKey.Mask = ptNormalItem->HashKey.Mask;
				memcpy(stNorCacheItm.HashKey.DstIp, ptNormalItem->HashKey.DstIp, 4 * sizeof(u_int));

				stNorCacheItm.HashItm.DstLen = ptNormalItem->HashItm.DstLen;
				stNorCacheItm.HashItm.InterfaceIndex = ptNormalItem->HashItm.InterfaceIndex;
				stNorCacheItm.HashItm.OpType = ptNormalItem->HashItm.OpType;
				stNorCacheItm.HashItm.NormalFlag = 1;
				memcpy(stNorCacheItm.HashItm.NextHop, ptNormalItem->HashItm.NextHop, 4 * sizeof(u_int));
				ret = Item_CacheAdd(&stNorCacheItm, Flag);
				ROUTER_ERROR_LOG("Item_SendAndRecovery---->Item_CacheAdd failed  ret =:%d\n", ret);
			}
		}
	}
	else
	{
		if (NULL != ptNormalItem)
		{
			stForwardItem.Mask = ptNormalItem->HashKey.Mask;
			memcpy(stForwardItem.DstIp, ptNormalItem->HashKey.DstIp, 4 * sizeof(u_int));

			stForwardItem.DstLen = ptNormalItem->HashItm.DstLen;
			stForwardItem.InterfaceIndex = ptNormalItem->HashItm.InterfaceIndex;
			stForwardItem.OpType = ptNormalItem->HashItm.OpType;
			memcpy(stForwardItem.NextHop, ptNormalItem->HashItm.NextHop, 4 * sizeof(u_int));

			ret = Item_SendToForward(&stForwardItem);
			ROUTER_ERROR_LOG("Item_SendAndRecovery---->Item_SendToForward failed  ret =:%d\n", ret);
		}
	}
	return 0;
}

/************************************************************************************
	name          ：Item_CompareResultGet()
	function      ：裁决
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_CompareResultGet()
{
	u_int ret = 0;
	u_int Dstaddr[4] = { 0 };
	u_int Mask = 0;
	u_int i = 0;
	u_int j = 0;
	u_int k = 0;
	u_int loop = 0;
	u_int TimeInterval = 0;
	u_int Max[EXECUTE_MAX_NUM] = { 0 };
	bool CacheAddFlag = false;
	HASH_COLL_TABLE* ptHashLink = NULL;
	HASH_COLL_TABLE* ptHashPre = NULL;
	HASH_COLL_TABLE* ptHead = NULL;
	HASH_KEY* ptItemKey = NULL;
	HASH_COLL_TABLE* ptNewHead = NULL;
	struct timeval  Time = { 0 };
	HASH_COLL_TABLE   stNormalRouterItem[EXECUTE_MAX_NUM] = { 0 };
	HASH_COLL_TABLE   stAbnormalRouterItem[EXECUTE_MAX_NUM] = { 0 };


	gettimeofday(&Time, NULL);

	/*读锁定*/
	pthread_rwlock_rdlock(&rwlock);

	for (i = 0; i < HASH_MAX_ITEM; i++)
	{
		ptHead = g_ptArbitratHashLink[i];
		
		if (NULL == ptHead)
		{
			continue;
		}
		
		printf("collindex= :%d\n",i + 1);
		while (ptHead)
		{			
			u_int ItemNum = 1;
			u_int DiffNum = 0;
			ptHashPre = ptHead;
			ptHashLink = ptHead->pNext;

			TimeInterval = (Time.tv_sec - ptHead->HashItm.Time.TimeSec) + (Time.tv_usec - ptHead->HashItm.Time.TimeUSec) / 1000000;	
			if (TimeInterval > MAX_WAIT_TIME)
			{
				memcpy(&stNormalRouterItem[0], ptHead, sizeof(HASH_COLL_TABLE));
				while (ptHashLink)
				{
					memcpy(Dstaddr, ptHashLink->HashKey.DstIp, sizeof(Dstaddr));
					Mask = ptHashLink->HashKey.Mask;
					if ((0 == memcmp(ptHead->HashKey.DstIp, Dstaddr, sizeof(Dstaddr))) && (ptHead->HashKey.Mask == Mask))
					{
						memcpy(&stNormalRouterItem[ItemNum], ptHashLink, sizeof(HASH_COLL_TABLE));
						ItemNum++;
					}
					
					ptHashLink = ptHashLink->pNext;
				}

				if (ItemNum < 2)
				{
					for (j = 0; j < ItemNum; j++)
					{
						memcpy(&stAbnormalRouterItem[j], &stNormalRouterItem[j], sizeof(HASH_COLL_TABLE));
						DiffNum++;
					}

                    Item_SendAndRecovery(NULL,stAbnormalRouterItem,DiffNum,CacheAddFlag);
					//Ttem_AbnormalRouterPrintf(stAbnormalRouterItem, DiffNum);
				}
				else
				{
					/*记录路由相同的个数*/
					for (j = 0; j < ItemNum; j++)
					{
						for (k = 0; k < ItemNum; k++)
						{
							if ((stNormalRouterItem[j].HashItm.OpType == stNormalRouterItem[k].HashItm.OpType) &&
								(0 == memcmp(stNormalRouterItem[j].HashItm.NextHop,stNormalRouterItem[k].HashItm.NextHop,4 * sizeof(u_int))) &&
								(stNormalRouterItem[j].HashItm.InterfaceIndex == stNormalRouterItem[k].HashItm.InterfaceIndex))
							{
								Max[j]++;
							}
							else
							{
								continue;
							}
							
						}
					}


					/*找出次数相同最多的路由*/
					u_int max = 0;
					for (j = 1; j < ItemNum; j++)
					{
						if (Max[max] < Max[j])
						{
							max = j;
						}
						else if (Max[max] == Max[j])
						{
							if (stNormalRouterItem[max].HashItm.OpType < stNormalRouterItem[j].HashItm.OpType)
							{
								max = j;
							}
							else
							{
								continue;
							}
						}

					}

					printf("max=         %d\n", max);

					/*遍历找出不同路由*/
					for (j = 0; j < ItemNum; j++)
					{
						if ((stNormalRouterItem[max].HashItm.OpType == stNormalRouterItem[j].HashItm.OpType) &&
							(0 == memcmp(stNormalRouterItem[max].HashItm.NextHop, stNormalRouterItem[j].HashItm.NextHop, 4 * sizeof(u_int))) &&
							(stNormalRouterItem[max].HashItm.InterfaceIndex == stNormalRouterItem[j].HashItm.InterfaceIndex))
						{
							continue;
						}
						else
						{
							/*所有异常路由条目*/
							memcpy(&stAbnormalRouterItem[DiffNum], &stNormalRouterItem[j], sizeof(HASH_COLL_TABLE));
							DiffNum++;
						}
					}
					
					Ttem_NormalRouterPrintf(&stNormalRouterItem[max]);
					Ttem_AbnormalRouterPrintf(stAbnormalRouterItem, DiffNum);
					CacheAddFlag = true;
					ret = Item_SendAndRecovery(&stNormalRouterItem[max], stAbnormalRouterItem, DiffNum, CacheAddFlag);
					ROUTER_ERROR_LOG("Item_CompareResultGet---->Item_SendAndRecovery failed  ret =:%d\n", ret);

				}

				/*裁决之后从链表中删除相同元素*/
				Item_HashDel(ptHead, &ptNewHead, i);
				ptHead = ptNewHead;
			}
			else
			{
				ptHead = ptHead->pNext;
			}

		}

	}

	/*读解锁*/
	pthread_rwlock_unlock(&rwlock);

	return 0;
}

/************************************************************************************
	name          ：Item_ArbitraExecute()
	function      ：
	inparameter   ：sig信号
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int Item_ArbitraExecute(int sig)
{
	if (sig == SIGALRM)
	{
		Item_CompareResultGet();
	}
	return 0;
}

/************************************************************************************
	name          ：Arbitratment_TimeSet()
	function      ：设置定时器
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void Arbitratment_TimeSet()
{
	struct itimerval new_value;

	memset(&new_value, 0, sizeof(new_value));
	new_value.it_value.tv_sec = 10;
	new_value.it_value.tv_usec = 0;
	new_value.it_interval.tv_sec = 10;
	new_value.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &new_value, NULL);
	return;
}

/************************************************************************************
	name          ：Item_Arbitrament()
	function      ：
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
void Item_Arbitrament()
{
	Arbitratment_TimeSet();
	signal(SIGALRM, Item_ArbitraExecute);
	return;
}

#endif

/************************************************************************************
	name          ：main()
	function      ：
	inparameter   ：
	outparameter  ：
	version：            author： xhl              data：2019-05-10
************************************************************************************/
u_int main()
{	
	u_int retval = 0;
	pthread_t GetData;
	pthread_t pArbitratment;
	
	Item_HashInit();

	GetCorrectValue(&g_tCorrectValue);

	retval = pthread_rwlock_init(&rwlock, NULL);
	if (retval != 0) 
	{
		//  error
		exit(1);
	}
	

	retval = pthread_create(&GetData, NULL, (void *)&Itm_ArbitraQueAdd, NULL);
	if (retval)
	{
		printf("pthread_create GetData failed \n");
	}
	
//#if 0
	retval = pthread_create(&pArbitratment, NULL, (void *)&Item_Arbitrament, NULL);
	if (retval)
	{
		printf("pthread_create pArbitratment failed \n");
	}
	

//#endif
	pthread_join(GetData, NULL);

	pthread_join(pArbitratment, NULL);

	pthread_rwlock_destroy(&rwlock);
	
	return 0;

}

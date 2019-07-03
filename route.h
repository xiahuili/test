#pragma once

#define HASH_MAX_ITEM   (4 *1024)
#define HASH_INDEX_FIND (0x0)
#define HASH_INDEX_NOT_FIND (0x1)

#define ROUTER_OK    (0)
#define ROUTER_ERROR (1)

typedef unsigned int  u_int;

struct {
	struct nlmsghdr nlmsg;
	struct rtmsg rt;
}req;

typedef struct fpm_msg_hdr_t {
	/* Protocol version. */
	uint8_t version;

	/* Type of message, see below. */
	uint8_t msg_type;

	/* Length of entire message, including the header, in network byte order. */
	uint16_t msg_len;
}fpm_msg_hdr_t;

typedef struct  HASH_KEY
{
	uint32_t DstIp[4];
	uint8_t Mask;
}HASH_KEY;

typedef struct TIME_TWAMP
{
	uint32_t TimeSec;
	uint32_t TimeUSec;
}TIME_TWAMP;

typedef struct HASH_ITEM
{
	uint32_t NextHop[4];
	uint32_t InterfaceIndex;
	uint32_t MngIp;
	TIME_TWAMP  Time;
	uint16_t OpType;
	uint16_t DstLen;
}HASH_ITEM;

typedef struct HASH_COLL_TABLE
{
	HASH_KEY  HashKey;
	HASH_ITEM  HashItm;
	struct HASH_COLL_TABLE *pNext;

}HASH_COLL_TABLE;

typedef struct ROUTER_ITM
{
	uint32_t DstIp[4];
	uint8_t Mask;
	uint32_t NextHop[4];
	uint32_t InterfaceIndex;
	uint16_t OpType;
	uint16_t DstLen;
}ROUTER_ITM;


typedef enum OP_TYPE {
	ROUTER_ADD = 1,
    ROUTER_DEL,
    ROUTER_UPDATE,
}OP_TYPE;

typedef struct HASH_CACHE_ITEM
{
	uint32_t NextHop[4];
	uint32_t InterfaceIndex;	
	uint16_t DstLen;
	uint16_t OpType;
	uint8_t NormalFlag;
}HASH_CACHE_ITEM;

typedef struct HASH_CACHE_TABLE
{
	HASH_KEY  HashKey;
	HASH_CACHE_ITEM  HashItm;
	struct HASH_CACHE_TABLE* pNext;
}HASH_CACHE_TABLE;

typedef struct CLIENT_INFO
{
	u_int ClientSocket;
	u_int ManageIp;
}CLIENT_INFO;


#define ROUTER_ERROR_LOG(buf,ret) \
do{\
   if(ret){\
       printf("%s %d\n",buf,ret);\
       return ret;\
   }\
} while (0);


#define _RD(buf)   (1)

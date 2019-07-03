#!/usr/bin/python2.7
# -*- coding: utf-8 -*-

import unittest
import ctypes
import time
import copy
from ctypes import *
import signal
import os
import socket

lib = ctypes.cdll.LoadLibrary('./libroute.so')


class Hashkey(Structure):
    _fields_ = [('dip',c_uint *4),('mask',c_int)]

class Twaptime(Structure):
    _fields_ = [('timesec',c_uint),('timeusec',c_float)]

class Hashitem(Structure):
    _fields_ = [('nexthop',c_uint *4),
                ('ifid',c_uint),
                ('mngip',c_uint),
                ('time',Twaptime),
                ('optype',c_ushort),
                ('dstlen',c_ushort)]

class Hashstructure(Structure):
    _fields_ = [('hashkey',Hashkey),
                ('hashitem',Hashitem)]

class ArbitraQueue(Structure):
    _fields_ = [('hashnode',Hashstructure),('hashnext',Hashstructure)]

class Test(unittest.TestCase):
    
    '''
    #模拟一个执行体，配置路由
    def test_case1(self):
        self.mask = 24
        self.interfaceid = 1
        self.optype = 1
        self.dstlen = 4
        self.mngip = 0xc0a8030c       

        hashkey = Hashkey()
        int_arr4 = ctypes.c_uint*4
        dip = int_arr4()
        dip[0] = 0x00016464
        dip[1] = 0
        dip[2] = 0
        dip[3] = 0 

        hashkey.dip = dip
        hashkey.mask = self.mask

        hashitem = Hashitem()
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey = hashkey
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet

        t = time.time()
        twmptime = Twaptime()
        twmptime.timesec = int(t)
        twmptime.timeusec = t%1000000
        arbitraQueue.hashnode.hashitem.time = twmptime
        arbitraQueue.hashnode.hashitem.mngip =  self.mngip
        Item_QueueAdd(pointer(arbitraQueue))
        
        time.sleep(5)
        Item_CompareResultGet()    
    
    
    #模拟两个执行体，配置路由
    def test_case2(self):
        self.mask = 24
        self.interfaceid = 1
        self.optype = 1
        self.dstlen = 4
        self.mngip = [0xc0a8030c,0xc0a8030d]        

        hashkey = Hashkey()
        int_arr4 = ctypes.c_uint*4
        dip = int_arr4()
        dip[0] = 0x00016464
        dip[1] = 0
        dip[2] = 0
        dip[3] = 0 

        hashkey.dip = dip
        hashkey.mask = self.mask

        hashitem = Hashitem()
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey = hashkey
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet

        for mngaddress in self.mngip:
            t = time.time()
            twmptime = Twaptime()
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            arbitraQueue.hashnode.hashitem.mngip = mngaddress
            Item_QueueAdd(pointer(arbitraQueue))
        
        time.sleep(5)
        Item_CompareResultGet()    
    
    
    #模拟三个执行体，配置相同的路由表项
    def test_case3(self):
        self.mask = 24
        self.interfaceid = 1
        self.optype = 1
        self.dstlen = 4
        self.mngip = [0xc0a8030c,0xc0a8030d,0xc0a8030e]        

        hashkey = Hashkey()
        int_arr4 = ctypes.c_uint*4
        dip = int_arr4()
        dip[0] = 0x00016464
        dip[1] = 0
        dip[2] = 0
        dip[3] = 0 

        hashkey.dip = dip
        hashkey.mask = self.mask

        hashitem = Hashitem()
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey = hashkey
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet

        for mngaddress in self.mngip:
            t = time.time()
            twmptime = Twaptime()
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            arbitraQueue.hashnode.hashitem.mngip = mngaddress
            Item_QueueAdd(pointer(arbitraQueue))
            
        time.sleep(5)
        Item_CompareResultGet()
    

    #模拟三个执行体，配置相同的路由表项
    def test_case4(self):
        self.mask = 24
        self.interfaceid = 1
        self.optype = 1
        self.dstlen = 4
        self.mngip = [0xc0a8030c,0xc0a8030d]        

        hashkey = Hashkey()
        int_arr4 = ctypes.c_uint*4
        dip = int_arr4()
        dip[0] = 0x00016464
        dip[1] = 0
        dip[2] = 0
        dip[3] = 0 

        hashkey.dip = dip
        hashkey.mask = self.mask

        hashitem = Hashitem()
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey = hashkey
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet

        twmptime = Twaptime()

        for mngaddress in self.mngip:
            t = time.time()
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            arbitraQueue.hashnode.hashitem.mngip = mngaddress
            Item_QueueAdd(pointer(arbitraQueue))
        
        #添加执行体三的路由
        time.sleep(2)
        arbitraQueue.hashnode.hashitem.mngip =  0xc0a8030e  
        arbitraQueue.hashnode.hashitem.ifid = 2
        t = time.time()       
        twmptime.timesec = int(t)
        twmptime.timeusec = t%1000000
        arbitraQueue.hashnode.hashitem.time = twmptime
        Item_QueueAdd(pointer(arbitraQueue))

        time.sleep(5)
        Item_CompareResultGet()


    #模拟四个执行体，两两路由相同
    def test_case5(self):
        self.mask = 24
        self.interfaceid = 1
        self.optype = 1
        self.dstlen = 4
        self.mngip = [0xc0a8030a,0xc0a8030b]        

        hashkey = Hashkey()
        int_arr4 = ctypes.c_uint*4
        dip = int_arr4()
        dip[0] = 0x00016464
        dip[1] = 0
        dip[2] = 0
        dip[3] = 0 

        hashkey.dip = dip
        hashkey.mask = self.mask

        hashitem = Hashitem()
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey = hashkey
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet

        twmptime = Twaptime()

        for mngaddress in self.mngip:
            t = time.time()
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            arbitraQueue.hashnode.hashitem.mngip = mngaddress
            Item_QueueAdd(pointer(arbitraQueue))
        
        #添加执行体三四的路由
        self.mngip1 = [0xc0a8030c,0xc0a8030d] 
        arbitraQueue.hashnode.hashitem.ifid = 2
        for mngip in self.mngip1:
            t = time.time()       
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            arbitraQueue.hashnode.hashitem.mngip = mngip
            arbitraQueue.hashnode.hashitem.optype = 2
            Item_QueueAdd(pointer(arbitraQueue))

        time.sleep(5)
        Item_CompareResultGet()
      

    #纠正模式，模拟三个执行体
    def test_case6(self):
        self.mask = 24
        self.interfaceid = 1
        self.optype = 2
        self.dstlen = 4
        self.mngip = [0xc0a8030a,0xc0a8030b]        

        hashkey = Hashkey()
        int_arr4 = ctypes.c_uint*4
        dip = int_arr4()
        dip[0] = 0x00016464
        dip[1] = 0
        dip[2] = 0
        dip[3] = 0 

        hashkey.dip = dip
        hashkey.mask = self.mask

        hashitem = Hashitem()
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey = hashkey
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet
        Item_Send_CacheAdd = lib.Item_Send_CacheAdd
        
        #添加缓存
        self.flag = False
        Item_Send_CacheAdd(pointer(arbitraQueue),self.flag)

        twmptime = Twaptime()

        for mngaddress in self.mngip:
            t = time.time()
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            arbitraQueue.hashnode.hashitem.mngip = mngaddress
            Item_QueueAdd(pointer(arbitraQueue))
        
        #添加执行体三四的路由
        self.mngip1 = 0xc0a8030c 
        arbitraQueue.hashnode.hashitem.ifid = 2
        Item_Send_CacheAdd(pointer(arbitraQueue),self.flag)
        
        t = time.time()       
        twmptime.timesec = int(t)
        twmptime.timeusec = t%1000000
        arbitraQueue.hashnode.hashitem.time = twmptime
        arbitraQueue.hashnode.hashitem.mngip = self.mngip1
        Item_QueueAdd(pointer(arbitraQueue))

        time.sleep(5)
        Item_CompareResultGet()
    '''  
        
    #添加多条路由
    def test_case7(self):
        self.mask = 32
        self.interfaceid = 1
        self.optype = 2
        self.dstlen = 4
        self.mngip = 0xc0a8030a       
      
        int_arr4 = ctypes.c_uint*4 
        nexthop =  int_arr4()
        nexthop[0] = 0x0a010101
        nexthop[1] = 0
        nexthop[2] = 0
        nexthop[3] = 0
        
        hashitem = Hashitem()
        hashitem.nexthop =  nexthop
        hashitem.ifid = self.interfaceid
        hashitem.optype =  self.optype
        hashitem.dstlen = self.dstlen

        arbitraQueue = ArbitraQueue()
        arbitraQueue.hashnode.hashkey.mask = self.mask
        arbitraQueue.hashnode.hashitem = hashitem

        #调用待裁决队列函数，添加待裁决队列
        Item_QueueAdd =  lib.Item_QueueAdd
        Item_QueueAdd.argtypes = [POINTER(ArbitraQueue)]
        Item_CompareResultGet = lib.Item_CompareResultGet
                     
        dip = int_arr4()
        twmptime = Twaptime()
        for j in range(0,255):                
            dip[0] = 0x64640100 + j
            dip[1] = 0
            dip[2] = 0
            dip[3] = 0
            arbitraQueue.hashnode.hashkey.dip = dip                
                   
            t = time.time()       
            twmptime.timesec = int(t)
            twmptime.timeusec = t%1000000
            arbitraQueue.hashnode.hashitem.time = twmptime
            Item_QueueAdd(pointer(arbitraQueue))

        time.sleep(5)
        Item_CompareResultGet()    
                            
if __name__=='__main__':
    
    unittest.main()

    

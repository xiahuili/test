#指定交叉编译工具链
CROSS_COMPILE = arm-hisiv100nptl-linux-
AS    = $(CROSS_COMPILE)as
LD    = $(CROSS_COMPILE)ld
CPP   = $(CC) -E
AR    = $(CROSS_COMPILE)ar
NM    = $(CROSS_COMPILE)nm

$sudo CFLAGS="-fPIC" ./configure

# socket  include and source makefile
CC = gcc
CFLAGS = -c
OBJ = route.o 
LIBOBJ = libroute.so
CPPFLAGS  :=
  
#获取工程的根目录的全路径
SOURCE_ROOT = $(shell pwd)

#-I是指定头文件的意思，所以这里是将所有的头文件都包含在INCLUDE_DIR变量中
INCLUDE_DIR := -I $(DOURCE_ROOT)/route/

#生成的目标为库文件名称是mylib.so
APP_NAME = libroute.so


#将所有的.c文件都包含在APP_OBJECT中

APP_OBJECTC += route.c

#作用： $(patsubst %.c,%.o,$(dir))中，patsubst把(dir)中变量名是.c的文件替换成.o

STATIC_OBJ_O = $(patsubst %.c,%.o,$(APP_OBJECTC))

all:test

#编译可执行文件
test:$(STATIC_OBJ_O) $(APP_NAME) 
	$(CC) -o test $(STATIC_OBJ_O) $(APP_NAME) -lpthread	

#-fPIC的作用是：告诉编译器，产生位置无关码

$(STATIC_OBJ_O):$(APP_OBJECTC) 
	$(CC) -fPIC -c $(APP_OBJECTC) -lpthread
    
#目标是.so，依赖.o文件
#-shared的作用就是制定生成.so文件
$(APP_NAME):$(STATIC_OBJ_O) 
	$(CC) -shared -fPIC -o $(APP_NAME) $(STATIC_OBJ_O)
 
clean:
		rm -f *o *so test  
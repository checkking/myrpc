WORKROOT = ../
RPC_BASE = $(WORKROOT)/base/

LIB_SRC = $(wildcard *.cpp)
INCLUDE_PATH = -I$(RPC_BASE)/output/include/ \
			   -I./
LIB_PATH = -L$(RPC_BASE)/output/lib -lrpc_base \
		   -lpthread \
		   -lrt

CC = g++
CPPFLAGS = -Wall -g $(INCLUDE_PATH) \
		   -W \
		   -Wno-unused

RPC_NET_SRC = $(notdir $(LIB_SRC))
OBJS = $(patsubst %.cpp,%.o,$(RPC_NET_SRC))

librpc_net.a: $(RPC_NET_SRC)
	$(CC) $(CPPFLAGS) -c $^
	ar rcs  $@ $(OBJS)
	mkdir -p ./output/lib
	cp -f --link librpc_net.a ./output/lib
	mkdir -p ./output/include
	cp -f --link *.h ./output/include

clean:
	rm -f *.o *.a *.core
	rm -rf ./output

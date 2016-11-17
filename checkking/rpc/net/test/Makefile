include ../../common.mk

WORKROOT = ../../
RPC_BASE = $(WORKROOT)/base/
NET_BASE = $(WORKROOT)/net/
THIRDLIBPATH = $(WORKROOT)third-64
GTEST=$(THIRDLIBPATH)/googletest/
INCLUDE_PATH = -I$(GTEST)/include/ \
			   -I$(RPC_BASE)/output/include/ \
			   -I$(NET_BASE)/output/include/ \
			   -I./

LIB_PATH = -L$(RPC_BASE)/output/lib -lrpc_base \
		   -L$(GTEST)/lib -lgtest \
		   -L$(GTEST)/lib -lgtest_main \
		   -L$(NET_BASE)/output/lib -lrpc_net \
		   -L$(RPC_BASE)/output/lib -lrpc_base \
		   -lpthread \
		   -lrt
CPP = g++
CPPFLAGS = -Wall -g $(INCLUDE_PATH) \
		   -W \
		   -Wno-unused

ALLTESTS=$(wildcard *_test.cpp)
CPPFILE=$(ALLTESTS)
EXE=$(basename $(ALLTESTS))

all : $(EXE)
$(EXE) : %: %.cpp $(OBJ)
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LIB_PATH)

.PHONY : clean all test

clean:
	rm -rf $(EXE) core.* log *.o *.log

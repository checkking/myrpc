WORKROOT = ../
LIB_SRC =		\
	LogStream.cpp \
	Timestamp.cpp

include $(WORKROOT)/common.mk

RPC_BASE_SRC = $(notdir $(LIB_SRC))
OBJS = $(patsubst %.cpp,%.o,$(RPC_BASE_SRC))

librpc_base.a: $(RPC_BASE_SRC)
	$(CC) $(CXXFLAGS) -c $^
	ar rcs $@ $(OBJS)
	mkdir -p ./output/lib
	cp -f --link librpc_base.a ./output/lib
	mkdir -p ./output/include
	cp -f --link *.h ./output/include

clean:
	rm -f *.o *.a *.core
	rm -rf ./output

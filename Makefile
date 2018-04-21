CXX=g++
ifndef (DEBUG)
DEBUG=1
endif

ifndef (VERSION)
VERSION="\"1.0.0.1\""
endif

ifndef (BUILDDATE)
BUILDDATE= "\"`date +%Y-%m-%d\ %k:%M:%S`\""
endif

TARGETNAME="mvpTest"

BUILDINFO=-DVERSION=$(VERSION) -DBUILDDATE=$(BUILDDATE)

#include dirs
INCLUDEDIR=-I/usr/local/include/ \
-I./include/ \

#include libs
LINK_A_LIB=-L ./lib -lvasapi -ltinyxml
LINK_SO_LIB=-L/opt/lib64 -lMiddleware -lcommon

LINK_ALL_LIB= -ldl -lpthread -lcrypt  $(LINK_SO_LIB) $(LINK_A_LIB)

ifeq ($(DEBUG),1)
CXXFLAGS=-g -Wall -Wno-unused-parameter $(INCLUDEDIR) -Wno-deprecated $(BUILDINFO)
else
CXXFLAGS=-Wall -O3 -Wno-unused-parameter $(INCLUDEDIR) $(BUILDINFO)
endif

#objects
SRC=$(wildcard ./*.cpp)
OBJS=$(patsubst %.cpp, %.o, $(notdir $(SRC)))

all: $(TARGETNAME)

$(TARGETNAME):$(OBJS)
	$(CXX) -o $@ $(OBJS) $(LINK_ALL_LIB)
%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
clean:
	@rm -rf *.o $(TARGETNAME)
install:
	


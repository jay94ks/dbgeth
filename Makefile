CXX=g++
CFLAGS=-static -O2
SRCS=main.cpp
OBJS=$(SRCS:.cpp=.o)
TARGET=dbgeth
INSTALLPATH=/usr/jaehoon
MKDIR=mkdir -v
RMDIR=rm
COPY=cp -v

.SUFFIXES: .cpp .o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)

install: $(TARGET)
	$(MKDIR) $(INSTALLPATH)
	$(COPY) $(TARGET) $(INSTALLPATH)/$(TARGET)

uninstall:
	$(RMDIR) $(INSTALLPATH) -rf

clean:
	rm -f $(OBJS) $(TARGET)


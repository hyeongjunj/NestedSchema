CXX=g++
CXXFLAGS=-std=c++17
OBJS=main.o	encoding.o
TARGET=main

all: $(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) -o	$@	$(OBJS)

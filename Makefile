CXX=g++
CXXFLAGS=-std=c++17
OBJS=src/main.o	src/encoding.o
TARGET=main

all: $(TARGET)

clean:
	rm -f src/*.o
	rm -f $(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) -o	$@	$(OBJS)

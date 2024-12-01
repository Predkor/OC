CC=g++
CXXFLAGS = -I./crow/include -I/usr/include -lpthread -lelf
TARGET = server
SRC = server.cpp
FLAGS=-lpthread

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

all: build start

build:
	$(CC) $(FLAGS) -o server server.cpp

start:
	./server

clean:
	rm server
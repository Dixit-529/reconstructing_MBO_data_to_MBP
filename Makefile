# Makefile for Orderbook Reconstruction

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -pedantic
TARGET = reconstruction_dixit

SRCS = main.cpp OrderBook.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) mbp_output.csv
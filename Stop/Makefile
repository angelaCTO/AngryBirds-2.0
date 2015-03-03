CXX = g++
CXXFLAGS = -O2 -std=c++0x
CLIENT_OBJS = ../Client/ClientSocket.o 
SOCKET_OBJS = ../Socket/Socket.o

all: sendStop.o $(CLIENT_OBJS) $(SOCKET_OBJS)
	$(CXX) -o sendStop sendStop.o $(CLIENT_OBJS) $(SOCKET_OBJS)
clean:
	rm -rf ServerSocket.o

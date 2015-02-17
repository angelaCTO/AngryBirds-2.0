CXX = g++ -O2 -std=c++0x 
CXXFLAGS =  
CXXLIBS = -lpthread
BLACK_OBJS = BlackLib/BlackLib.o
SERVER_OBJS = Server/ServerSocket.o Socket/Socket.o
CLIENT_OBJS = Client/ClientSocket.o Socket/Socket.o
SERIAL_OBJS = serial/serialib.o
OBJS= $(BLACK_OBJS) $(SENSOR_SIGNAL_OBJS) $(SERIAL_OBJS)

all: $(OBJS) main 

main: main.o $(BLACK_OBJS) $(SERVER_OBJS) $(SERIAL_OBJS)
	$(CXX) -o main main.o $(SERVER_OBJS) $(BLACK_OBJS) $(SERIAL_OBJS) `pkg-config --libs opencv` $(CXXLIBS)
sendStop: sendStop.o $(CLIENT_OBJS)
	$(CXX) -o sendStop sendStop.o $(CLIENT_OBJS)
serialib.o: serial/serialib.cpp
	$(CXX) -c serial/serialib.cpp $(CXXLIBS)
BlackLib/BlackLib.o: BlackLib/BlackLib.cpp
	$(MAKE) -C BlackLib
Server/ServerSocket.o: Server/ServerSocket.cpp
	$(MAKE) -C Server
Client/ClientSocket.o: Client/ClientSocket.cpp
	$(MAKE) -C Client
Socket/Socket.o: Socket/Socket.cpp
	$(MAKE) -C Socket

clean:
	rm -rf *.o main
	rm -rf bin/main 
	$(MAKE) -C BlackLib clean
	$(MAKE) -C cron clean
	$(MAKE) -C SensorSignal clean
	$(MAKE) -C Server clean
	$(MAKE) -C Client clean
	$(MAKE) -C Socket clean

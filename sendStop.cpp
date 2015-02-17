#include "Client/ClientSocket.h"
#include "Socket/SocketException.h"
#include <iostream>
#include <string>

using namespace std;

#define PORT_NUMBER 30000

//Uncomment below for debug
//#define DEBUG

int main(int argc, char** argv){
    try{
        ClientSocket client_socket("localhost", PORT_NUMBER);
	try{
	    client_socket << "STOP";
	} catch (SocketException &){}
	#ifdef DEBUG
	cout << "We sent a signal" << endl
	#endif
    }catch(SocketException& e){
	#ifdef DEBUG
        cout << "Exception was caught: " << e.description() << endl;
	#endif
    }
}

#include "Server.h"

#define DEFAULT_PORT 999

int main(int argc, char *argv[]) 
{
	int port = DEFAULT_PORT;
	if(argc >= 2)
	{
		port = atoi(argv[1]);
		port = (port == 0) ? DEFAULT_PORT : port;
	}
	
	Server* server = new Server(port);
	server->initialize();
	server->start();
	delete server;
	return 0;
}
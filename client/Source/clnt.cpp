#include "Client.h"

int main() {
	Client* client = new Client;
	if(!client->initialize())
	{
		return 0;
	}

	std::cout << ">> ";
	std::string serverAddress;
	getline(std::cin, serverAddress);

	if(!client->serverConnect(serverAddress))
	{
		return 0;
	}

	client->start();
	delete client;
	return 0;
}
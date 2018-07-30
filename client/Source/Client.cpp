#include "Client.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <iostream>

#define BUF_SIZE 1024

Client::Client() :
	mSocket(-1),
	mCurrentState(NULL),
	mTree(NULL)
{
	mRecvExpression = new RecvExpressionState(this);
	mRecvVrblValues = new RecvVrblValuesState(this);
	mRecvResult = new RecvResultState(this);
}
Client::~Client()
{
	if(mTree)
	{
		delete mTree;
	}

	delete mRecvExpression;
	delete mRecvVrblValues;
	delete mRecvResult;
}

bool Client::initialize()
{
	char buff[BUF_SIZE];
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
    {
        printf("WSAStart error %d\n", WSAGetLastError());
        return false;
    }
 
    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket < 0)
    {
        printf("Socket() error %d\n", WSAGetLastError());
        return false;
    }
	return true;
}

bool Client::serverConnect(const std::string& srvraddr)
{
	int port;
	std::string sHost;
	if(!mParser.parseServerAddress(srvraddr, sHost, port))
	{
		std::cout << "Incorrect server address\n";
		return false;
	}
	
	sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    HOSTENT *host;

	destAddr.sin_addr.s_addr = inet_addr(sHost.c_str());
	if (destAddr.sin_addr.s_addr == INADDR_NONE)
    {
		if (host = gethostbyname(sHost.c_str()))
            ((unsigned long *)&destAddr.sin_addr)[0] =
            ((unsigned long **)host->h_addr_list)[0][0];
        else
        {
			std::cout << "Invalid address " << sHost << "\n";
			closeSocket();
            return false;
        }
    }
 
    if (connect(mSocket, (sockaddr *)&destAddr, sizeof(destAddr)))
    {
		std::cout << "Connect error " << WSAGetLastError() << "\n";
        return false;
    }
 
	std::cout << "<< connected\n";

	mCurrentState = mRecvExpression;
	return true;	
}

void Client::start()
{
	while(1)
	{
		if(mSocket == 0)
		{
			std::cout << "server closed connection\n";
			break;
		}
		
		if(mCurrentState->getData())
		{
			continue;
		}

		if(mCurrentState->getVariableValues())
		{
			continue;
		}

		if(mCurrentState->getResult())
		{
			continue;
		}
	}
}

bool Client::createTree()
{
	mTree = mParser.createTree();
	if(mTree)
	{
		std::string serialized;
		mTree->serialize(serialized);

		if(!sendTree(serialized))
		{
			return false;
		}

		std::string buffer;	 
		if(!recvData(mSocket, buffer))
		{
			return false;
		}
		if(buffer != "ok")
		{
			if(mTree)
				delete mTree;
			mParser.initialize();
			std::cout << "<<" + buffer + "\n";
			std::cout << "<< enter expression again\n";
			return false;
		}
		
		std::cout << "<< OK\n";
		return true;
	}
	else
	{
		if(mParser.checkExit())
		{
			std::string exit("exit");
			send(mSocket, exit.c_str(), exit.length(), 0);	
			closeSocket();
		}
		else
		{
			std::string error;
			mParser.getLastError(error);
			std::cout << "<< " + error + "\n";
			std::cout << "<< enter expression again\n";
			mParser.initialize();
		}
		return false;
	}
}

bool Client::sendTree(std::string& serialized)
{
	std::string buf;
	int i = 1;
	buf = serialized.substr(0, BUF_SIZE);
	while(1)
	{
		if(send(mSocket, buf.c_str(), buf.length(), 0) == SOCKET_ERROR)
		{
			std::cout << "<< connection to server lost\n";
			closeSocket();
			return false;
		}
		if(buf.length() < BUF_SIZE)
		{
			break;
		}
		buf = "";
		buf = serialized.substr(i*BUF_SIZE, BUF_SIZE);
		i++;
	}
	return true;
}

bool Client::processVrblValues()
{
	std::string variables;
	if(mParser.createVariableValues(variables))
	{ 
		if(send(mSocket, variables.c_str(), variables.length(), 0) == SOCKET_ERROR)
		{
			std::cout << "<< connection to server lost\n";
			closeSocket();
			return false;
		}
		
		std::string buffer;
		if(!recvData(mSocket, buffer))
		{
			return false;
		}
		if(buffer != "ok")
		{
			std::cout << buffer << "\n";
			return false;
		}

		return true;
	}
	return false;
}

bool Client::getResult()
{
	std::string str("calculate");
	if(send(mSocket, str.c_str(), str.length(), 0) == SOCKET_ERROR)
	{
		closeSocket();
		return false;
	}
	
	std::string buffer;
	if(!recvData(mSocket, buffer))
	{
		return false;
	}

	std::cout << "<< " << buffer << "\n";
	
	if(buffer == "unable to count expression: not all variables values were sent to server")
	{
		std::cout << "<< for having result enter all variables correctly again\n";
		return false;
	}

	delete mTree;
	mParser.initialize();

	return true;
}

void Client::closeSocket()
{
	closesocket(mSocket);
    WSACleanup();
	mSocket = 0;
}

bool Client::recvData(SOCKET s, std::string& buf)
{
	char recvbuf[BUF_SIZE];
	int nsize = recv(mSocket, (char *)&recvbuf, sizeof(recvbuf), 0);
	if(nsize == SOCKET_ERROR)
	{
		std::cout << "<< connection to server lost\n";
		closeSocket();
		return false;
	}
	std::string strbuf(recvbuf, nsize);
	buf = strbuf;
	size_t pos = 0;
	do
	{
		pos = buf.find("ping");

		if(pos == -1)
		{	
			break;
		}

		buf = buf.substr(pos + 4).c_str();

	}while(pos != -1);

	if(buf.length() == 0)
	{
		nsize = recv(mSocket, (char *)&recvbuf, sizeof(recvbuf), 0);
		if(nsize == SOCKET_ERROR)
		{
			std::cout << "<< connection to server lost\n";
			closeSocket();
			return false;
		}
		std::string strbuf(recvbuf, nsize);
		buf = strbuf;
	}
	return true;
}

void Client::setNextState(State* state)
{
	mCurrentState = state;
}

RecvExpressionState::RecvExpressionState(Client* client) :
	mClient(client)
{
}

bool RecvExpressionState::getData()
{
	std::cout << "<< enter expression\n";
	std::cout << ">> ";
	if(mClient->createTree())
	{
		mClient->setNextState(mClient->mRecvVrblValues);
	}
	return true;
}

bool RecvExpressionState::getVariableValues()
{
	return false;
}

bool RecvExpressionState::getResult()
{
	return false;
}

RecvVrblValuesState::RecvVrblValuesState(Client* client) :
	mClient(client)
{
}


bool RecvVrblValuesState::getData()
{
	return false;
}

bool RecvVrblValuesState::getVariableValues()
{
	if(mClient->processVrblValues())
	{
		mClient->setNextState(mClient->mRecvResult);
	}
	return true;
}

bool RecvVrblValuesState::getResult()
{
	return false;
}

RecvResultState::RecvResultState(Client* client) :
	mClient(client)
{
}


bool RecvResultState::getData()
{
	return false;
}
bool RecvResultState::getVariableValues()
{
	return false; 
}

bool RecvResultState::getResult()
{
	if(mClient->getResult())
	{
		mClient->setNextState(mClient->mRecvExpression);
	}
	else
	{
		mClient->setNextState(mClient->mRecvVrblValues);
	}
	return true;
}
#include "Server.h"

#include <cstdio>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

#define BUF_SIZE 1024

Server::Server(int port) :
	mPort(port),
	mListenSocket(-1)
{
	std::cout<< "Server created\n";
}

bool Server::initialize()
{
	char buff[BUF_SIZE]; 

    if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
    {
		std::cout << "Error WSAStartup " << WSAGetLastError() << "\n";
        return false;
    }
 
    if ((mListenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "Error socket " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

	DWORD dw = 1;
	ioctlsocket(mListenSocket, FIONBIO, &dw);

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(mPort);
    localAddr.sin_addr.s_addr = 0;
 
    if (bind(mListenSocket, (sockaddr *)&localAddr, sizeof(localAddr)))
    {
        std::cout << "Error bind " << WSAGetLastError() << "\n";
        closesocket(mListenSocket);
        WSACleanup();
        return false;
    }

	std::cout << "Server successfully initialized on port" << " " << mPort << "\n";
	return true;
}

void Server::start()
{
	std::cout << "Waiting for connections\n";
 
	listen(mListenSocket, SOMAXCONN); 
 
	while (true)
    {
		sockaddr_in clientAddr;
		int clientAddrSize = sizeof(clientAddr);

		SOCKET socket = accept(mListenSocket, (sockaddr *)&clientAddr, &clientAddrSize);

		process(socket);
    }
}

void Server::process(SOCKET socket)
{
	if (socket != INVALID_SOCKET)
	{
		std::cout << "new client " << socket << " connected\n";
		mClientSessions[socket] = new ClientSession(socket);
	}

	std::map<SOCKET, ClientSession*>::iterator it = mClientSessions.begin();
	while(it != mClientSessions.end())
	{
		if (!it->second->processData())
		{
			std::cout << "client " << it->first << " disconnected\n";
			std::map<SOCKET, ClientSession*>::iterator it1 = it++;
			delete it1->second;
			mClientSessions.erase(it1);
		}
		else
		{
			it++;
		}
	}
}

void Server::stop()
{
 //TODO
}

ClientSession::ClientSession(SOCKET socket) :
	mNoData(0),
	mCurrentState(NULL),
	mClientSocket(socket),
	mTree(NULL)
{
	mRecvExpression = new RcvExpressionState(this);
	mRecvVrblValues = new RcvVrblValuesState(this);
	mSendResult = new SendResultState(this);

	mCurrentState = mRecvExpression;
}

bool ClientSession::processData()
{
	int nsize;
	char recvbuf[BUF_SIZE];
	while ((nsize = recv(mClientSocket, (char *) &recvbuf, sizeof(recvbuf), 0)) != SOCKET_ERROR)
	{
		recvbuf[nsize] = 0;
		
		std::string data(recvbuf);
		std::string message = "received: ";
		message += data;
		log(message);
		if (data == "exit")
		{
			return false;
		}

		if(mCurrentState->getData(data))
		{
			break;
		}
		
		if(mCurrentState->getVariableValues(data))
		{
			break;
		}

		if(mCurrentState->getResult(data))
		{
			break;
		}
	}
	if(nsize == SOCKET_ERROR)
	{
		mNoData++;
		std::string ping = "ping";
		if(mNoData == 8000000)
		{
			if(send(mClientSocket, ping.c_str(), ping.length(), 0) == SOCKET_ERROR)
			{
				log("is unreachable");
				return false;
			}
		}
		
	}
	else
	{
		mNoData = 0;
	}

	return true;
}

bool ClientSession::createTree(const std::string& data)
{
	bool result = false;
	std::string response = "unable to allocate memory for tree";
	mTree = Parser::createEmptyTree();
	if(mTree)
	{
		mTree->deserialize(data);
		log("tree was created");
		response = "ok";
		result = true;
	}
	else
	{
		log(response);
	}
	
	send(mClientSocket, response.c_str(), response.length(), 0);
	return true;
}

bool ClientSession::processVrblValues(const std::string& data)
{
	bool result = false;
	std::string response = "error in variable values";
	
	if(Parser::parseVariableValues(data, *mTree))
	{
		log("variable values were successfully parsed");
		response = "ok";
		result = true;
	}

	send(mClientSocket, response.c_str(), response.length(), 0);
	return result;
}

bool ClientSession::sendResult()
{
	std::string response = "unable to count expression";
	if(mTree)
	{
			
		long long value = mTree->count();
		if(value == -1)
		{
			response = "unable to count expression: not all variables values were sent to server";
			log(response);
		}
		else
		{
			std::ostringstream oss;
			oss << value;
			response = oss.str();
		}
	}
	log(response);
	send(mClientSocket, response.c_str(), response.length(), 0);

	delete mTree;

	return true;
}

void ClientSession::setNextState(SessionState* state)
{
	mCurrentState = state;
}

void ClientSession::log(const std::string& string)
{
	std::cout << "client " << mClientSocket << ": " << string << "\n";
}

RcvExpressionState::RcvExpressionState(ClientSession* session) :
	mSession(session)
{
}

bool RcvExpressionState::getData(const std::string& data)
{
	if(mSession->createTree(data))
	{
		mSession->setNextState(mSession->mRecvVrblValues);
	}
	return true;
}

bool RcvExpressionState::getVariableValues(const std::string& data)
{
	return false;
}

bool RcvExpressionState::getResult(const std::string& data)
{
	return false;
}

RcvVrblValuesState::RcvVrblValuesState(ClientSession* session) :
	mSession(session)
{
}

bool RcvVrblValuesState::getData(const std::string& data)
{
	return false;
}

bool RcvVrblValuesState::getVariableValues(const std::string& data)
{
	if(mSession->processVrblValues(data))
	{
		mSession->setNextState(mSession->mSendResult);
	}
	return true;
}

bool RcvVrblValuesState::getResult(const std::string& data)
{
	return false;
}

SendResultState::SendResultState(ClientSession* session) :
	mSession(session)
{
}

bool SendResultState::getData(const std::string& data)
{
	return false;
}

bool SendResultState::getVariableValues(const std::string& data)
{
	return false;
}

bool SendResultState::getResult(const std::string& data)
{
	if(mSession->sendResult())
	{
		mSession->setNextState(mSession->mRecvExpression);
	}
	return true;
}
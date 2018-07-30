#include "Parser.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

#include <map>
#include <vector>

class Client;

class State
{
public:
	virtual bool getData() = 0;
	virtual bool getVariableValues() = 0;
	virtual bool getResult() = 0;
};

class RecvExpressionState: public State
{
	Client* mClient;
public:
	RecvExpressionState(Client* client);

	bool getData();
	bool getVariableValues();
	bool getResult();
};

class RecvVrblValuesState: public State
{
	Client* mClient;
public:
	RecvVrblValuesState(Client* client);

	bool getData();
	bool getVariableValues();
	bool getResult();
};

class RecvResultState: public State
{
	Client* mClient;
public:
	RecvResultState(Client* client);

	bool getData();
	bool getVariableValues();
	bool getResult();
};

class Client
{
public:
	friend class RecvExpressionState;
	friend class RecvVrblValuesState;
	friend class RecvResultState;
	State* mCurrentState;

	State* mRecvExpression;
	State* mRecvVrblValues;
	State* mRecvResult;
public:
	Client();
	~Client();

	bool initialize();
	bool serverConnect(const std::string& srvraddr);
	void start();
	
private:
	bool createTree();
	bool sendTree(std::string& serialized);

	bool processVrblValues();
	bool getResult();

	void closeSocket();

	bool recvData(SOCKET s, std::string& buf);

	void setNextState(State* state);

	bool processInput();

	SOCKET mSocket;

	Parser mParser;
	ASTTree* mTree;
};

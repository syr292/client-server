#include "Parser.h"
#include <map>
#include <winsock2.h>
#include <windows.h>

class ClientSession;

class SessionState
{
public:
	virtual bool getData(const std::string& data) = 0;
	virtual bool getVariableValues(const std::string& data) = 0;
	virtual bool getResult(const std::string& data) = 0;
};

class RcvExpressionState: public SessionState
{
	ClientSession* mSession;
public:
	RcvExpressionState(ClientSession* session);

	bool getData(const std::string& data);
	bool getVariableValues(const std::string& data);
	bool getResult(const std::string& data);
};

class RcvVrblValuesState: public SessionState
{
	ClientSession* mSession;
public:
	RcvVrblValuesState(ClientSession* session);

	bool getData(const std::string& data);
	bool getVariableValues(const std::string& data);
	bool getResult(const std::string& data);
};

class SendResultState: public SessionState
{
	ClientSession* mSession;
public:
	SendResultState(ClientSession* session);

	bool getData(const std::string& data);
	bool getVariableValues(const std::string& data);
	bool getResult(const std::string& data);
};


class ClientSession
{
public:
	int mNoData;
	
	SessionState* mCurrentState;

	SessionState* mRecvExpression;
	SessionState* mRecvVrblValues;
	SessionState* mSendResult;
public:
	ClientSession(SOCKET socket);

	bool processData();
	bool createTree(const std::string& data);
	bool processVrblValues(const std::string& data);
	bool sendResult();

	void setNextState(SessionState* state);
	
private:

	void log(const std::string& string);
	
	SOCKET mClientSocket;
	ASTTree* mTree;
};

class Server
{
public:
	Server(int port);
	bool initialize();
	void start();
	void stop();
private:
	void process(SOCKET socket);

	int mPort;

	std::map<SOCKET, ClientSession*> mClientSessions;

	SOCKET mListenSocket;
};


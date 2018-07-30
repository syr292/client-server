#include "Parser.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <iostream>

#define MAX_VALUE 65535

Parser::Parser() :
	mLastChar(' '),
	mNumVal(-1),
	mVariableStr(),
	mCurrentToken(0)
{
	mOperatorPriority['+'] = 10;
	mOperatorPriority['-'] = 10;
	mOperatorPriority['/'] = 20;
	mOperatorPriority['*'] = 20; 
}

ASTTree* Parser::createTree()
{
	getNextToken();
	if(mVariableStr == "exit")
		return NULL;
	ExprAST* root = parseExpression();
	if(!root || mError != "")
	{
		if(root)
			delete root;

		return NULL;
	}

	return new ASTTree(root);

}

ASTTree* Parser::createEmptyTree()
{
	return new ASTTree();
}

bool Parser::checkExit()
{
	if(mVariableStr == "exit")
		return true;
	return false;
}

void Parser::initialize()
{
	if(mLastChar != '\n')
	{
		std::string buf;
		getline(std::cin, buf);
	}
	mLastChar = ' ';
	mNumVal = -1;
	mVariableStr = "";
	mCurrentToken = 0;
	mError = "";
}

bool Parser::parseVariableValues(const std::string& str, ASTTree& tree)
{
	if(str == "no arguments")
		return true;

	unsigned i = 0;
	std::string name;
	std::string val;
	bool isName = true;
	while(i < str.length())
	{
		if(str[i] == '=')
		{
			isName = false;
		}
		else if(str[i] == ';')
		{
			tree.mValues[name] = atoi(val.c_str());
			name = "";
			val = "";
			isName = true;
		}
		else
		{
			if(isName)
			{
				name += str[i];
			}
			else
			{
				val += str[i];
			}
		}
		i++;
	}
	return true;
}

bool Parser::createVariableValues(std::string& str)
{
	while(true)
	{
		std::cout << ">> ";
		
		std::string buf;
		getline(std::cin, buf);

		if(buf == "calculate")
			break;
		
		if(!checkVariableValue(buf))
		{
			std::string error;
			getLastError(error);
			std::cout << "<< " + error + "\n";
			std::cout << "<< continue\n";
			continue;
		}

		std::cout << "<< OK\n";

		str += buf;
		str += ";";
	}
	if(str.empty())
	{
		str = "no arguments";
	}
	return true;
}

bool Parser::checkVariableValue(std::string& str)
{
	unsigned i = 0;
	size_t pos = str.find("=");

	if(!isalpha(str[0]))
	{
		mError = "error: variable name should start with letter";
		return false;
	}

	std::string buffer;
	
	while(i < pos)
	{
		if(isspace(str[i]))
		{
			i++;
			continue;
		}

		if(!isalnum(str[i]))
		{
			mError = "error: only letters and numbers are allowed in variable name";
			return false;
		}
		else
			buffer += str[i];
		
		i++;
	}

	buffer += "=";

	int j = 0;
	i = pos + 1;
	while(i < str.length())
	{
		if(isspace(str[i]))
		{
			if(j == 0)
			{
				i++;
				continue;
			}
			else
			{
				mError = "error: there should be no spaces in variable value";
				return false;
			}
		}

		if(isdigit(str[i]))
		{
			buffer += str[i];
			j++;
			if(j == 6)
			{
				mError = "error: variable value is out of range";
				return false;
			}
		}
		else
		{
			mError = "error: variable value is not number";
			return false;
		}

		i++;
	}
	str = buffer;
	return true;
}

bool Parser::parseServerAddress(const std::string& srvraddr, std::string& host, int& port)
{
	size_t pos = srvraddr.find(":");
	if(pos == -1)
	{
		std::cout << "error: there is no port in server address\n";
		return false;
	}

	host = srvraddr.substr(0, pos);
	unsigned i = 1;

	while(i < srvraddr.length() - pos)
	{
		if(!isdigit(srvraddr[pos + i]))
		{
			std::cout << "error: port should consist only digits\n";
			return false;
		}
		if(i == 6)
		{
			std::cout << "error port value is out of range\n";
			return false;
		}
		i++;
	}
	std::string sPort = srvraddr.substr(pos + 1, srvraddr.length());
	port = atoi(sPort.c_str());
	return true;
}

void Parser::getLastError(std::string& error)
{
	error = mError;
}

bool Parser::checkSanity()
{
	if(mNumVal > -1 || mVariableStr != "")
	{
		mError = "error: after number or variable must be an operator or )";
		return false;
	}
	return true;
}

int Parser::getToken() {
	if (mLastChar == '\n')
		return -1;

	while (isspace(mLastChar))
		mLastChar = getchar();

	if (isalpha(mLastChar)) 
	{
		if(!checkSanity())
		{
			return -1;
		}
		mVariableStr = mLastChar;
		while (isalnum((mLastChar = getchar())))
			mVariableStr += mLastChar;
		return tokenIdentifier;
	}

	if (isdigit(mLastChar))
	{
		if(!checkSanity())
		{
			return -1;
		}
		
		std::string NumStr;
		int num = 0;
		do {
			NumStr += mLastChar;
			mLastChar = getchar();
		} while (isdigit(mLastChar));
		long int val = strtol(NumStr.c_str(), NULL, 10);
		if(val > MAX_VALUE)
		{
			//std::cout << "number " << mNumVal << " is out of range\n"; 
			mError = "number " + NumStr + " is out of range";
			return tokenError;
		}
		mNumVal = val;
		return tokenNumber;
	}

	int ThisChar = mLastChar;
	mNumVal = -1;
	mVariableStr = "";
	mLastChar = getchar();
	return ThisChar;
}

int Parser::getNextToken() {
	return mCurrentToken = getToken();
}

int Parser::getTokenPriority() {
	if(mCurrentToken == -1)
	{
		return tokenError;
	}

	if (!isascii(mCurrentToken))
	{
		mError = "error: not an ascii symbol";
		return tokenError;
	}

	int tokenPriority = mOperatorPriority[mCurrentToken];
	if (tokenPriority <= 0) 
	{
		if(mCurrentToken != ')')
		{
			mError = "error: unknown operator";
		}
		return tokenError;
	}
	return tokenPriority;
}

ExprAST* Parser::parseIdentifierExpr() {
	std::string varName = mVariableStr;
	getNextToken();
	return new VariableExprAST(varName);
}

ExprAST* Parser::parseNumberExpr() {
	int num = mNumVal;
	getNextToken();
	return new NumberExprAST(num);
}

ExprAST* Parser::parseParenExpr() {
	getNextToken();
	ExprAST* expr = parseExpression();
	if (!expr) 
		return NULL;

	if (mCurrentToken != ')')
	{
		mError = "error: expected ')'";
		return NULL;
	}

	getNextToken();

	return expr;
}

ExprAST* Parser::parsePrimary() {
	switch (mCurrentToken) {
		case tokenIdentifier: 
			return parseIdentifierExpr();
		case tokenNumber:     
			return parseNumberExpr();
		case '(':            
			return parseParenExpr();
		default:
			if(mError == "")
			{
				mError = "error: incorrect symbol is entered";
			}
			return NULL;
	}
}

ExprAST* Parser::parseOperator(int exprPriority, ExprAST *leftExpr) {
	while (1) 
	{
		int tokenPriority = getTokenPriority();

		if (tokenPriority < exprPriority)
			return leftExpr;

		int BinOp = mCurrentToken;
		getNextToken(); 

		ExprAST *rightExpr = parsePrimary();
		if (!rightExpr)
			return NULL;

		int nextPriority = getTokenPriority();
		if (tokenPriority < nextPriority) {
			rightExpr = parseOperator(tokenPriority + 1, rightExpr);
			if (rightExpr == 0) 
				return NULL;
		}

		leftExpr = new BinaryExprAST(BinOp, leftExpr, rightExpr);
	}
}

ExprAST* Parser::parseExpression() {
	ExprAST *leftExpr = parsePrimary();
	if (!leftExpr) 
		return NULL;

	return parseOperator(0, leftExpr);
}
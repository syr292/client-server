#include <map>
#include <vector>
#include "ASTTree.h"

class Parser
{
	std::map<char, int> mOperatorPriority;

	enum Token {
		tokenError = -1,
		tokenIdentifier = 0, 
		tokenNumber = 1		
	};

	std::string mVariableStr;
	int mNumVal;
public:
	Parser();
	ASTTree* createTree();
		
	bool checkExit();
	void initialize();
	bool createVariableValues(std::string& str);

	static:: ASTTree* createEmptyTree();
	static bool parseServerAddress(const std::string& srvraddr, std::string& host, int& port);
	static bool parseVariableValues(const std::string& str, ASTTree& tree);
	
	void getLastError(std::string& error);
private:

	std::string mError;

	bool checkSanity();
	
	int getToken();
	int getTokenPriority();
	int getNextToken();
	
	bool checkVariableValue(std::string& str);
	
	ExprAST* parseExpression();
	ExprAST* parseIdentifierExpr();
	ExprAST* parseNumberExpr();
	ExprAST* parseParenExpr();
	ExprAST* parsePrimary();
	ExprAST* parseOperator(int exprPriority, ExprAST *leftExpr);

	int mCurrentToken;
	int mLastChar;
	
};
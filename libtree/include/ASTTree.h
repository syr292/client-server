#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <iostream>

class NumberExprAST;
class BinaryExprAST;
class VariableExprAST;

class ASTTree;

class Visitor {
protected:
	ASTTree* mTree;
public:
	Visitor(ASTTree* tree);

	virtual void visit(NumberExprAST &ref) = 0;
	virtual void visit(BinaryExprAST &ref) = 0;
	virtual void visit(VariableExprAST &ref) = 0;

	~Visitor() {}
};

class ASTTreeVisitor: public Visitor
{
	long long mRightVal;
	long long mLeftVal;
public:
	ASTTreeVisitor(ASTTree* tree);

	void visit(NumberExprAST &ref);
	void visit(BinaryExprAST &ref);
	void visit(VariableExprAST &ref);
private:
	long long doOperation(char oper);
};

class SerializeVisitor: public Visitor
{
public:
	SerializeVisitor(ASTTree* tree);
	
	void visit(NumberExprAST &ref);
	void visit(BinaryExprAST &ref);
	void visit(VariableExprAST &ref);
};

/// ExprAST - Базовый класс для всех узлов выражений.
class ExprAST {
public:
	ExprAST(ExprAST* lExpr, ExprAST* rExpr);
	virtual void accept(Visitor& v) = 0;
	ExprAST* leftExpr;
	ExprAST* rightExpr;
	long long value;
	virtual ~ExprAST() {}
};

class NumberExprAST: public ExprAST
{
	unsigned mValue;
public:
	NumberExprAST(unsigned value);
	unsigned getValue();
	void accept(Visitor& v);
};

class VariableExprAST: public ExprAST
{
	std::string mName;
public:
	VariableExprAST(const std::string &name);
	std::string getName();
	void accept(Visitor& v);
};

class BinaryExprAST: public ExprAST
{
	char mOperator;
public:
	BinaryExprAST(char oper, ExprAST *leftExpr, ExprAST *rightExpr);
	char getOperator();
	void accept(Visitor& v);
};

class ASTTree
{
	friend class Parser;

	std::map<std::string, unsigned> mValues;

	ExprAST* mRoot;
	Visitor* mCountVisitor;
	Visitor* mSerializeVisitor;

	friend class SerializeVisitor;
	friend class ASTTreeVisitor;

	std::string mSerialized;
	int mCounter;
	
public:
	~ASTTree();

	long long count();
	void serialize(std::string& serialized);
	bool deserialize(const std::string& serialized);
	bool checkData(const std::string& data);
	void getError(std::string& error);

private:
	void deleteTree(ExprAST* node);

	ASTTree();
	ASTTree(ExprAST* node);

	void serialize(ExprAST* node);
	void deserialize(ExprAST*& node);
	void count(ExprAST* node);

	std::string mLastError;
};


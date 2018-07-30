#include "ASTTree.h"

Visitor::Visitor(ASTTree* tree) :
	mTree(tree)
{
}

ASTTreeVisitor::ASTTreeVisitor(ASTTree* tree) :
	Visitor(tree),
	mRightVal(0),
	mLeftVal(0)
{	
}

void ASTTreeVisitor::visit(NumberExprAST &ref) {
	ref.value = ref.getValue();
}
void ASTTreeVisitor::visit(BinaryExprAST &ref) {
	mLeftVal = ref.leftExpr->value;
	mRightVal = ref.rightExpr->value;
	if(mLeftVal == -1 || mRightVal == -1)
	{
		ref.value = -1;
	}
	else
	{
		ref.value = doOperation(ref.getOperator());
	}
}
void ASTTreeVisitor::visit(VariableExprAST &ref) {
	if(mTree->mValues.find(ref.getName()) == mTree->mValues.end())
		ref.value = -1;
	else
		ref.value = mTree->mValues[ref.getName()];
	std::cout << ref.value + "\n";
}

long long ASTTreeVisitor::doOperation(char oper)
{
	switch(oper)
	{
	case '+':
		return mLeftVal + mRightVal;
	case '-':
		return mLeftVal - mRightVal;
	case '*':
		return mLeftVal * mRightVal;
	case '/':
		return mLeftVal / mRightVal;
	default:
		return -1;
	}
 }

SerializeVisitor::SerializeVisitor(ASTTree* tree):
	Visitor(tree)
{
}

void SerializeVisitor::visit(NumberExprAST &ref)
{
	std::string str;
	char* n = new char[16];
	_itoa_s(ref.getValue(), n, 16, 10);
	str += n;
	delete [] n;
	str += "N";
	mTree->mSerialized.append(str);
}
void SerializeVisitor::visit(BinaryExprAST &ref)
{
	std::string str;
	str.append(1, ref.getOperator());
	str.append(1, 'O');
	mTree->mSerialized.append(str);
}
void SerializeVisitor::visit(VariableExprAST &ref)
{
	std::string str(ref.getName());
	str.append(1, 'V');
	mTree->mSerialized.append(str);
}	

ExprAST::ExprAST(ExprAST* lExpr, ExprAST* rExpr) :
	leftExpr(lExpr),
	rightExpr(rExpr),
	value(0)
{
}

NumberExprAST::NumberExprAST(unsigned value) : 
	ExprAST(NULL, NULL),
	mValue(value)
{
}

unsigned NumberExprAST::getValue()
{
	return mValue;
}

void NumberExprAST::accept(Visitor& v)
{
	v.visit(*this);
}

VariableExprAST::VariableExprAST(const std::string &name) : 
	ExprAST(NULL, NULL),
	mName(name)
{
}

std::string VariableExprAST::getName()
{
	return mName;
}

void VariableExprAST::accept(Visitor& v)
{
	v.visit(*this);
}

BinaryExprAST::BinaryExprAST(char oper, ExprAST *leftExpr, ExprAST *rightExpr) :
	ExprAST(leftExpr, rightExpr),	
	mOperator(oper)
{
}
char BinaryExprAST::getOperator()
{
	return mOperator;
}

void BinaryExprAST::accept(Visitor& v)
{
	v.visit(*this);
}

ASTTree::ASTTree() :
	mRoot(NULL),
	mSerialized(),
	mCounter(0)
	{
		mCountVisitor = new ASTTreeVisitor(this);
		mSerializeVisitor = new SerializeVisitor(this);
	}

ASTTree::ASTTree(ExprAST* node) :
	mRoot(node),
	mSerialized(),
	mCounter(0),
	mLastError()
{
	mCountVisitor = new ASTTreeVisitor(this);
	mSerializeVisitor = new SerializeVisitor(this);
}

ASTTree::~ASTTree()
{
	deleteTree(mRoot);
	delete mCountVisitor;
	delete mSerializeVisitor;
}

void ASTTree::deleteTree(ExprAST* node)
{
	if (node != NULL)
	{
	   deleteTree(node->leftExpr);
	   deleteTree(node->rightExpr);
	   delete node;
	}
}

long long ASTTree::count()
{
	count(mRoot);
	return mRoot->value;
}

void ASTTree::serialize(std::string& serialized)
{
	serialize(mRoot);
	mSerialized += ";";
	serialized = mSerialized;
}

bool ASTTree::deserialize(const std::string& serialized)
{
	mSerialized = serialized;
	int in = mSerialized.length();
	deserialize(mRoot);
	return true;
}

bool ASTTree::checkData(const std::string& data)
{
	if(data.find(";") == -1)
	{
		return false;
	}
	return true;
}
	
void ASTTree::getError(std::string& error)
{
	error = mLastError;
}

void ASTTree::serialize(ExprAST* node)
{
	if(node != NULL)
	{
		node->accept(*mSerializeVisitor);
		serialize(node->leftExpr);
		serialize(node->rightExpr);
	}
}
void ASTTree::deserialize(ExprAST*& node)
{
	std::string buf;
	int locCounter = 0;
	while(1)
	{
		mCounter++;
		if(mSerialized[mCounter - 1] == 'N')
		{
			node = new NumberExprAST(atoi(buf.c_str()));
			return;
		}
		if(mSerialized[mCounter - 1] == 'V')
		{
			node = new VariableExprAST(buf.c_str());
			return;
		}
		if(mSerialized[mCounter - 1] == 'O')
		{
			node = new BinaryExprAST(buf[locCounter - 1], NULL, NULL);
			break;
		}
		buf.append(1, mSerialized[mCounter - 1]);
		locCounter++;
	}
	deserialize(node->leftExpr);
	deserialize(node->rightExpr);
}
void ASTTree::count(ExprAST* node)
{
	if (node != NULL)
	{
	   count(node->leftExpr);
	   count(node->rightExpr);
	   node->accept(*mCountVisitor);
	}
}


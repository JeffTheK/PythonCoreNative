
#include <ast/NamedExprNode.h>

using namespace PythonCoreNative::RunTime::Parser::AST;

NamedExprNode::NamedExprNode(  
                            unsigned int start, unsigned int end, 
                            std::shared_ptr<ExpressionNode> left,
                            std::shared_ptr<Token> op1,
                            std::shared_ptr<ExpressionNode> right
                        ) : ExpressionNode(start, end)
{
    mLeft = left;
    mOp1 = op1;
    mRight = right;
}

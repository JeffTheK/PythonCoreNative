
#include <ast/UnaryPlusNode.h>

using namespace PythonCoreNative::RunTime::Parser::AST;

UnaryPlusNode::UnaryPlusNode(  
                            unsigned int start, unsigned int end, 
                            std::shared_ptr<Token> op1,
                            std::shared_ptr<ExpressionNode> right
                        ) : ExpressionNode(start, end)
{
    mOp1 = op1;
    mRight = right;
}

#pragma once

#include <ast/ExpressionNode.h>
#include <Token.h>

#include <memory>

namespace PythonCoreNative::RunTime::Parser::AST
{
    class BitXorNode : public ExpressionNode
    {
        public:
            BitXorNode(  
                            unsigned int start, unsigned int end, 
                            std::shared_ptr<ExpressionNode> left,
                            std::shared_ptr<Token> op1,
                            std::shared_ptr<ExpressionNode> right
                        );

        protected:
            std::shared_ptr<ExpressionNode> mLeft;
            std::shared_ptr<Token> mOp1;
            std::shared_ptr<ExpressionNode> mRight;
    };
}


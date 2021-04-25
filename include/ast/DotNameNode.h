
#include <ast/ExpressionNode.h>
#include <Token.h>

#include <memory>

namespace PythonCoreNative::RunTime::Parser::AST
{
    class DotNameNode : protected ExpressionNode
    {
        public:
            DotNameNode(  
                            unsigned int start, unsigned int end, 
                            std::shared_ptr<Token> op1,
                            std::shared_ptr<NameToken> op2
                        );

        protected:
            std::shared_ptr<Token> mOp1;
            std::shared_ptr<NameToken> mOp2;
    };
}

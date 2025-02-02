#pragma once

#include <Token.h>

#include <exception>
#include <string>
#include <memory>

namespace PythonCoreNative::RunTime::Parser
{
    class SyntaxError : public std::exception
    {
        public:
            SyntaxError(unsigned int position, std::shared_ptr<Token> curSymbol, std::shared_ptr<std::wstring> msg);

        protected:
            unsigned int mPosition;
            std::shared_ptr<Token> mSymbol;
            std::shared_ptr<std::wstring> mMsg;
    };
}

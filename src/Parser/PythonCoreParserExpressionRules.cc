
#include <PythonCoreParser.h>

#include <typeinfo>

using namespace PythonCoreNative::RunTime::Parser;

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseAtom()
{
    auto startPos = mLexer->Position();
    auto curSymbol = mLexer->CurSymbol();

    switch (curSymbol->GetSymbolKind())
    {
        case TokenKind::PyFalse:
            mLexer->Advance();
            return std::make_shared<AST::AtomFalseNode>(startPos, mLexer->Position(), curSymbol);
        case TokenKind::PyTrue:
            mLexer->Advance();
            return std::make_shared<AST::AtomTrueNode>(startPos, mLexer->Position(), curSymbol);
        case TokenKind::PyNone:
            mLexer->Advance();
            return std::make_shared<AST::AtomNoneNode>(startPos, mLexer->Position(), curSymbol);
        case TokenKind::PyElipsis:
            mLexer->Advance();
            return std::make_shared<AST::AtomElipsisNode>(startPos, mLexer->Position(), curSymbol);
        case TokenKind::Name:
            mLexer->Advance();
            return std::make_shared<AST::AtomNameNode>(startPos, mLexer->Position(), std::static_pointer_cast<NameToken>(curSymbol));
        case TokenKind::Number:
            mLexer->Advance();
            return std::make_shared<AST::AtomNumberNode>(startPos, mLexer->Position(), std::static_pointer_cast<NumberToken>(curSymbol));
        case TokenKind::String:
            {
                auto lst = std::make_shared<std::vector<std::shared_ptr<StringToken>>>();
                while (curSymbol->GetSymbolKind() == TokenKind::String)
                {
                    lst->push_back(std::static_pointer_cast<StringToken>(curSymbol));
                    mLexer->Advance();
                    curSymbol = mLexer->CurSymbol();
                }
                return std::make_shared<AST::AtomStringNode>(startPos, mLexer->Position(), lst);
            }
        case TokenKind::PyLeftParen:
            {
                mLexer->Advance();
                if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightParen)
                {
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    return std::make_shared<AST::AtomTupleNode>(startPos, mLexer->Position(), curSymbol, nullptr, symbol2);
                }
                if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyYield)
                {
                    auto node = ParseYieldExpr();
                    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightParen) 
                        throw std::make_shared<SyntaxError>(startPos, curSymbol, std::make_shared<std::basic_string<char32_t>>(U"Missing ')' in tuple!"));
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    return std::make_shared<AST::AtomTupleNode>(startPos, mLexer->Position(), curSymbol, node, symbol2);
                }
                else
                {
                    auto node = ParseTestListComp();
                    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightParen) 
                        throw std::make_shared<SyntaxError>(startPos, curSymbol, std::make_shared<std::basic_string<char32_t>>(U"Missing ')' in tuple!"));
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    return std::make_shared<AST::AtomTupleNode>(startPos, mLexer->Position(), curSymbol, node, symbol2);
                }
            }
        case TokenKind::PyLeftBracket:
            {
                mLexer->Advance();
                if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightBracket)
                {
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    return std::make_shared<AST::AtomListNode>(startPos, mLexer->Position(), curSymbol, nullptr, symbol2);
                }
                else
                {
                    auto node = ParseTestListComp();
                    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightBracket) 
                        throw std::make_shared<SyntaxError>(startPos, curSymbol, std::make_shared<std::basic_string<char32_t>>(U"Missing ']' in list!"));
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    return std::make_shared<AST::AtomListNode>(startPos, mLexer->Position(), curSymbol, node, symbol2);
                }
            }
        case TokenKind::PyLeftCurly:
            {
                mLexer->Advance();
                if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightCurly)
                {
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    return std::make_shared<AST::AtomDictionaryNode>(startPos, mLexer->Position(), curSymbol, nullptr, symbol2);
                }
                else
                {
                    auto node = ParseDictorSetMaker();
                    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightCurly) 
                        throw std::make_shared<SyntaxError>(startPos, curSymbol, std::make_shared<std::basic_string<char32_t>>(U"Missing '}' in dictionary!"));
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    if (typeid(node) == typeid(AST::AtomSetNode))
                    {
                        return std::make_shared<AST::AtomSetNode>(startPos, mLexer->Position(), curSymbol, node, symbol2);
                    }
                    return std::make_shared<AST::AtomDictionaryNode>(startPos, mLexer->Position(), curSymbol, node, symbol2);
                }
            }
        default:
            throw std::make_shared<SyntaxError>(startPos, curSymbol, std::make_shared<std::basic_string<char32_t>>(U"Illegal literal found!"));
    }
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseAtomExpr()
{
    auto startPos = mLexer->Position();

    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyAwait)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto node = ParseAtom();
        auto lst = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
        while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLeftParen ||
                mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLeftBracket ||
                mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyDot)
                {
                    lst->push_back(ParseTrailer());
                }
        return std::make_shared<AST::AtomExprNode>(startPos, mLexer->Position(), symbol, node, lst->size() == 0 ? nullptr : lst);
    }
    else
    {
        auto node = ParseAtom();
        if (    mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLeftParen ||
                mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLeftBracket ||
                mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyDot)
                {
                    auto lst = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
                    lst->push_back(ParseTrailer());
                    while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLeftParen ||
                            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLeftBracket ||
                            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyDot)
                            {
                                lst->push_back(ParseTrailer());
                            }
                    return std::make_shared<AST::AtomExprNode>(startPos, mLexer->Position(), nullptr, node, lst);
                }
        return node;
    }
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParsePower()
{
    auto startPos = mLexer->Position();
    auto left = ParseAtomExpr();

    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyPower)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseFactor();
        return std::make_shared<AST::PowerNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseFactor()
{
    auto startPos = mLexer->Position();
    auto symbol = mLexer->CurSymbol();

    switch (symbol->GetSymbolKind())
    {
        case TokenKind::PyPlus:
            {
                mLexer->Advance();
                auto rightPlus = ParseFactor();
                return std::make_shared<AST::UnaryPlusNode>(startPos, mLexer->Position(), symbol, rightPlus);
            }
        case TokenKind::PyMinus:
            {
                mLexer->Advance();
                auto rightMinus = ParseFactor();
                return std::make_shared<AST::UnaryMinusNode>(startPos, mLexer->Position(), symbol, rightMinus);
            }
        case TokenKind::PyBitInvert:
            {
                mLexer->Advance();
                auto rightInvert = ParseFactor();
                return std::make_shared<AST::UnaryBitInvertNode>(startPos, mLexer->Position(), symbol, rightInvert);
            }
        default:
            return ParsePower();
    }
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTerm()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseArith()
{
    return nullptr;
}


std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseShift()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseAndExpr()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseXorExpr()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseOrExpr()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseStarExpr()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseComparison()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseNotTest()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseAndTest()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseOrTest()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseLambda(bool isCond)
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTestNoCond()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTest()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseNamedExpr()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTestListComp()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTrailer()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseSubscriptList()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseSubscript()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseExprList()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTestList()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseDictorSetMaker()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseArgList()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseArgument()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseCompIter()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseSyncCompFor()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseCompFor()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseCompIf()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseYieldExpr()
{
    return std::make_shared<AST::ExpressionNode>(0, 0);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseVarArgsList()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseVFPAssign()
{
    return nullptr;
}

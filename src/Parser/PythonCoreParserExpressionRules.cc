
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
    auto startPos = mLexer->Position();
    auto left = ParseFactor();

    while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMul ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyDiv ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyModulo ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMatrice ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyFloorDiv)
            {
                auto symbol = mLexer->CurSymbol();
                mLexer->Advance();
                auto right = ParseFactor();
                switch (symbol->GetSymbolKind())
                {
                    case TokenKind::PyMul:
                        left = std::make_shared<AST::MulNode>(startPos, mLexer->Position(), left, symbol, right);
                        break;
                    case TokenKind::PyDiv:
                        left = std::make_shared<AST::DivNode>(startPos, mLexer->Position(), left, symbol, right);
                        break;
                    case TokenKind::PyModulo:
                        left = std::make_shared<AST::ModuloNode>(startPos, mLexer->Position(), left, symbol, right);
                        break;
                    case TokenKind::PyMatrice:
                        left = std::make_shared<AST::MatriceNode>(startPos, mLexer->Position(), left, symbol, right);
                        break;
                    case TokenKind::PyFloorDiv:
                        left = std::make_shared<AST::FloorDivNode>(startPos, mLexer->Position(), left, symbol, right);
                        break;
                    default:    break;
                }
            }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseArith()
{
    auto startPos = mLexer->Position();
    auto left = ParseTerm();

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyPlus || mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMinus)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseTerm();

        if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyPlus)
        {
            left = std::make_shared<AST::PlusNode>(startPos, mLexer->Position(), left, symbol, right);
        }
        else
        {
            left = std::make_shared<AST::MinusNode>(startPos, mLexer->Position(), left, symbol, right);
        }
    }

    return left;
}


std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseShift()
{
    auto startPos = mLexer->Position();
    auto left = ParseArith();

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyShiftLeft || mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyShiftRight)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseArith();

        if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyShiftLeft)
        {
            left = std::make_shared<AST::ShiftLeftNode>(startPos, mLexer->Position(), left, symbol, right);
        }
        else
        {
            left = std::make_shared<AST::ShiftRightNode>(startPos, mLexer->Position(), left, symbol, right);
        }
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseAndExpr()
{
    auto startPos = mLexer->Position();
    auto left = ParseShift();

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyBitAnd)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseShift();

        left = std::make_shared<AST::BitAndNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseXorExpr()
{
    auto startPos = mLexer->Position();
    auto left = ParseAndExpr();

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyBitXor)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseAndExpr();

        left = std::make_shared<AST::BitXorNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseOrExpr()
{
    auto startPos = mLexer->Position();
    auto left = ParseXorExpr();

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyBitOr)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseXorExpr();

        left = std::make_shared<AST::BitOrNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseStarExpr()
{
    auto startPos = mLexer->Position();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyMul)
        throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Missing '*' in star expression!"));

    auto symbol = mLexer->CurSymbol();
    auto right = ParseOrExpr();
    return std::make_shared<AST::StarExprNode>(startPos, mLexer->Position(), symbol, right);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseComparison()
{
    auto startPos = mLexer->Position();
    auto left = ParseOrExpr();

    while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLess ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLessEqual ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyEqual ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyGreater ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyGreaterEqual ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyNotEqual ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyIn ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyNot ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyIs )
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();

        switch (symbol->GetSymbolKind())
        {
            case TokenKind::PyLess:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareLessNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyLessEqual:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareLessEqualNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyEqual:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareEqualNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyGreater:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareGreaterNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyGreaterEqual:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareGreaterEqualNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyIn:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareInNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyNotEqual:
                {
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareNotEqualNode>(startPos, mLexer->Position(), left, symbol, right);
                }
                break;
            case TokenKind::PyIs:
                {
                    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyNot)
                    {
                        auto symbol2 = mLexer->CurSymbol();
                        mLexer->Advance();
                        auto right = ParseOrExpr();
                        left = std::make_shared<AST::CompareIsNotNode>(startPos, mLexer->Position(), left, symbol, symbol2, right);
                    }
                    else
                    {
                        auto right = ParseOrExpr();
                        left = std::make_shared<AST::CompareIsNode>(startPos, mLexer->Position(), left, symbol, right);
                    }
                }
                break;
            case TokenKind::PyNot:
                {
                    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyIn)
                        throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Missing 'in' after 'not' in 'not in' operator!"));
                    
                    auto symbol2 = mLexer->CurSymbol();
                    mLexer->Advance();
                    auto right = ParseOrExpr();
                    left = std::make_shared<AST::CompareNotInNode>(startPos, mLexer->Position(), left, symbol, symbol2, right);
                }
                break;
            default:    break;
        }
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseNotTest()
{
    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyNot)
    {
        auto startPos = mLexer->Position();
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseNotTest();

        return std::make_shared<AST::NotTestNode>(startPos, mLexer->Position(), symbol, right);
    }

    return ParseComparison();
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseAndTest()
{
    auto startPos = mLexer->Position();
    auto left = ParseNotTest();

    while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyAnd)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseNotTest();

        left = std::make_shared<AST::AndTestNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseOrTest()
{
    auto startPos = mLexer->Position();
    auto left = ParseAndTest();

    while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyOr)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseAndTest();

        left = std::make_shared<AST::OrTestNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseLambda(bool isCond)
{
    auto startPos = mLexer->Position();
    auto symbol = mLexer->CurSymbol();
    mLexer->Advance();
    auto left = mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyColon ? nullptr : ParseVarArgsList();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyColon)
        throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Missing ':' in 'lambda' expression!"));

    auto symbol2 = mLexer->CurSymbol();
    mLexer->Advance();
    auto right = isCond ? ParseTest() : ParseTestNoCond();

    return std::make_shared<AST::LambdaNode>(startPos, mLexer->Position(), symbol, left, symbol2, right);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTestNoCond()
{
    return mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLambda ? ParseLambda(false) : ParseOrTest();
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTest()
{
    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyLambda) return ParseLambda(true);

    auto startPos = mLexer->Position();
    auto left = ParseOrTest();

    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyIf)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseOrTest();

        if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyElse)
            throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Missing 'else' in 'test expression!"));

        auto symbol2 = mLexer->CurSymbol();
        mLexer->Advance();
        auto next = ParseTest();

        return std::make_shared<AST::TestNode>(startPos, mLexer->Position(), left, symbol, right, symbol2, next);
    }

    return left;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseNamedExpr()
{
    auto startPos = mLexer->Position();
    auto left = ParseTest();

    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyColonAssign)
    {
        auto symbol = mLexer->CurSymbol();
        mLexer->Advance();
        auto right = ParseTest();

        return std::make_shared<AST::NamedExprNode>(startPos, mLexer->Position(), left, symbol, right);
    }

    return left;;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTestListComp()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back(mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMul ? ParseStarExpr() : ParseNamedExpr());
    
    if (    mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyAsync ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyFor )
            {
                nodes->push_back( ParseCompIter() );
            }
    else
    {
        while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma )
        {
            separators->push_back( mLexer->CurSymbol() );
            mLexer->Advance();
            if (    mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightParen ||
                    mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightBracket )
                    {
                        nodes->push_back(mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMul ? ParseStarExpr() : ParseNamedExpr());
                    }
            else if ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma  )
                throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Expecting item in testlist!"));
        }
    }

    if ( nodes->size() == 1 && separators->size() == 0 ) return nodes->back();
    
    return std::make_shared<AST::TestListCompNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTrailer()
{
    auto startPos = mLexer->Position();
    auto symbol = mLexer->CurSymbol();
    mLexer->Advance();

    switch (symbol->GetSymbolKind())
    {
        case TokenKind::PyLeftParen:
            {
                auto right = mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightParen ? ParseVarArgsList() : nullptr;

                if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightParen)
                    throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Expecting ')' in call!"));

                auto symbol2 = mLexer->CurSymbol();
                mLexer->Advance();

                return std::make_shared<AST::CallNode>(startPos, mLexer->Position(), symbol, right, symbol2);
            }
        case TokenKind::PyLeftBracket:
            {
                auto right = mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightBracket ? ParseSubscriptList() : nullptr;

                if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightBracket)
                    throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Expecting ']' in subscript!"));

                auto symbol2 = mLexer->CurSymbol();
                mLexer->Advance();

                return std::make_shared<AST::IndexNode>(startPos, mLexer->Position(), symbol, right, symbol2);
            }
        default:    // Dot Name
            {
                if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightBracket)
                    throw std::make_shared<SyntaxError>(startPos, mLexer->CurSymbol(), std::make_shared<std::basic_string<char32_t>>(U"Expecting Name literal after '.'!"));
            
                auto symbol2 = mLexer->CurSymbol();
                mLexer->Advance();

                return std::make_shared<AST::DotNameNode>(startPos, mLexer->Position(), symbol, std::static_pointer_cast<NameToken>( symbol2 ) );
            }
            break;
    }
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseSubscriptList()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back( ParseSubscript() );

    while ( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma )
    {
        separators->push_back( mLexer->CurSymbol() );
        mLexer->Advance();
        if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightBracket) break;
        nodes->push_back( ParseSubscript() );
    }

    if (nodes->size() == 1 && separators->size() == 0) return nodes->back();

    return std::make_shared<AST::SubscriptListNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseSubscript()
{
    auto startPos = mLexer->Position();
    std::shared_ptr<AST::ExpressionNode> first = nullptr, second = nullptr, third = nullptr;
    std::shared_ptr<Token> one = nullptr, two = nullptr;

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyColon) first = ParseTest();
    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyColon)
    {
        one = mLexer->CurSymbol();
        mLexer->Advance();
        if (    mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyComma ||
                mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyColon ||
                mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightBracket ) second = ParseTest();
        if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyColon)
        {
            two = mLexer->CurSymbol();
            mLexer->Advance();
            if (    mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyComma ||
                    mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightBracket ) third = ParseTest();
        }
    }

    return std::make_shared<AST::SubscriptNode>(startPos, mLexer->Position(), first, one, second, two, third);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseExprList()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMul ? ParseStarExpr() : ParseOrExpr() );

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma)
    {
        separators->push_back(mLexer->CurSymbol());
        mLexer->Advance();
        if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyIn) break;
        nodes->push_back( mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyMul ? ParseStarExpr() : ParseOrExpr() );
    }

    if (nodes->size() == 1 && separators->size() == 0) return nodes->back();

    return std::make_shared<AST::ExprListNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseTestList()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back(ParseTest());

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma)
    {
        separators->push_back(mLexer->CurSymbol());
        mLexer->Advance();
        if (    mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PySemiColon ||
                mLexer->CurSymbol()->GetSymbolKind() == TokenKind::Newline ||
                mLexer->CurSymbol()->GetSymbolKind() == TokenKind::EndOfFile) break;
        nodes->push_back(ParseTest());
    }

    if (nodes->size() == 1 && separators->size() == 0) return nodes->back();

    return std::make_shared<AST::TestListNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseDictorSetMaker()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseArgList()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::ExpressionNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back(ParseArgument());

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma)
    {
        separators->push_back(mLexer->CurSymbol());
        mLexer->Advance();
        if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyRightParen) break;
        nodes->push_back(ParseArgument());
    }

    if (nodes->size() == 1 && separators->size() == 0) return nodes->back();

    return std::make_shared<AST::ArgsListNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseArgument()
{
    return nullptr;
}

std::shared_ptr<AST::ExpressionNode> PythonCoreParser::ParseCompIter()
{
    return  mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyAsync ||
            mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyFor ?
                ParseCompFor() : ParseCompIf();
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

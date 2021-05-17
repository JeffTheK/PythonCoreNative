
#include <PythonCoreParser.h>

using namespace PythonCoreNative::RunTime::Parser;

// This is the rules for Match statement introduced in Python 3.10

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseMatch()
{
    auto startPos = mLexer->Position();
    auto symbol = mLexer->CurSymbol(); /* Identifier 'match' */
    mLexer->Advance();

    auto left = ParseSubjectExpr();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyColon)
        throw std::make_shared<SyntaxError>(
            mLexer->Position(), 
            mLexer->CurSymbol(),
            std::make_shared<std::wstring>(L"Expecting ':' in 'match' statement!"));

    auto symbol2 = mLexer->CurSymbol();
    mLexer->Advance();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::Newline)
        throw std::make_shared<SyntaxError>(
            mLexer->Position(), 
            mLexer->CurSymbol(),
            std::make_shared<std::wstring>(L"Expecting Newline in 'match' statement!"));

    auto symbol3 = mLexer->CurSymbol();
    mLexer->Advance();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::Indent)
        throw std::make_shared<SyntaxError>(
            mLexer->Position(), 
            mLexer->CurSymbol(),
            std::make_shared<std::wstring>(L"Expecting Indent in 'match' statement!"));

    auto symbol4 = mLexer->CurSymbol();
    mLexer->Advance();

    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::StatementNode>>>();

    do
    {
        
        nodes->push_back( ParseCaseBlock() );

    } while (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::Dedent);
    
    auto symbol5 = mLexer->CurSymbol();
    mLexer->Advance();

    return std::make_shared<AST::MatchStatementNode>(
        startPos,
        mLexer->Position(),
        symbol,     /* 'match' */
        left, 
        symbol2,    /* ':' */
        symbol3,    /* 'NEWLINE' */
        symbol4,    /* 'INDENT' */
        nodes,
        symbol5 );  /* 'DEDENT' */
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseSubjectExpr()
{
    auto startPos = mLexer->Position();
    auto right = ParseStarNamedExpression();

    return std::make_shared<AST::SubjectExprNode>(startPos, mLexer->Position(), right);
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseCaseBlock()
{
    auto startPos = mLexer->Position();

    if (    mLexer->CurSymbol()->GetSymbolKind() == TokenKind::Name && 
            std::static_pointer_cast<NameToken> (mLexer->CurSymbol())->IsCaseSoftKeyword() )
            {

                auto symbol = mLexer->CurSymbol();
                mLexer->Advance();

                auto left = ParsePatterns();

                auto right = mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyIf ?
                                ParseGuard() : nullptr;

                if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyColon)
                    throw std::make_shared<SyntaxError>(
                                mLexer->Position(), 
                                mLexer->CurSymbol(),
                                std::make_shared<std::wstring>(L"Expecting ':' in 'case' statement!"));

                auto symbol2 = mLexer->CurSymbol();
                mLexer->Advance();

                auto next = ParseSuite();

                return std::make_shared<AST::CaseStatementNode>(
                        startPos,
                        mLexer->Position(),
                        std::static_pointer_cast<NameToken>(symbol), 
                        left, 
                        right, 
                        symbol2, 
                        next);

            }

    else
        throw std::make_shared<SyntaxError>(
            mLexer->Position(), 
            mLexer->CurSymbol(),
            std::make_shared<std::wstring>(L"Expecting 'case' in 'match' statement!"));

}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseGuard()
{
    auto startPos = mLexer->Position();
    auto symbol = mLexer->CurSymbol();  /* 'if' */
    mLexer->Advance();

    auto right = ParseNamedExpr();

    return std::make_shared<AST::GuardNode>(startPos, mLexer->Position(), symbol, right);
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParsePatterns()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParsePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseAsPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseOrPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseClosedPattern()
{
    return nullptr;
}
std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseLiteralPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseLiteralExpr()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseComplexNumber()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseSignedNumber()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseSignedRealNumber()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseRealNumber()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseImaginaryNumber()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseCapturePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParsePatternCaptureTarget()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseWildCardPattern()
{
    auto startPos = mLexer->Position();

    if (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::Name && std::static_pointer_cast<NameToken>(mLexer->CurSymbol())->IsWildCardPattern() )
    {

        auto symbol = mLexer->CurSymbol();  /* '_' */
        mLexer->Advance();

        return std::make_shared<AST::WildCardPatternNode>(startPos, mLexer->Position(), symbol);

    }

    return nullptr; /* Should never happend, because we need '_' to get to this rule. */
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseValuePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseAttr()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseNameOrAttr()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseGroupPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseSequencePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseOpenSequencePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseMaybeeSequencePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseMaybeeStarExpr()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseStarPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseMappingPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseItemsPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseKeyValuePattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseDoubleStarPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseClassPattern()
{
    return nullptr;
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParsePositionalPattern()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::StatementNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back( ParsePattern() );

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma)
    {
        separators->push_back( mLexer->CurSymbol() );
        mLexer->Advance();

        if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightParen)
            nodes->push_back( ParsePattern() );
    }

    return std::make_shared<AST::PositionalPatternsNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseKeywordPatterns()
{
    auto startPos = mLexer->Position();
    auto nodes = std::make_shared<std::vector<std::shared_ptr<AST::StatementNode>>>();
    auto separators = std::make_shared<std::vector<std::shared_ptr<Token>>>();

    nodes->push_back( ParseKeywordPattern() );

    while (mLexer->CurSymbol()->GetSymbolKind() == TokenKind::PyComma)
    {
        separators->push_back( mLexer->CurSymbol() );
        mLexer->Advance();

        if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyRightParen)
            nodes->push_back( ParseKeywordPattern() );
    }

    return std::make_shared<AST::KeywordPatternsNode>(startPos, mLexer->Position(), nodes, separators);
}

std::shared_ptr<AST::StatementNode> PythonCoreParser::ParseKeywordPattern()
{
    auto startPos = mLexer->Position();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::Name)
        throw std::make_shared<SyntaxError>(
            mLexer->Position(), 
            mLexer->CurSymbol(),
            std::make_shared<std::wstring>(L"Expecting Name in keyword pattern!"));

    auto symbol = std::static_pointer_cast<NameToken>( mLexer->CurSymbol() );
    mLexer->Advance();

    if (mLexer->CurSymbol()->GetSymbolKind() != TokenKind::PyAssign)
        throw std::make_shared<SyntaxError>(
            mLexer->Position(), 
            mLexer->CurSymbol(),
            std::make_shared<std::wstring>(L"Expecting '=' in keyword pattern!"));

    auto symbol2 = mLexer->CurSymbol();
    mLexer->Advance();

    auto right = ParsePattern();

    return std::make_shared<AST::KeywordPatternNode>(
                            startPos, mLexer->Position(), symbol, symbol2, right);
}

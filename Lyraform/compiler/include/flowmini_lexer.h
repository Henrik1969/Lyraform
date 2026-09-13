#ifndef FLOWMINI_LEXER_H
#define FLOWMINI_LEXER_H

#include <string>
#include <vector>

namespace flowmini {

    enum class TokenKind {
        Identifier,
        Number,
        String,
        KeywordModule,
        KeywordProgram,
        KeywordUnit,
        KeywordProducer,
        KeywordNode,
        KeywordSink,
        KeywordWire,
        KeywordPolicy,
        KeywordState,
        KeywordPersistent,
        KeywordTarget,
        KeywordMain,
        KeywordFn,
        KeywordScope,
        KeywordWhile,
        KeywordIf,
        KeywordElse,
        KeywordBreak,
        KeywordContinue,
        KeywordPrint,
        KeywordTrue,
        KeywordFalse,
        Colon,
        Dot,
        Equals,
        Arrow, // => graph flow
        PlaceArrow, // -> expression placement / assignment
        LeftParen,
        RightParen,
        LeftBracket,
        RightBracket,
        LeftBrace,
        RightBrace,
        Less,
        Greater,
        GreaterEqual,
        LessEqual,
        EqualEqual,
        BangEqual,
        Comma,
        Plus,
        Minus,
        Star,
        Slash,
        Percent,
        Newline,
        End
    };

    struct Token {
    TokenKind kind;
    std::string text;
    int line = 1;
    int column = 1;
};

[[nodiscard]] const char* tokenKindName(TokenKind kind);
[[nodiscard]] std::vector<Token> lexSource(const std::string& source);

} // namespace flowmini

#endif // FLOWMINI_LEXER_H

#include "flowmini_lexer.h"

#include "flow_common.h"

#include <cctype>
#include <utility>

namespace flowmini {

const char* tokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind::Identifier:      return "identifier";
        case TokenKind::Number:          return "number";
        case TokenKind::String:          return "string";
        case TokenKind::KeywordModule:   return "module";
        case TokenKind::KeywordProgram:  return "program";
        case TokenKind::KeywordUnit:     return "unit";
        case TokenKind::KeywordProducer: return "producer";
        case TokenKind::KeywordNode:     return "node";
        case TokenKind::KeywordSink:     return "sink";
        case TokenKind::KeywordWire:     return "wire";
        case TokenKind::KeywordPolicy:   return "policy";
        case TokenKind::KeywordState:    return "state";
        case TokenKind::KeywordPersistent:return "persistent";
        case TokenKind::KeywordTarget:   return "target";
        case TokenKind::KeywordMain:     return "main";
        case TokenKind::KeywordFn:       return "fn";
        case TokenKind::KeywordScope:    return "scope";
        case TokenKind::KeywordWhile:    return "while";
        case TokenKind::KeywordIf:       return "if";
        case TokenKind::KeywordElse:     return "else";
        case TokenKind::KeywordBreak:    return "break";
        case TokenKind::KeywordContinue: return "continue";
        case TokenKind::KeywordPrint:    return "print";
        case TokenKind::KeywordTrue:     return "true";
        case TokenKind::KeywordFalse:    return "false";
        case TokenKind::Colon:           return ":";
        case TokenKind::Dot:             return ".";
        case TokenKind::Equals:          return "=";
        case TokenKind::Arrow:           return "=>";
        case TokenKind::PlaceArrow:      return "->";
        case TokenKind::LeftParen:       return "(";
        case TokenKind::RightParen:      return ")";
        case TokenKind::LeftBracket:     return "[";
        case TokenKind::RightBracket:    return "]";
        case TokenKind::LeftBrace:       return "{";
        case TokenKind::RightBrace:      return "}";
        case TokenKind::Less:            return "<";
        case TokenKind::Greater:         return ">";
        case TokenKind::GreaterEqual:    return ">=";
        case TokenKind::LessEqual:       return "<=";
        case TokenKind::EqualEqual:      return "==";
        case TokenKind::BangEqual:       return "!=";
        case TokenKind::Comma:           return ",";
        case TokenKind::Plus:            return "+";
        case TokenKind::Minus:           return "-";
        case TokenKind::Star:            return "*";
        case TokenKind::Slash:           return "/";
        case TokenKind::Percent:         return "%";
        case TokenKind::Newline:         return "newline";
        case TokenKind::End:             return "end";
    }

    return "unknown";
}

namespace {

[[nodiscard]] bool isIdentStart(unsigned char c) {
    return std::isalpha(c) || c == '_';
}

[[nodiscard]] bool isIdentBody(unsigned char c) {
    return std::isalnum(c) || c == '_' || c == '-';
}

[[nodiscard]] TokenKind keywordOrIdentifier(const std::string& text) {
    if (text == "module") { return TokenKind::KeywordModule; }
    if (text == "program") { return TokenKind::KeywordProgram; }
    if (text == "unit") { return TokenKind::KeywordUnit; }
    if (text == "producer") { return TokenKind::KeywordProducer; }
    if (text == "node") { return TokenKind::KeywordNode; }
    if (text == "sink") { return TokenKind::KeywordSink; }
    if (text == "wire") { return TokenKind::KeywordWire; }
    if (text == "policy") { return TokenKind::KeywordPolicy; }
    if (text == "state") { return TokenKind::KeywordState; }
    if (text == "persistent") { return TokenKind::KeywordPersistent; }
    if (text == "target") { return TokenKind::KeywordTarget; }
    if (text == "main") { return TokenKind::KeywordMain; }
    if (text == "fn") { return TokenKind::KeywordFn; }
    if (text == "scope") { return TokenKind::KeywordScope; }
    if (text == "while") { return TokenKind::KeywordWhile; }
    if (text == "if") { return TokenKind::KeywordIf; }
    if (text == "else") { return TokenKind::KeywordElse; }
    if (text == "break") { return TokenKind::KeywordBreak; }
    if (text == "continue") { return TokenKind::KeywordContinue; }
    if (text == "true") { return TokenKind::KeywordTrue; }
    if (text == "false") { return TokenKind::KeywordFalse; }
    return TokenKind::Identifier;
}

void validateUtf8(const std::string& source) {
    const auto continuation = [&](std::size_t index) {
        return index < source.size() &&
               (static_cast<unsigned char>(source[index]) & 0xc0U) == 0x80U;
    };
    for (std::size_t index = 0; index < source.size();) {
        const auto byte = static_cast<unsigned char>(source[index]);
        std::size_t length = 0;
        if (byte <= 0x7fU) {
            length = 1;
        } else if (byte >= 0xc2U && byte <= 0xdfU) {
            length = 2;
        } else if (byte >= 0xe0U && byte <= 0xefU) {
            length = 3;
            if (byte == 0xe0U) {
                if (index + 1 >= source.size() || static_cast<unsigned char>(source[index + 1]) < 0xa0U) length = 0;
            } else if (byte == 0xedU) {
                if (index + 1 >= source.size() || static_cast<unsigned char>(source[index + 1]) > 0x9fU) length = 0;
            }
        } else if (byte >= 0xf0U && byte <= 0xf4U) {
            length = 4;
            if (byte == 0xf0U) {
                if (index + 1 >= source.size() || static_cast<unsigned char>(source[index + 1]) < 0x90U) length = 0;
            } else if (byte == 0xf4U) {
                if (index + 1 >= source.size() || static_cast<unsigned char>(source[index + 1]) > 0x8fU) length = 0;
            }
        }
        if (length == 0 || index + length > source.size() ||
            (length >= 2 && !continuation(index + 1)) ||
            (length >= 3 && !continuation(index + 2)) ||
            (length == 4 && !continuation(index + 3))) {
            throw flow::DiagnosticError{"source", "invalid UTF-8 at byte " + std::to_string(index)};
        }
        index += length;
    }
}

} // namespace

std::vector<Token> lexSource(const std::string& source) {
    validateUtf8(source);
    std::vector<Token> tokens;

    std::size_t i = 0;
    int line = 1;
    int column = 1;

    auto push = [&](TokenKind kind, std::string text, int tokenLine, int tokenColumn) {
        tokens.push_back(Token{kind, std::move(text), tokenLine, tokenColumn});
    };

    while (i < source.size()) {
        const auto c = static_cast<unsigned char>(source[i]);

        if (c == ' ' || c == '\t' || c == '\r') {
            ++i;
            ++column;
            continue;
        }

        if (c == '\n') {
            push(TokenKind::Newline, "\\n", line, column);
            ++i;
            ++line;
            column = 1;
            continue;
        }

        if (c == '#') {
            while (i < source.size() && source[i] != '\n') {
                ++i;
                ++column;
            }
            continue;
        }

        if (c == '/' && i + 1 < source.size() && source[i + 1] == '/') {
            while (i < source.size() && source[i] != '\n') {
                ++i;
                ++column;
            }
            continue;
        }

        if (c == '/' && i + 1 < source.size() && source[i + 1] == '*') {
            const int commentLine = line;
            const int commentColumn = column;
            std::size_t depth = 1;
            i += 2;
            column += 2;

            while (i < source.size() && depth > 0) {
                if (source[i] == '/' && i + 1 < source.size() && source[i + 1] == '*') {
                    ++depth;
                    i += 2;
                    column += 2;
                    continue;
                }

                if (source[i] == '*' && i + 1 < source.size() && source[i + 1] == '/') {
                    --depth;
                    i += 2;
                    column += 2;
                    continue;
                }

                if (source[i] == '\n') {
                    ++i;
                    ++line;
                    column = 1;
                    continue;
                }

                ++i;
                ++column;
            }

            if (depth != 0) {
                throw flow::DiagnosticError{
                    "lexer",
                    "unterminated block comment starting at line " + std::to_string(commentLine) +
                    ", column " + std::to_string(commentColumn)
                };
            }
            continue;
        }

        if (isIdentStart(c)) {
            const int tokenLine = line;
            const int tokenColumn = column;
            const std::size_t start = i;

            while (i < source.size() && isIdentBody(static_cast<unsigned char>(source[i]))) {
                ++i;
                ++column;
            }

            std::string text = source.substr(start, i - start);
            push(keywordOrIdentifier(text), text, tokenLine, tokenColumn);
            continue;
        }

        if (std::isdigit(c)) {
            const int tokenLine = line;
            const int tokenColumn = column;
            const std::size_t start = i;
            while (i < source.size() && std::isdigit(static_cast<unsigned char>(source[i]))) {
                ++i;
                ++column;
            }
            push(TokenKind::Number, source.substr(start, i - start), tokenLine, tokenColumn);
            continue;
        }

        if (c == '"') {
            const int tokenLine = line;
            const int tokenColumn = column;
            ++i;
            ++column;
            std::string text;
            bool closed = false;
            while (i < source.size()) {
                const char ch = source[i];
                if (ch == '"') {
                    ++i;
                    ++column;
                    push(TokenKind::String, text, tokenLine, tokenColumn);
                    closed = true;
                    break;
                }
                if (ch == '\\') {
                    if (i + 1 >= source.size()) {
                        throw flow::DiagnosticError{"lexer", "unterminated escape in string"};
                    }
                    const char esc = source[i + 1];
                    if (esc == 'n') { text.push_back('\n'); }
                    else if (esc == 't') { text.push_back('\t'); }
                    else if (esc == 'r') { text.push_back('\r'); }
                    else { text.push_back(esc); }
                    i += 2;
                    column += 2;
                    continue;
                }
                if (ch == '\n') {
                    throw flow::DiagnosticError{"lexer", "unterminated string literal"};
                }
                text.push_back(ch);
                ++i;
                ++column;
            }
            if (!closed) {
                throw flow::DiagnosticError{"lexer", "unterminated string literal"};
            }
            continue;
        }

        const int tokenLine = line;
        const int tokenColumn = column;

        if (c == '=' && i + 1 < source.size() && source[i + 1] == '>') {
            push(TokenKind::Arrow, "=>", tokenLine, tokenColumn);
            i += 2;
            column += 2;
            continue;
        }

        if (c == '-' && i + 1 < source.size() && source[i + 1] == '>') {
            push(TokenKind::PlaceArrow, "->", tokenLine, tokenColumn);
            i += 2;
            column += 2;
            continue;
        }

        if (c == '>' && i + 1 < source.size() && source[i + 1] == '=') {
            push(TokenKind::GreaterEqual, ">=", tokenLine, tokenColumn);
            i += 2;
            column += 2;
            continue;
        }

        if (c == '<' && i + 1 < source.size() && source[i + 1] == '=') {
            push(TokenKind::LessEqual, "<=", tokenLine, tokenColumn);
            i += 2;
            column += 2;
            continue;
        }

        if (c == '=' && i + 1 < source.size() && source[i + 1] == '=') {
            push(TokenKind::EqualEqual, "==", tokenLine, tokenColumn);
            i += 2;
            column += 2;
            continue;
        }

        if (c == '!' && i + 1 < source.size() && source[i + 1] == '=') {
            push(TokenKind::BangEqual, "!=", tokenLine, tokenColumn);
            i += 2;
            column += 2;
            continue;
        }

        switch (c) {
            case ':': push(TokenKind::Colon, ":", tokenLine, tokenColumn); break;
            case '.': push(TokenKind::Dot, ".", tokenLine, tokenColumn); break;
            case '=': push(TokenKind::Equals, "=", tokenLine, tokenColumn); break;
            case '(': push(TokenKind::LeftParen, "(", tokenLine, tokenColumn); break;
            case ')': push(TokenKind::RightParen, ")", tokenLine, tokenColumn); break;
            case '[': push(TokenKind::LeftBracket, "[", tokenLine, tokenColumn); break;
            case ']': push(TokenKind::RightBracket, "]", tokenLine, tokenColumn); break;
            case '{': push(TokenKind::LeftBrace, "{", tokenLine, tokenColumn); break;
            case '}': push(TokenKind::RightBrace, "}", tokenLine, tokenColumn); break;
            case '<': push(TokenKind::Less, "<", tokenLine, tokenColumn); break;
            case '>': push(TokenKind::Greater, ">", tokenLine, tokenColumn); break;
            case ',': push(TokenKind::Comma, ",", tokenLine, tokenColumn); break;
            case '+': push(TokenKind::Plus, "+", tokenLine, tokenColumn); break;
            case '-': push(TokenKind::Minus, "-", tokenLine, tokenColumn); break;
            case '*': push(TokenKind::Star, "*", tokenLine, tokenColumn); break;
            case '/': push(TokenKind::Slash, "/", tokenLine, tokenColumn); break;
            case '%': push(TokenKind::Percent, "%", tokenLine, tokenColumn); break;
            default:
                throw flow::DiagnosticError{
                    "lexer",
                    "unexpected character at line " + std::to_string(line) +
                    ", column " + std::to_string(column)
                };
        }
        ++i;
        ++column;

    }

    push(TokenKind::End, "", line, column);
    return tokens;
}

} // namespace flowmini

#include <cstddef>
#include "flowmini_ast_builder.h"

namespace flowmini::ast {

    SourceLocation location_from_token(const flowmini::Token& token) {
        const auto line = token.line < 0 ? 0 : token.line;
        const auto column = token.column < 0 ? 0 : token.column;

        return SourceLocation{
            static_cast<std::size_t>(line),
            static_cast<std::size_t>(column)
        };
    }

    namespace {
        bool is_end_token(const flowmini::Token& token);




        TypeRef make_named_type_ref(const flowmini::Token& token) {
            TypeRef type;
            type.kind = TypeRefKind::Named;
            type.name = token.text;
            type.raw_text = token.text;
            type.location = location_from_token(token);
            return type;
        }

        bool is_identifier_like_type_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::Identifier;
        }

        std::size_t parse_function_signature(const std::vector<flowmini::Token>& tokens,
                                             std::size_t i,
                                             FunctionDecl& fn) {
            if (i >= tokens.size() || tokens[i].kind != flowmini::TokenKind::LeftParen) {
                return i;
            }

            ++i; // consume '('

            while (i < tokens.size() &&
                   tokens[i].kind != flowmini::TokenKind::RightParen &&
                   !is_end_token(tokens[i])) {
                if (tokens[i].kind == flowmini::TokenKind::Newline ||
                    tokens[i].kind == flowmini::TokenKind::Comma) {
                    ++i;
                    continue;
                }

                if (tokens[i].kind != flowmini::TokenKind::Identifier) {
                    ++i;
                    continue;
                }

                Parameter param;
                param.name = tokens[i].text;
                param.location = location_from_token(tokens[i]);
                ++i;

                if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Colon) {
                    ++i;

                    if (i < tokens.size() && is_identifier_like_type_token(tokens[i])) {
                        param.type = make_named_type_ref(tokens[i]);
                        ++i;
                    }
                }

                fn.parameters.push_back(std::move(param));

                if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Comma) {
                    ++i;
                }
            }

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::RightParen) {
                ++i; // consume ')'
            }

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Colon) {
                ++i;

                if (i < tokens.size() && is_identifier_like_type_token(tokens[i])) {
                    fn.return_type = make_named_type_ref(tokens[i]);
                    ++i;
                }
            }

            return i;
        }


        bool is_identifier_text(const flowmini::Token& token, const std::string& text) {
            return token.kind == flowmini::TokenKind::Identifier && token.text == text;
        }

        bool is_import_token(const flowmini::Token& token) {
            return is_identifier_text(token, "import");
        }

        bool is_type_token(const flowmini::Token& token) {
            return is_identifier_text(token, "type");
        }

        bool is_field_token(const flowmini::Token& token) {
            return is_identifier_text(token, "field");
        }

        bool is_refines_token(const flowmini::Token& token) {
            return is_identifier_text(token, "refines");
        }

        std::size_t skip_until_line_end(const std::vector<flowmini::Token>& tokens,
                                        std::size_t i) {
            while (i < tokens.size() &&
                   !is_end_token(tokens[i]) &&
                   tokens[i].kind != flowmini::TokenKind::Newline) {
                ++i;
            }
            return i;
        }

        std::size_t skip_balanced_brace_group(const std::vector<flowmini::Token>& tokens,
                                              std::size_t i) {
            if (i >= tokens.size() || tokens[i].kind != flowmini::TokenKind::LeftBrace) {
                return i;
            }

            std::size_t depth = 0;
            while (i < tokens.size() && !is_end_token(tokens[i])) {
                if (tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                    ++depth;
                } else if (tokens[i].kind == flowmini::TokenKind::RightBrace) {
                    if (depth == 0) {
                        return i;
                    }

                    --depth;
                    if (depth == 0) {
                        return i + 1;
                    }
                }

                ++i;
            }

            return i;
        }

        std::size_t parse_import_declaration(const std::vector<flowmini::Token>& tokens,
                                             std::size_t i,
                                             AstModule& module) {
            ImportDecl importDecl;
            importDecl.location = location_from_token(tokens[i]);
            ++i; // consume import

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::String) {
                importDecl.module_name = tokens[i].text;
                ++i;
            }

            module.source_unit.declarations.emplace_back(std::move(importDecl));
            return skip_until_line_end(tokens, i);
        }

        std::size_t parse_record_fields(const std::vector<flowmini::Token>& tokens,
                                        std::size_t i,
                                        RecordDecl& recordDecl) {
            if (i >= tokens.size() || tokens[i].kind != flowmini::TokenKind::LeftBrace) {
                return i;
            }

            ++i; // consume '{'

            while (i < tokens.size() &&
                   tokens[i].kind != flowmini::TokenKind::RightBrace &&
                   !is_end_token(tokens[i])) {
                if (tokens[i].kind == flowmini::TokenKind::Newline ||
                    tokens[i].kind == flowmini::TokenKind::Comma) {
                    ++i;
                    continue;
                }

                if (!is_field_token(tokens[i])) {
                    ++i;
                    continue;
                }

                RecordField field;
                field.location = location_from_token(tokens[i]);
                ++i; // consume field

                if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Identifier) {
                    field.name = tokens[i].text;
                    ++i;
                }

                if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Colon) {
                    ++i;

                    if (i < tokens.size() && is_identifier_like_type_token(tokens[i])) {
                        field.type = make_named_type_ref(tokens[i]);
                        ++i;
                    }
                }

                recordDecl.fields.push_back(std::move(field));
            }

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::RightBrace) {
                ++i;
            }

            return i;
        }

        std::size_t parse_type_declaration(const std::vector<flowmini::Token>& tokens,
                                           std::size_t i,
                                           AstModule& module) {
            const SourceLocation typeLocation = location_from_token(tokens[i]);
            ++i; // consume type

            if (i >= tokens.size() || tokens[i].kind != flowmini::TokenKind::Identifier) {
                return skip_until_line_end(tokens, i);
            }

            const std::string typeName = tokens[i].text;
            const SourceLocation nameLocation = location_from_token(tokens[i]);
            ++i;

            if (i < tokens.size() && is_refines_token(tokens[i])) {
                TypeAliasDecl aliasDecl;
                aliasDecl.name = typeName;
                aliasDecl.location = typeLocation;
                ++i; // consume refines

                if (i < tokens.size() && is_identifier_like_type_token(tokens[i])) {
                    aliasDecl.target = make_named_type_ref(tokens[i]);
                    ++i;
                }

                module.source_unit.declarations.emplace_back(std::move(aliasDecl));

                if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                    return skip_balanced_brace_group(tokens, i);
                }

                return skip_until_line_end(tokens, i);
            }

            RecordDecl recordDecl;
            recordDecl.name = typeName;
            recordDecl.location = nameLocation;

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                i = parse_record_fields(tokens, i, recordDecl);
            }

            module.source_unit.declarations.emplace_back(std::move(recordDecl));
            return i;
        }

        bool is_main_token(const flowmini::Token& token) {
            return token.kind == TokenKind::KeywordMain ||
                   (token.kind == flowmini::TokenKind::Identifier && token.text == "main");
        }

        bool is_newline_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::Newline;
        }



        bool is_end_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::End;
        }

        bool is_top_level_boundry(const flowmini::Token& token) {
            return  token.kind == flowmini::TokenKind::KeywordFn ||
                    is_main_token(token) ||
                    token.kind == flowmini::TokenKind::KeywordMain ||
                    token.kind == flowmini::TokenKind::KeywordUnit ||
                    token.kind == flowmini::TokenKind::KeywordProgram ||
                    token.kind == flowmini::TokenKind::KeywordModule;
        }

        std::size_t skip_group(const std::vector<flowmini::Token>& tokens,
                        std::size_t i,
                        flowmini::TokenKind openKind,
                        flowmini::TokenKind closeKind
                        ) {
            if (i >= tokens.size() || tokens[i].kind != openKind) {
                return i;
            }

            std::size_t depth = 0;

            while (i < tokens.size() && !is_end_token(tokens[i])) {
                if (tokens[i].kind == openKind) { ++depth; }
                else if (tokens[i].kind == closeKind) {
                    if (depth == 0) {
                        return i;
                    }

                    --depth;

                    if (depth == 0) {
                        return i + 1;
                    }
                }

                ++i;
            }

            return i;
        }


        std::size_t skip_nonsemantic_separators(const std::vector<flowmini::Token>& tokens,
                                                std::size_t i) {
            while (i < tokens.size() &&
                   (tokens[i].kind == flowmini::TokenKind::Newline ||
                    tokens[i].kind == flowmini::TokenKind::Comma)) {
                ++i;
            }

            return i;
        }


        bool is_return_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::Identifier && token.text == "return";
        }

        bool is_break_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::KeywordBreak ||
                   is_identifier_text(token, "break");
        }

        bool is_continue_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::KeywordContinue ||
                   is_identifier_text(token, "continue");
        }

        Statement make_statement_shell(StatementKind kind, const flowmini::Token& token) {
            Statement statement;
            statement.kind = kind;
            statement.location = location_from_token(token);
            return statement;
        }





        bool token_text_contains(const flowmini::Token& token, const char needle) {
            return token.text.find(needle) != std::string::npos;
        }

        ExpressionKind classify_expression_token(const flowmini::Token& token) {
            switch (token.kind) {
                case flowmini::TokenKind::Identifier:
                    return ExpressionKind::Identifier;

                case flowmini::TokenKind::Number:
                    return token_text_contains(token, '.')
                        ? ExpressionKind::FloatLiteral
                        : ExpressionKind::IntegerLiteral;

                case flowmini::TokenKind::String:
                    return ExpressionKind::StringLiteral;

                case flowmini::TokenKind::KeywordTrue:
                case flowmini::TokenKind::KeywordFalse:
                    return ExpressionKind::BoolLiteral;

                default:
                    break;
            }

            if (token.text == "true" || token.text == "false") {
                return ExpressionKind::BoolLiteral;
            }

            return ExpressionKind::Unknown;
        }

        std::size_t add_expression_placeholder(std::vector<Expression>& expressionPool,
                                               Statement& statement,
                                               const flowmini::Token& token) {
            Expression expression;
            expression.kind = classify_expression_token(token);
            expression.location = location_from_token(token);
            expression.text = token.text;

            expressionPool.push_back(std::move(expression));
            const auto expressionId = expressionPool.size() - 1;
            statement.expressions.push_back(expressionId);
            return expressionId;
        }



        bool is_binary_operator_token(const flowmini::Token& token) {
            switch (token.kind) {
                case flowmini::TokenKind::Plus:
                case flowmini::TokenKind::Minus:
                case flowmini::TokenKind::Star:
                case flowmini::TokenKind::Slash:
                case flowmini::TokenKind::Percent:
                case flowmini::TokenKind::Less:
                case flowmini::TokenKind::Greater:
                case flowmini::TokenKind::LessEqual:
                case flowmini::TokenKind::GreaterEqual:
                case flowmini::TokenKind::EqualEqual:
                case flowmini::TokenKind::BangEqual:
                    return true;

                default:
                    break;
            }

            return false;
        }

        bool is_expression_boundary_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::Newline ||
                   token.kind == flowmini::TokenKind::RightBrace ||
                   token.kind == flowmini::TokenKind::End;
        }

        std::size_t find_shallow_binary_operator(const std::vector<flowmini::Token>& tokens,
                                                 std::size_t i) {
            const auto expressionStart = i;

            std::size_t parenDepth = 0;
            std::size_t bracketDepth = 0;

            while (i < tokens.size() && !is_expression_boundary_token(tokens[i])) {
                const auto& token = tokens[i];

                if (token.kind == flowmini::TokenKind::LeftBrace && parenDepth == 0 && bracketDepth == 0) {
                    return tokens.size();
                }

                if (token.kind == flowmini::TokenKind::LeftParen) {
                    ++parenDepth;
                    ++i;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightParen) {
                    if (parenDepth == 0) {
                        return tokens.size();
                    }

                    --parenDepth;
                    ++i;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBracket) {
                    ++bracketDepth;
                    ++i;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBracket) {
                    if (bracketDepth == 0) {
                        return tokens.size();
                    }

                    --bracketDepth;
                    ++i;
                    continue;
                }

                if (i == expressionStart && token.kind == flowmini::TokenKind::Minus) {
                    ++i;
                    continue;
                }

                if (parenDepth == 0 && bracketDepth == 0 && is_binary_operator_token(token)) {
                    return i;
                }

                ++i;
            }

            return tokens.size();
        }

        bool expression_starts_unary(const std::vector<flowmini::Token>& tokens,
                                     const std::size_t i) {
            return i < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::Minus;
        }



        bool expression_starts_list_literal(const std::vector<flowmini::Token>& tokens,
                                            const std::size_t i) {
            return i < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::LeftBracket;
        }

        bool expression_starts_record_literal(const std::vector<flowmini::Token>& tokens,
                                              const std::size_t i) {
            return i < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::LeftBrace;
        }

        bool expression_starts_index_access(const std::vector<flowmini::Token>& tokens,
                                            const std::size_t i) {
            return i + 1 < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::Identifier &&
                   tokens[i + 1].kind == flowmini::TokenKind::LeftBracket;
        }

        bool expression_starts_field_access(const std::vector<flowmini::Token>& tokens,
                                            const std::size_t i) {
            return i + 2 < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::Identifier &&
                   tokens[i + 1].kind == flowmini::TokenKind::Dot &&
                   tokens[i + 2].kind == flowmini::TokenKind::Identifier;
        }

        std::string field_access_text(const std::vector<flowmini::Token>& tokens,
                                      const std::size_t i) {
            if (expression_starts_field_access(tokens, i)) {
                return tokens[i].text + "." + tokens[i + 2].text;
            }

            if (i < tokens.size()) {
                return tokens[i].text;
            }

            return {};
        }

        bool expression_starts_call(const std::vector<flowmini::Token>& tokens,
                                    const std::size_t i) {
            return i + 1 < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::Identifier &&
                   tokens[i + 1].kind == flowmini::TokenKind::LeftParen;
        }

    std::size_t find_matching_right_paren(const std::vector<flowmini::Token>& tokens,
                                          const std::size_t leftParenIndex) {
        if (leftParenIndex >= tokens.size() ||
            tokens[leftParenIndex].kind != flowmini::TokenKind::LeftParen) {
            return tokens.size();
        }

        std::size_t depth = 0;

        for (std::size_t i = leftParenIndex; i < tokens.size(); ++i) {
            if (tokens[i].kind == flowmini::TokenKind::LeftParen) {
                ++depth;
                continue;
            }

            if (tokens[i].kind == flowmini::TokenKind::RightParen) {
                if (depth == 0) {
                    return tokens.size();
                }

                --depth;

                if (depth == 0) {
                    return i;
                }
            }

            if (is_end_token(tokens[i]) || tokens[i].kind == flowmini::TokenKind::Newline) {
                return tokens.size();
            }
        }

        return tokens.size();
    }

    bool is_expression_separator_token(const flowmini::Token& token) {
        return token.kind == flowmini::TokenKind::Comma ||
               token.kind == flowmini::TokenKind::Newline;
    }

    std::size_t first_argument_token(const std::vector<flowmini::Token>& tokens,
                                     std::size_t begin,
                                     const std::size_t end) {
        while (begin < end && is_expression_separator_token(tokens[begin])) {
            ++begin;
        }

        return begin;
    }

    Expression make_shallow_expression_from_tokens(const std::vector<flowmini::Token>& tokens,
                                                   const std::size_t i) {
        Expression expression;

        if (tokens.empty() || i >= tokens.size()) {
            return expression;
        }

        const auto& token = tokens[i];
        expression.location = location_from_token(token);

        const auto binaryOperatorIndex = find_shallow_binary_operator(tokens, i);
        if (binaryOperatorIndex < tokens.size()) {
            expression.kind = ExpressionKind::Binary;
            expression.text = tokens[binaryOperatorIndex].text;
        } else if (expression_starts_unary(tokens, i)) {
            expression.kind = ExpressionKind::Unary;
            expression.text = token.text;
        } else if (expression_starts_call(tokens, i)) {
            expression.kind = ExpressionKind::Call;
            expression.text = token.text;
        } else if (expression_starts_index_access(tokens, i)) {
            expression.kind = ExpressionKind::Index;
            expression.text = token.text;
        } else if (expression_starts_field_access(tokens, i)) {
            expression.kind = ExpressionKind::FieldAccess;
            expression.text = field_access_text(tokens, i);
        } else if (expression_starts_list_literal(tokens, i)) {
            expression.kind = ExpressionKind::ListLiteral;
            expression.text = token.text;
        } else if (expression_starts_record_literal(tokens, i)) {
            expression.kind = ExpressionKind::RecordLiteral;
            expression.text = token.text;
        } else {
            expression.kind = classify_expression_token(token);
            expression.text = token.text;
        }

        return expression;
    }

    void append_call_argument_child(std::vector<Expression>& expressionPool,
                                    const std::size_t callExpressionId,
                                    const std::vector<flowmini::Token>& tokens,
                                    const std::size_t begin,
                                    const std::size_t end) {
        const auto argStart = first_argument_token(tokens, begin, end);
        if (argStart >= end) {
            return;
        }

        std::vector<flowmini::Token> argumentTokens;
        argumentTokens.reserve(end - argStart);

        for (std::size_t i = argStart; i < end; ++i) {
            argumentTokens.push_back(tokens[i]);
        }

        Expression argument = make_shallow_expression_from_tokens(argumentTokens, 0);

        expressionPool.push_back(std::move(argument));
        const auto argumentId = expressionPool.size() - 1;

        expressionPool[callExpressionId].child_expressions.push_back(argumentId);
    }

    void populate_call_argument_children(std::vector<Expression>& expressionPool,
                                         const std::size_t callExpressionId,
                                         const std::vector<flowmini::Token>& tokens,
                                         const std::size_t callIndex) {
        if (callIndex + 1 >= tokens.size() ||
            tokens[callIndex + 1].kind != flowmini::TokenKind::LeftParen) {
            return;
        }

        const auto closeParenIndex = find_matching_right_paren(tokens, callIndex + 1);
        if (closeParenIndex == tokens.size() || closeParenIndex <= callIndex + 2) {
            return;
        }

        std::size_t argBegin = callIndex + 2;
        std::size_t parenDepth = 0;
        std::size_t bracketDepth = 0;
        std::size_t braceDepth = 0;

        for (std::size_t i = argBegin; i <= closeParenIndex; ++i) {
            const bool atEnd = i == closeParenIndex;

            if (!atEnd) {
                const auto& token = tokens[i];

                if (token.kind == flowmini::TokenKind::LeftParen) {
                    ++parenDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightParen) {
                    if (parenDepth > 0) {
                        --parenDepth;
                    }
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBracket) {
                    ++bracketDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBracket) {
                    if (bracketDepth > 0) {
                        --bracketDepth;
                    }
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBrace) {
                    ++braceDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBrace) {
                    if (braceDepth > 0) {
                        --braceDepth;
                    }
                    continue;
                }

                const bool atTopLevelComma =
                    token.kind == flowmini::TokenKind::Comma &&
                    parenDepth == 0 &&
                    bracketDepth == 0 &&
                    braceDepth == 0;

                if (!atTopLevelComma) {
                    continue;
                }
            }

            append_call_argument_child(expressionPool,
                                       callExpressionId,
                                       tokens,
                                       argBegin,
                                       i);
            argBegin = i + 1;
        }
    }


    
    void append_binary_operand_child(std::vector<Expression>& expressionPool,
                                     const std::size_t binaryExpressionId,
                                     const std::vector<flowmini::Token>& tokens,
                                     const std::size_t begin,
                                     const std::size_t end) {
        std::size_t operandStart = begin;

        while (operandStart < end &&
               (tokens[operandStart].kind == flowmini::TokenKind::Comma ||
                tokens[operandStart].kind == flowmini::TokenKind::Newline)) {
            ++operandStart;
        }

        if (operandStart >= end) {
            return;
        }

        std::vector<flowmini::Token> operandTokens;
        operandTokens.reserve(end - operandStart);

        for (std::size_t i = operandStart; i < end; ++i) {
            operandTokens.push_back(tokens[i]);
        }

        Expression operand = make_shallow_expression_from_tokens(operandTokens, 0);

        expressionPool.push_back(std::move(operand));
        const auto operandId = expressionPool.size() - 1;

        expressionPool[binaryExpressionId].child_expressions.push_back(operandId);
    }

    void populate_binary_operand_children(std::vector<Expression>& expressionPool,
                                          const std::size_t binaryExpressionId,
                                          const std::vector<flowmini::Token>& tokens,
                                          const std::size_t expressionStart) {
        const auto binaryOperatorIndex = find_shallow_binary_operator(tokens, expressionStart);
        if (binaryOperatorIndex == tokens.size()) {
            return;
        }

        if (binaryOperatorIndex <= expressionStart) {
            return;
        }

        append_binary_operand_child(expressionPool,
                                    binaryExpressionId,
                                    tokens,
                                    expressionStart,
                                    binaryOperatorIndex);

        append_binary_operand_child(expressionPool,
                                    binaryExpressionId,
                                    tokens,
                                    binaryOperatorIndex + 1,
                                    tokens.size());
    }


    void populate_unary_operand_child(std::vector<Expression>& expressionPool,
                                      const std::size_t unaryExpressionId,
                                      const std::vector<flowmini::Token>& tokens,
                                      const std::size_t unaryIndex) {
        if (unaryIndex + 1 >= tokens.size()) {
            return;
        }

        const auto operandIndex = unaryIndex + 1;

        if (is_expression_boundary_token(tokens[operandIndex]) ||
            tokens[operandIndex].kind == flowmini::TokenKind::Comma ||
            tokens[operandIndex].kind == flowmini::TokenKind::RightParen ||
            tokens[operandIndex].kind == flowmini::TokenKind::RightBracket ||
            tokens[operandIndex].kind == flowmini::TokenKind::RightBrace) {
            return;
        }

        std::vector<flowmini::Token> operandTokens;
        operandTokens.reserve(tokens.size() - operandIndex);

        for (std::size_t i = operandIndex; i < tokens.size(); ++i) {
            if (is_expression_boundary_token(tokens[i]) ||
                tokens[i].kind == flowmini::TokenKind::Comma ||
                tokens[i].kind == flowmini::TokenKind::RightParen ||
                tokens[i].kind == flowmini::TokenKind::RightBracket ||
                tokens[i].kind == flowmini::TokenKind::RightBrace) {
                break;
            }

            operandTokens.push_back(tokens[i]);
        }

        if (operandTokens.empty()) {
            return;
        }

        Expression operand = make_shallow_expression_from_tokens(operandTokens, 0);

        expressionPool.push_back(std::move(operand));
        const auto operandId = expressionPool.size() - 1;

        expressionPool[unaryExpressionId].child_expressions.push_back(operandId);
    }


    std::size_t find_matching_right_bracket(const std::vector<flowmini::Token>& tokens,
                                            const std::size_t leftBracketIndex) {
        if (leftBracketIndex >= tokens.size() ||
            tokens[leftBracketIndex].kind != flowmini::TokenKind::LeftBracket) {
            return tokens.size();
        }

        std::size_t depth = 0;

        for (std::size_t i = leftBracketIndex; i < tokens.size(); ++i) {
            if (tokens[i].kind == flowmini::TokenKind::LeftBracket) {
                ++depth;
                continue;
            }

            if (tokens[i].kind == flowmini::TokenKind::RightBracket) {
                if (depth == 0) {
                    return tokens.size();
                }

                --depth;

                if (depth == 0) {
                    return i;
                }
            }

            if (is_end_token(tokens[i]) || tokens[i].kind == flowmini::TokenKind::Newline) {
                return tokens.size();
            }
        }

        return tokens.size();
    }

    void append_index_child(std::vector<Expression>& expressionPool,
                            const std::size_t indexExpressionId,
                            const std::vector<flowmini::Token>& tokens,
                            const std::size_t begin,
                            const std::size_t end) {
        std::size_t childStart = begin;

        while (childStart < end &&
               (tokens[childStart].kind == flowmini::TokenKind::Comma ||
                tokens[childStart].kind == flowmini::TokenKind::Newline)) {
            ++childStart;
        }

        if (childStart >= end) {
            return;
        }

        std::vector<flowmini::Token> childTokens;
        childTokens.reserve(end - childStart);

        for (std::size_t i = childStart; i < end; ++i) {
            childTokens.push_back(tokens[i]);
        }

        Expression child = make_shallow_expression_from_tokens(childTokens, 0);

        expressionPool.push_back(std::move(child));
        const auto childId = expressionPool.size() - 1;

        expressionPool[indexExpressionId].child_expressions.push_back(childId);
    }

    void populate_index_expression_children(std::vector<Expression>& expressionPool,
                                            const std::size_t indexExpressionId,
                                            const std::vector<flowmini::Token>& tokens,
                                            const std::size_t expressionStart) {
        if (!expression_starts_index_access(tokens, expressionStart)) {
            return;
        }

        const auto leftBracketIndex = expressionStart + 1;
        const auto rightBracketIndex = find_matching_right_bracket(tokens, leftBracketIndex);

        if (rightBracketIndex == tokens.size() || rightBracketIndex <= leftBracketIndex + 1) {
            return;
        }

        append_index_child(expressionPool,
                           indexExpressionId,
                           tokens,
                           expressionStart,
                           leftBracketIndex);

        append_index_child(expressionPool,
                           indexExpressionId,
                           tokens,
                           leftBracketIndex + 1,
                           rightBracketIndex);
    }


    void populate_field_access_children(std::vector<Expression>& expressionPool,
                                        const std::size_t fieldExpressionId,
                                        const std::vector<flowmini::Token>& tokens,
                                        const std::size_t expressionStart) {
        if (!expression_starts_field_access(tokens, expressionStart)) {
            return;
        }

        Expression base;
        base.kind = ExpressionKind::Identifier;
        base.text = tokens[expressionStart].text;
        base.location = location_from_token(tokens[expressionStart]);

        expressionPool.push_back(std::move(base));
        const auto baseId = expressionPool.size() - 1;

        expressionPool[fieldExpressionId].child_expressions.push_back(baseId);
    }


    void append_list_literal_element_child(std::vector<Expression>& expressionPool,
                                           const std::size_t listExpressionId,
                                           const std::vector<flowmini::Token>& tokens,
                                           const std::size_t begin,
                                           const std::size_t end) {
        std::size_t elementStart = begin;

        while (elementStart < end &&
               (tokens[elementStart].kind == flowmini::TokenKind::Comma ||
                tokens[elementStart].kind == flowmini::TokenKind::Newline)) {
            ++elementStart;
        }

        if (elementStart >= end) {
            return;
        }

        std::vector<flowmini::Token> elementTokens;
        elementTokens.reserve(end - elementStart);

        for (std::size_t i = elementStart; i < end; ++i) {
            elementTokens.push_back(tokens[i]);
        }

        Expression element = make_shallow_expression_from_tokens(elementTokens, 0);

        expressionPool.push_back(std::move(element));
        const auto elementId = expressionPool.size() - 1;

        expressionPool[listExpressionId].child_expressions.push_back(elementId);
    }

    void populate_list_literal_children(std::vector<Expression>& expressionPool,
                                        const std::size_t listExpressionId,
                                        const std::vector<flowmini::Token>& tokens,
                                        const std::size_t expressionStart) {
        if (!expression_starts_list_literal(tokens, expressionStart)) {
            return;
        }

        const auto leftBracketIndex = expressionStart;
        const auto rightBracketIndex = find_matching_right_bracket(tokens, leftBracketIndex);

        if (rightBracketIndex == tokens.size() || rightBracketIndex <= leftBracketIndex + 1) {
            return;
        }

        std::size_t elementBegin = leftBracketIndex + 1;
        std::size_t parenDepth = 0;
        std::size_t bracketDepth = 0;
        std::size_t braceDepth = 0;

        for (std::size_t i = elementBegin; i <= rightBracketIndex; ++i) {
            const bool atEnd = i == rightBracketIndex;

            if (!atEnd) {
                const auto& token = tokens[i];

                if (token.kind == flowmini::TokenKind::LeftParen) {
                    ++parenDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightParen) {
                    if (parenDepth > 0) {
                        --parenDepth;
                    }
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBracket) {
                    ++bracketDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBracket) {
                    if (bracketDepth > 0) {
                        --bracketDepth;
                    }
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBrace) {
                    ++braceDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBrace) {
                    if (braceDepth > 0) {
                        --braceDepth;
                    }
                    continue;
                }

                const bool atTopLevelComma =
                    token.kind == flowmini::TokenKind::Comma &&
                    parenDepth == 0 &&
                    bracketDepth == 0 &&
                    braceDepth == 0;

                if (!atTopLevelComma) {
                    continue;
                }
            }

            append_list_literal_element_child(expressionPool,
                                              listExpressionId,
                                              tokens,
                                              elementBegin,
                                              i);
            elementBegin = i + 1;
        }
    }


    std::size_t find_matching_right_brace(const std::vector<flowmini::Token>& tokens,
                                          const std::size_t leftBraceIndex) {
        if (leftBraceIndex >= tokens.size() ||
            tokens[leftBraceIndex].kind != flowmini::TokenKind::LeftBrace) {
            return tokens.size();
        }

        std::size_t depth = 0;

        for (std::size_t i = leftBraceIndex; i < tokens.size(); ++i) {
            if (tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                ++depth;
                continue;
            }

            if (tokens[i].kind == flowmini::TokenKind::RightBrace) {
                if (depth == 0) {
                    return tokens.size();
                }

                --depth;

                if (depth == 0) {
                    return i;
                }
            }

            if (is_end_token(tokens[i]) || tokens[i].kind == flowmini::TokenKind::Newline) {
                return tokens.size();
            }
        }

        return tokens.size();
    }

    std::size_t find_top_level_colon(const std::vector<flowmini::Token>& tokens,
                                     const std::size_t begin,
                                     const std::size_t end) {
        std::size_t parenDepth = 0;
        std::size_t bracketDepth = 0;
        std::size_t braceDepth = 0;

        for (std::size_t i = begin; i < end; ++i) {
            const auto& token = tokens[i];

            if (token.kind == flowmini::TokenKind::LeftParen) {
                ++parenDepth;
                continue;
            }

            if (token.kind == flowmini::TokenKind::RightParen) {
                if (parenDepth > 0) {
                    --parenDepth;
                }
                continue;
            }

            if (token.kind == flowmini::TokenKind::LeftBracket) {
                ++bracketDepth;
                continue;
            }

            if (token.kind == flowmini::TokenKind::RightBracket) {
                if (bracketDepth > 0) {
                    --bracketDepth;
                }
                continue;
            }

            if (token.kind == flowmini::TokenKind::LeftBrace) {
                ++braceDepth;
                continue;
            }

            if (token.kind == flowmini::TokenKind::RightBrace) {
                if (braceDepth > 0) {
                    --braceDepth;
                }
                continue;
            }

            if (token.kind == flowmini::TokenKind::Colon &&
                parenDepth == 0 &&
                bracketDepth == 0 &&
                braceDepth == 0) {
                return i;
            }
        }

        return tokens.size();
    }

    void append_record_literal_value_child(std::vector<Expression>& expressionPool,
                                           const std::size_t recordExpressionId,
                                           const std::vector<flowmini::Token>& tokens,
                                           const std::size_t begin,
                                           const std::size_t end) {
        std::size_t fieldStart = begin;

        while (fieldStart < end &&
               (tokens[fieldStart].kind == flowmini::TokenKind::Comma ||
                tokens[fieldStart].kind == flowmini::TokenKind::Newline)) {
            ++fieldStart;
        }

        if (fieldStart >= end) {
            return;
        }

        const auto colonIndex = find_top_level_colon(tokens, fieldStart, end);
        if (colonIndex == tokens.size() || colonIndex + 1 >= end) {
            return;
        }

        std::size_t valueStart = colonIndex + 1;
        while (valueStart < end &&
               (tokens[valueStart].kind == flowmini::TokenKind::Comma ||
                tokens[valueStart].kind == flowmini::TokenKind::Newline)) {
            ++valueStart;
        }

        if (valueStart >= end) {
            return;
        }

        std::vector<flowmini::Token> valueTokens;
        valueTokens.reserve(end - valueStart);

        for (std::size_t i = valueStart; i < end; ++i) {
            valueTokens.push_back(tokens[i]);
        }

        Expression value = make_shallow_expression_from_tokens(valueTokens, 0);

        expressionPool.push_back(std::move(value));
        const auto valueId = expressionPool.size() - 1;

        expressionPool[recordExpressionId].child_expressions.push_back(valueId);
    }

    void populate_record_literal_children(std::vector<Expression>& expressionPool,
                                          const std::size_t recordExpressionId,
                                          const std::vector<flowmini::Token>& tokens,
                                          const std::size_t expressionStart) {
        if (!expression_starts_record_literal(tokens, expressionStart)) {
            return;
        }

        const auto leftBraceIndex = expressionStart;
        const auto rightBraceIndex = find_matching_right_brace(tokens, leftBraceIndex);

        if (rightBraceIndex == tokens.size() || rightBraceIndex <= leftBraceIndex + 1) {
            return;
        }

        std::size_t fieldBegin = leftBraceIndex + 1;
        std::size_t parenDepth = 0;
        std::size_t bracketDepth = 0;
        std::size_t braceDepth = 0;

        for (std::size_t i = fieldBegin; i <= rightBraceIndex; ++i) {
            const bool atEnd = i == rightBraceIndex;

            if (!atEnd) {
                const auto& token = tokens[i];

                if (token.kind == flowmini::TokenKind::LeftParen) {
                    ++parenDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightParen) {
                    if (parenDepth > 0) {
                        --parenDepth;
                    }
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBracket) {
                    ++bracketDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBracket) {
                    if (bracketDepth > 0) {
                        --bracketDepth;
                    }
                    continue;
                }

                if (token.kind == flowmini::TokenKind::LeftBrace) {
                    ++braceDepth;
                    continue;
                }

                if (token.kind == flowmini::TokenKind::RightBrace) {
                    if (braceDepth > 0) {
                        --braceDepth;
                    }
                    continue;
                }

                const bool atTopLevelComma =
                    token.kind == flowmini::TokenKind::Comma &&
                    parenDepth == 0 &&
                    bracketDepth == 0 &&
                    braceDepth == 0;

                if (!atTopLevelComma) {
                    continue;
                }
            }

            append_record_literal_value_child(expressionPool,
                                              recordExpressionId,
                                              tokens,
                                              fieldBegin,
                                              i);
            fieldBegin = i + 1;
        }
    }

    std::size_t add_expression_placeholder_at(std::vector<Expression>& expressionPool,
                                                  Statement& statement,
                                                  const std::vector<flowmini::Token>& tokens,
                                                  const std::size_t i) {
            if (tokens.empty()) {
                return 0;
            }

            if (i >= tokens.size()) {
                return add_expression_placeholder(expressionPool, statement, tokens.back());
            }

            Expression expression = make_shallow_expression_from_tokens(tokens, i);

            expressionPool.push_back(std::move(expression));
            const auto expressionId = expressionPool.size() - 1;
            statement.expressions.push_back(expressionId);

        if (expressionPool[expressionId].kind == ExpressionKind::RecordLiteral) {
            populate_record_literal_children(expressionPool, expressionId, tokens, i);
        }

        if (expressionPool[expressionId].kind == ExpressionKind::ListLiteral) {
            populate_list_literal_children(expressionPool, expressionId, tokens, i);
        }

        if (expressionPool[expressionId].kind == ExpressionKind::FieldAccess) {
            populate_field_access_children(expressionPool, expressionId, tokens, i);
        }

        if (expressionPool[expressionId].kind == ExpressionKind::Index) {
            populate_index_expression_children(expressionPool, expressionId, tokens, i);
        }

        if (expressionPool[expressionId].kind == ExpressionKind::Binary) {
            populate_binary_operand_children(expressionPool, expressionId, tokens, i);
        }

        if (expressionPool[expressionId].kind == ExpressionKind::Unary) {
            populate_unary_operand_child(expressionPool, expressionId, tokens, i);
        }

        if (expressionPool[expressionId].kind == ExpressionKind::Call) {
            populate_call_argument_children(expressionPool, expressionId, tokens, i);
        }

        return expressionId;
        }



        bool has_expression_until_body_or_line_end(const std::vector<flowmini::Token>& tokens,
                                                   std::size_t i) {
            while (i < tokens.size() &&
                   !is_end_token(tokens[i]) &&
                   tokens[i].kind != flowmini::TokenKind::LeftBrace &&
                   tokens[i].kind != flowmini::TokenKind::RightBrace &&
                   tokens[i].kind != flowmini::TokenKind::Newline) {
                if (tokens[i].kind != flowmini::TokenKind::Comma) {
                    return true;
                }

                ++i;
            }

            return false;
        }

        bool has_expression_until_statement_boundary(const std::vector<flowmini::Token>& tokens,
                                                     std::size_t i) {
            while (i < tokens.size() &&
                   !is_end_token(tokens[i]) &&
                   tokens[i].kind != flowmini::TokenKind::Newline &&
                   tokens[i].kind != flowmini::TokenKind::RightBrace) {
                if (tokens[i].kind != flowmini::TokenKind::Comma) {
                    return true;
                }

                ++i;
            }

            return false;
        }

        bool is_typed_binding_start(const std::vector<flowmini::Token>& tokens,
                                    const std::size_t i) {
            return i + 2 < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::Identifier &&
                   tokens[i + 1].kind == flowmini::TokenKind::Colon &&
                   is_identifier_like_type_token(tokens[i + 2]);
        }

        std::size_t parse_typed_binding_statement_shell(const std::vector<flowmini::Token>& tokens,
                                                        std::size_t i,
                                                        std::vector<Statement>& body,
                                                        std::vector<Expression>& expressionPool) {
            Statement statement;
            statement.kind = StatementKind::Let;
            statement.location = location_from_token(tokens[i]);
            statement.name = tokens[i].text;

            i += 2; // consume name and ':'

            if (i < tokens.size() && is_identifier_like_type_token(tokens[i])) {
                statement.type = make_named_type_ref(tokens[i]);
                ++i;
            }

            statement.has_initializer =
                i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftParen;

            if (statement.has_initializer) {
                const auto expressionStart = i + 1;
                if (expressionStart < tokens.size() &&
                    tokens[expressionStart].kind != flowmini::TokenKind::RightParen) {
                    add_expression_placeholder_at(expressionPool, statement, tokens, expressionStart);
                } else {
                    add_expression_placeholder_at(expressionPool, statement, tokens, i);
                }
            }

            body.push_back(std::move(statement));
            return i;
        }


        bool is_plain_assignment_start(const std::vector<flowmini::Token>& tokens,
                                       const std::size_t i) {
            return i + 1 < tokens.size() &&
                   tokens[i].kind == flowmini::TokenKind::Identifier &&
                   tokens[i + 1].kind == flowmini::TokenKind::Equals;
        }

        std::size_t parse_plain_assignment_statement_shell(const std::vector<flowmini::Token>& tokens,
                                                           std::size_t i,
                                                           std::vector<Statement>& body,
                                                           std::vector<Expression>& expressionPool) {
            Statement statement;
            statement.kind = StatementKind::Assignment;
            statement.location = location_from_token(tokens[i]);
            statement.name = tokens[i].text;

            i += 2; // consume name and '='

            statement.has_value = has_expression_until_statement_boundary(tokens, i);

            if (statement.has_value && i < tokens.size()) {
                add_expression_placeholder_at(expressionPool, statement, tokens, i);
            }

            body.push_back(std::move(statement));
            return i;
        }


        bool is_if_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::KeywordIf ||
                   is_identifier_text(token, "if");
        }

        bool is_while_token(const flowmini::Token& token) {
            return token.kind == flowmini::TokenKind::KeywordWhile ||
                   is_identifier_text(token, "while");
        }

        std::size_t skip_until_body_block_or_line_end(const std::vector<flowmini::Token>& tokens,
                                                      std::size_t i) {
            while (i < tokens.size() &&
                   !is_end_token(tokens[i]) &&
                   tokens[i].kind != flowmini::TokenKind::LeftBrace &&
                   tokens[i].kind != flowmini::TokenKind::Newline) {
                ++i;
            }

            return i;
        }

        std::size_t parse_body_statement_shells(const std::vector<flowmini::Token>& tokens,
                                                std::size_t i,
                                                std::vector<Statement>& body,
                                                std::vector<Expression>& expressionPool);

        std::size_t parse_if_statement_shell(const std::vector<flowmini::Token>& tokens,
                                             std::size_t i,
                                             std::vector<Statement>& body,
                                             std::vector<Expression>& expressionPool) {
            Statement statement;
            statement.kind = StatementKind::If;
            statement.location = location_from_token(tokens[i]);

            ++i; // consume if

            statement.has_condition = has_expression_until_body_or_line_end(tokens, i);
            if (statement.has_condition && i < tokens.size()) {
                add_expression_placeholder_at(expressionPool, statement, tokens, i);
            }
            i = skip_until_body_block_or_line_end(tokens, i);

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                statement.has_body = true;
                statement.body_location = location_from_token(tokens[i]);
                i = parse_body_statement_shells(tokens, i, statement.body, expressionPool);
            }

            body.push_back(std::move(statement));
            return i;
        }

        std::size_t parse_while_statement_shell(const std::vector<flowmini::Token>& tokens,
                                                std::size_t i,
                                                std::vector<Statement>& body,
                                                std::vector<Expression>& expressionPool) {
            Statement statement;
            statement.kind = StatementKind::While;
            statement.location = location_from_token(tokens[i]);

            ++i; // consume while

            statement.has_condition = has_expression_until_body_or_line_end(tokens, i);
            if (statement.has_condition && i < tokens.size()) {
                add_expression_placeholder_at(expressionPool, statement, tokens, i);
            }
            i = skip_until_body_block_or_line_end(tokens, i);

            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                statement.has_body = true;
                statement.body_location = location_from_token(tokens[i]);
                i = parse_body_statement_shells(tokens, i, statement.body, expressionPool);
            }

            body.push_back(std::move(statement));
            return i;
        }

        std::size_t parse_body_statement_shells(const std::vector<flowmini::Token>& tokens,
                                                std::size_t i,
                                                std::vector<Statement>& body,
                                                std::vector<Expression>& expressionPool) {
            i = skip_nonsemantic_separators(tokens, i);

            if (i >= tokens.size() || tokens[i].kind != flowmini::TokenKind::LeftBrace) {
                return i;
            }

            ++i; // consume '{'

            while (i < tokens.size() && !is_end_token(tokens[i])) {
                if (tokens[i].kind == flowmini::TokenKind::RightBrace) {
                    ++i;
                    return i;
                }

                if (tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                    i = skip_group(tokens, i, flowmini::TokenKind::LeftBrace, flowmini::TokenKind::RightBrace);
                    continue;
                }

                if (is_typed_binding_start(tokens, i)) {
                    i = parse_typed_binding_statement_shell(tokens, i, body, expressionPool);
                    continue;
                }

                if (is_if_token(tokens[i])) {
                    i = parse_if_statement_shell(tokens, i, body, expressionPool);
                    continue;
                }

                if (is_while_token(tokens[i])) {
                    i = parse_while_statement_shell(tokens, i, body, expressionPool);
                    continue;
                }

                if (is_plain_assignment_start(tokens, i)) {
                    i = parse_plain_assignment_statement_shell(tokens, i, body, expressionPool);
                    continue;
                }

                if (is_return_token(tokens[i])) {
                    Statement statement = make_statement_shell(StatementKind::Return, tokens[i]);
                    statement.has_value = has_expression_until_statement_boundary(tokens, i + 1);
                    if (statement.has_value && i + 1 < tokens.size()) {
                        add_expression_placeholder_at(expressionPool, statement, tokens, i + 1);
                    }
                    body.push_back(std::move(statement));
                    ++i;
                    continue;
                }

                if (is_break_token(tokens[i])) {
                    body.push_back(make_statement_shell(StatementKind::Break, tokens[i]));
                    ++i;
                    continue;
                }

                if (is_continue_token(tokens[i])) {
                    body.push_back(make_statement_shell(StatementKind::Continue, tokens[i]));
                    ++i;
                    continue;
                }

                ++i;
            }

            return i;
        }

        std::size_t mark_body_container(const std::vector<flowmini::Token>& tokens,
                                        std::size_t i,
                                        bool& hasBody,
                                        SourceLocation& bodyLocation,
                                        std::vector<Statement>& body,
                                        std::vector<Expression>& expressionPool) {
            i = skip_nonsemantic_separators(tokens, i);

            if (i >= tokens.size() || tokens[i].kind != flowmini::TokenKind::LeftBrace) {
                return i;
            }

            hasBody = true;
            bodyLocation = location_from_token(tokens[i]);

            ++i; // consume '{'
            std::size_t braceDepth = 1;

            while (i < tokens.size() && !is_end_token(tokens[i])) {
                if (tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                    ++braceDepth;
                    ++i;
                    continue;
                }

                if (tokens[i].kind == flowmini::TokenKind::RightBrace) {
                    --braceDepth;
                    ++i;

                    if (braceDepth == 0) {
                        return i;
                    }

                    continue;
                }

                if (braceDepth == 1) {
                    if (is_typed_binding_start(tokens, i)) {
                        i = parse_typed_binding_statement_shell(tokens, i, body, expressionPool);
                        continue;
                    }

                    if (is_if_token(tokens[i])) {
                        i = parse_if_statement_shell(tokens, i, body, expressionPool);
                        continue;
                    }

                    if (is_while_token(tokens[i])) {
                        i = parse_while_statement_shell(tokens, i, body, expressionPool);
                        continue;
                    }

                    if (is_plain_assignment_start(tokens, i)) {
                        i = parse_plain_assignment_statement_shell(tokens, i, body, expressionPool);
                        continue;
                    }

                    if (is_return_token(tokens[i])) {
                        body.push_back(make_statement_shell(StatementKind::Return, tokens[i]));
                        ++i;
                        continue;
                    }

                    if (is_break_token(tokens[i])) {
                        body.push_back(make_statement_shell(StatementKind::Break, tokens[i]));
                        ++i;
                        continue;
                    }

                    if (is_continue_token(tokens[i])) {
                        body.push_back(make_statement_shell(StatementKind::Continue, tokens[i]));
                        ++i;
                        continue;
                    }
                }

                ++i;
            }

            return i;
        }

        std::size_t skip_until_next_top_levelish_token(const std::vector<flowmini::Token>& tokens,std::size_t i) {
            while (i < tokens.size() && !is_end_token(tokens[i])) {
                if (tokens[i].kind == flowmini::TokenKind::LeftParen) {
                    i = skip_group(tokens, i, flowmini::TokenKind::LeftParen, flowmini::TokenKind::RightParen);
                    continue;
                }

                if (tokens[i].kind == flowmini::TokenKind::LeftBracket) {
                    i = skip_group(tokens, i, flowmini::TokenKind::LeftBracket, flowmini::TokenKind::RightBracket);
                    continue;
                }

                if (tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                    i = skip_group(tokens, i, flowmini::TokenKind::LeftBrace, flowmini::TokenKind::RightBrace);
                    continue;
                }

                if (is_top_level_boundry(tokens[i])) {
                    return i;
                }

                ++i;
            }

            return i;
        }

    } // namespace

    AstModule build_source_header_ast(const std::vector<flowmini::Token>& tokens) {
        AstModule module = make_empty_ast_module();

        std::size_t i = 0;

        while (i < tokens.size() && is_newline_token(tokens[i])) {
            ++i;
        }

        if (i >= tokens.size()) {
            return module;
        }

        const flowmini::Token& kindToken = tokens[i];

        if (kindToken.kind == flowmini::TokenKind::KeywordProgram) {
            module.source_unit.kind = SourceUnitKind::Program;
        } else if (kindToken.kind == flowmini::TokenKind::KeywordUnit) {
            module.source_unit.kind = SourceUnitKind::Unit;
        } else {
            module.source_unit.kind = SourceUnitKind::Unknown;
            module.source_unit.location = location_from_token(tokens[i]);
            return module;
        }

        module.source_unit.location = location_from_token(tokens[i]) ;

        ++i;

        while (i < tokens.size() && is_newline_token(tokens[i])) {
            ++i;
        }

        if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Identifier) {
            module.source_unit.name = tokens[i].text;
            ++i;
        }

        // continue walking
        while (i<tokens.size() && !is_end_token(tokens[i])) {
            if (is_newline_token(tokens[i])) {
                ++i;
                continue;
            }

            if (is_import_token(tokens[i])) {
                i = parse_import_declaration(tokens, i, module);
                continue;
            }

            if (is_type_token(tokens[i])) {
                i = parse_type_declaration(tokens, i, module);
                continue;
            }

            if (tokens[i].kind == flowmini::TokenKind::KeywordFn) {
                FunctionDecl fn;
                fn.location = location_from_token(tokens[i]);

                ++i;

                if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::Identifier) {
                    fn.name = tokens[i].text;
                    ++i;
                }

                i = parse_function_signature(tokens, i, fn);
                i = mark_body_container(tokens, i, fn.has_body, fn.body_location, fn.body, module.expression_pool);

                module.source_unit.declarations.emplace_back(std::move(fn));
                i = skip_until_next_top_levelish_token(tokens, i);
                continue;
            }

            if (is_main_token(tokens[i])) {
                MainBlock mainBlock;
                mainBlock.location = location_from_token(tokens[i]);

                ++i;
                i = mark_body_container(tokens, i, mainBlock.has_body, mainBlock.body_location, mainBlock.body, module.expression_pool);

                module.source_unit.declarations.emplace_back(std::move(mainBlock));

                i = skip_until_next_top_levelish_token(tokens, i);
                continue;
            }

            ++i;
        }
        return module;
    }

} // namespace flowmini::ast

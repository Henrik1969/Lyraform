#!/usr/bin/env python3
from pathlib import Path
import subprocess
import sys

if sys.argv[1:] not in ([], ["--check"]):
    print(f"usage: {Path(sys.argv[0]).name} [--check]", file=sys.stderr)
    raise SystemExit(2)

CHECK_ONLY = sys.argv[1:] == ["--check"]
ROOT = Path.cwd()

try:
    git_root = Path(subprocess.check_output(
        ["git", "rev-parse", "--show-toplevel"], text=True
    ).strip())
except (OSError, subprocess.CalledProcessError) as error:
    print(f"error: not inside a Git worktree: {error}", file=sys.stderr)
    raise SystemExit(2)

if git_root != ROOT:
    print(f"error: run from repository root: {git_root}", file=sys.stderr)
    raise SystemExit(2)

if subprocess.check_output(["git", "status", "--porcelain"], text=True).strip():
    print("error: worktree is not clean; refusing to mutate", file=sys.stderr)
    raise SystemExit(2)

FILES = {
    "header": ROOT / "Flowmini/flowmini_v24_explicit_ast/include/flowmini_ast.h",
    "builder": ROOT / "Flowmini/flowmini_v24_explicit_ast/src/flowmini_ast_builder.cpp",
    "astcpp": ROOT / "Flowmini/flowmini_v24_explicit_ast/src/flowmini_ast.cpp",
    "validator": ROOT / "Flowmini/flowmini_v24_explicit_ast/tools/validate-flowmini-ast-payloads.py",
    "probe": ROOT / "Flowmini/flowmini_v24_explicit_ast/examples/ast/statement_return_probe.flow",
}

def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly 1 match, found {count}")
    return text.replace(old, new, 1)

missing = [str(p) for k, p in FILES.items() if k != "probe" and not p.is_file()]
if missing:
    print("error: expected Flowcore repository root; missing:", file=sys.stderr)
    for p in missing:
        print(f"  {p}", file=sys.stderr)
    raise SystemExit(2)

texts = {k: p.read_text(encoding="utf-8") for k, p in FILES.items() if k != "probe"}

old = '''    bool has_initializer = false;
    bool has_value = false;
    bool has_condition = false;

    bool has_body = false;
    SourceLocation body_location;

    std::vector<Statement> body;

    // For v24 this remains intentionally skeletal.
    // Expressions and nested statement bodies are populated in later steps.
    std::vector<std::size_t> child_statements;
    std::vector<std::size_t> expressions;
'''
new = '''    bool has_initializer = false;
    bool has_value = false;
    bool has_condition = false;

    // C5 statement-role deepening starts with Return.
    // For Return this is canonical ownership; expression_ids remains a
    // derived JSON compatibility projection.
    std::optional<std::size_t> value_expression;

    bool has_body = false;
    SourceLocation body_location;

    std::vector<Statement> body;

    // Legacy statement-expression storage retained while the remaining
    // statement kinds migrate to explicit semantic roles during C5.
    std::vector<std::size_t> child_statements;
    std::vector<std::size_t> expressions;
'''
texts["header"] = replace_once(texts["header"], old, new, "flowmini_ast.h Statement")

old = '''        std::size_t add_expression_placeholder(std::vector<Expression>& expressionPool,
                                               Statement& statement,
                                               const flowmini::Token& token) {
            Expression expression;
            expression.location = location_from_token(token);
            expression.payload = make_leaf_payload(token);

            expressionPool.push_back(std::move(expression));
            const auto expressionId = expressionPool.size() - 1;
            statement.expressions.push_back(expressionId);
            return expressionId;
        }
'''
new = '''        std::size_t add_expression_placeholder(std::vector<Expression>& expressionPool,
                                               Statement& statement,
                                               const flowmini::Token& token,
                                               const bool attachToLegacyProjection = true) {
            Expression expression;
            expression.location = location_from_token(token);
            expression.payload = make_leaf_payload(token);

            expressionPool.push_back(std::move(expression));
            const auto expressionId = expressionPool.size() - 1;
            if (attachToLegacyProjection) {
                statement.expressions.push_back(expressionId);
            }
            return expressionId;
        }
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder add_expression_placeholder")

old = '''    std::size_t add_expression_placeholder_at(std::vector<Expression>& expressionPool,
                                              Statement& statement,
                                              const std::vector<flowmini::Token>& tokens,
                                              const std::size_t i) {
            if (tokens.empty()) {
                return 0;
            }

            if (i >= tokens.size()) {
                return add_expression_placeholder(expressionPool, statement, tokens.back());
            }

            auto expressionTokens = strip_enclosing_parentheses(expression_token_slice(tokens, i));
            if (expressionTokens.empty()) {
                return add_expression_placeholder(expressionPool, statement, tokens[i]);
            }

            Expression expression = make_shallow_expression_from_tokens(expressionTokens, 0);

            expressionPool.push_back(std::move(expression));
            const auto expressionId = expressionPool.size() - 1;
            statement.expressions.push_back(expressionId);

            populate_expression_children(expressionPool, expressionId, expressionTokens, 0, 0);

            return expressionId;
        }
'''
new = '''    std::size_t add_expression_placeholder_at(std::vector<Expression>& expressionPool,
                                              Statement& statement,
                                              const std::vector<flowmini::Token>& tokens,
                                              const std::size_t i,
                                              const bool attachToLegacyProjection = true) {
            if (tokens.empty()) {
                return 0;
            }

            if (i >= tokens.size()) {
                return add_expression_placeholder(
                    expressionPool, statement, tokens.back(), attachToLegacyProjection);
            }

            auto expressionTokens = strip_enclosing_parentheses(expression_token_slice(tokens, i));
            if (expressionTokens.empty()) {
                return add_expression_placeholder(
                    expressionPool, statement, tokens[i], attachToLegacyProjection);
            }

            Expression expression = make_shallow_expression_from_tokens(expressionTokens, 0);

            expressionPool.push_back(std::move(expression));
            const auto expressionId = expressionPool.size() - 1;
            if (attachToLegacyProjection) {
                statement.expressions.push_back(expressionId);
            }

            populate_expression_children(expressionPool, expressionId, expressionTokens, 0, 0);

            return expressionId;
        }
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder add_expression_placeholder_at")

old = '''        std::size_t parse_body_statement_shells(const std::vector<flowmini::Token>& tokens,
                                                std::size_t i,
                                                std::vector<Statement>& body,
                                                std::vector<Expression>& expressionPool);

        std::size_t parse_if_statement_shell(const std::vector<flowmini::Token>& tokens,
'''
new = '''        std::size_t parse_body_statement_shells(const std::vector<flowmini::Token>& tokens,
                                                std::size_t i,
                                                std::vector<Statement>& body,
                                                std::vector<Expression>& expressionPool);

        std::size_t parse_return_statement_shell(const std::vector<flowmini::Token>& tokens,
                                                 std::size_t i,
                                                 std::vector<Statement>& body,
                                                 std::vector<Expression>& expressionPool) {
            Statement statement = make_statement_shell(StatementKind::Return, tokens[i]);
            const auto valueStart = i + 1;

            if (valueStart < tokens.size() &&
                has_expression_until_statement_boundary(tokens, valueStart)) {
                statement.value_expression = add_expression_placeholder_at(
                    expressionPool,
                    statement,
                    tokens,
                    valueStart,
                    false);
            }

            body.push_back(std::move(statement));

            i = valueStart;
            while (i < tokens.size() &&
                   !is_end_token(tokens[i]) &&
                   tokens[i].kind != flowmini::TokenKind::Newline &&
                   tokens[i].kind != flowmini::TokenKind::RightBrace) {
                ++i;
            }
            return i;
        }

        std::size_t parse_if_statement_shell(const std::vector<flowmini::Token>& tokens,
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder return parser insertion")

old = '''                if (is_return_token(tokens[i])) {
                    Statement statement = make_statement_shell(StatementKind::Return, tokens[i]);
                    statement.has_value = has_expression_until_statement_boundary(tokens, i + 1);
                    if (statement.has_value && i + 1 < tokens.size()) {
                        add_expression_placeholder_at(expressionPool, statement, tokens, i + 1);
                    }
                    body.push_back(std::move(statement));
                    ++i;
                    continue;
                }
'''
new = '''                if (is_return_token(tokens[i])) {
                    i = parse_return_statement_shell(tokens, i, body, expressionPool);
                    continue;
                }
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder nested return")

old = '''            hasBody = true;
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
'''
new = '''            hasBody = true;
            bodyLocation = location_from_token(tokens[i]);

            // Function/main bodies and nested control-flow bodies share one
            // canonical statement parser.
            return parse_body_statement_shells(tokens, i, body, expressionPool);
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder body parser unification")

old = '''                for (std::size_t i = 0; i < statements.size(); ++i) {
                    const auto& statement = statements[i];

                    dump_indent(out, indent + 2);
'''
new = '''                for (std::size_t i = 0; i < statements.size(); ++i) {
                    const auto& statement = statements[i];

                    const bool hasCanonicalReturnValue =
                        statement.kind == StatementKind::Return &&
                        statement.value_expression.has_value();
                    const bool hasValue = hasCanonicalReturnValue || statement.has_value;

                    std::vector<std::size_t> projectedExpressionIds = statement.expressions;
                    if (statement.kind == StatementKind::Return) {
                        projectedExpressionIds.clear();
                        if (statement.value_expression) {
                            projectedExpressionIds.push_back(*statement.value_expression);
                        }
                    }

                    dump_indent(out, indent + 2);
'''
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp statement projections")

old = '''                    if (statement.has_value) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_value\\": true";
                    }

                    if (statement.has_condition) {
'''
new = '''                    if (hasValue) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_value\\": true";
                    }

                    if (hasCanonicalReturnValue) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"value_expression_id\\": " << *statement.value_expression;
                    }

                    if (statement.has_condition) {
'''
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp return value dump")

old = '''                    if (!statement.expressions.empty()) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"expression_ids\\": [";
                        for (std::size_t exprIndex = 0; exprIndex < statement.expressions.size(); ++exprIndex) {
                            if (exprIndex > 0) {
                                out << ", ";
                            }
                            out << statement.expressions[exprIndex];
                        }
                        out << "]";
                    }
'''
new = '''                    if (!projectedExpressionIds.empty()) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"expression_ids\\": [";
                        for (std::size_t exprIndex = 0; exprIndex < projectedExpressionIds.size(); ++exprIndex) {
                            if (exprIndex > 0) {
                                out << ", ";
                            }
                            out << projectedExpressionIds[exprIndex];
                        }
                        out << "]";
                    }
'''
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp expression projection")

old = '''def validate_statement_types(statements: object, context: str) -> None:
    if not isinstance(statements, list):
        raise TypeError(f"{context}: body_statements must be an array")
    for index, statement in enumerate(statements):
        statement_context = f"{context}: statement {index}"
        if not isinstance(statement, dict):
            raise TypeError(f"{statement_context} must be an object")
        if "type" in statement or "type_ref" in statement:
            canonical = validate_type_ref(statement.get("type_ref"), statement_context)
            if statement.get("type") != canonical:
                raise ValueError(f"{statement_context}: type string disagrees with canonical type_ref")
        if "body_statements" in statement:
            validate_statement_types(statement["body_statements"], statement_context)


def validate_declaration_types(document: dict, path: Path) -> None:
'''
new = '''def validate_statement_types(statements: object, context: str, expression_count: int) -> None:
    if not isinstance(statements, list):
        raise TypeError(f"{context}: body_statements must be an array")
    for index, statement in enumerate(statements):
        statement_context = f"{context}: statement {index}"
        if not isinstance(statement, dict):
            raise TypeError(f"{statement_context} must be an object")

        expression_ids = statement.get("expression_ids", [])
        if not isinstance(expression_ids, list) or not all(
            isinstance(expression_id, int) for expression_id in expression_ids
        ):
            raise TypeError(f"{statement_context}: expression_ids must be an integer array")
        for expression_id in expression_ids:
            if expression_id < 0 or expression_id >= expression_count:
                raise ValueError(f"{statement_context}: dangling statement expression id {expression_id}")

        if statement.get("kind") == "return":
            value_expression_id = statement.get("value_expression_id")
            if value_expression_id is None:
                if statement.get("has_value") is True or expression_ids:
                    raise ValueError(
                        f"{statement_context}: valueless return disagrees with compatibility projection"
                    )
            else:
                if not isinstance(value_expression_id, int):
                    raise TypeError(f"{statement_context}: value_expression_id must be an integer")
                if value_expression_id < 0 or value_expression_id >= expression_count:
                    raise ValueError(
                        f"{statement_context}: dangling return value expression id {value_expression_id}"
                    )
                if statement.get("has_value") is not True:
                    raise ValueError(f"{statement_context}: return value requires has_value projection")
                if expression_ids != [value_expression_id]:
                    raise ValueError(
                        f"{statement_context}: return value does not match expression_ids projection"
                    )

        if "type" in statement or "type_ref" in statement:
            canonical = validate_type_ref(statement.get("type_ref"), statement_context)
            if statement.get("type") != canonical:
                raise ValueError(f"{statement_context}: type string disagrees with canonical type_ref")
        if "body_statements" in statement:
            validate_statement_types(
                statement["body_statements"], statement_context, expression_count
            )


def validate_declaration_types(document: dict, path: Path, expression_count: int) -> None:
'''
texts["validator"] = replace_once(texts["validator"], old, new, "validator statement block")

statement_body_call = '''            validate_statement_types(declaration.get("body_statements"), context)
'''
statement_body_replacement = '''            validate_statement_types(
                declaration.get("body_statements"), context, expression_count
            )
'''
count = texts["validator"].count(statement_body_call)
if count != 2:
    raise RuntimeError(
        f"validator declaration body statements: expected exactly 2 matches, found {count}"
    )
texts["validator"] = texts["validator"].replace(
    statement_body_call, statement_body_replacement
)


old = '''    validate_declaration_types(document, path)

    expressions = document.get("expression_pool")
    if not isinstance(expressions, list):
        raise TypeError(f"{path}: expression_pool must be an array")
    if document.get("expression_pool_size") != len(expressions):
        raise ValueError(f"{path}: expression_pool_size does not match expression_pool")

    for expected_id, expression in enumerate(expressions):
'''
new = '''    expressions = document.get("expression_pool")
    if not isinstance(expressions, list):
        raise TypeError(f"{path}: expression_pool must be an array")
    if document.get("expression_pool_size") != len(expressions):
        raise ValueError(f"{path}: expression_pool_size does not match expression_pool")

    validate_declaration_types(document, path, len(expressions))

    for expected_id, expression in enumerate(expressions):
'''
texts["validator"] = replace_once(texts["validator"], old, new, "validator validation order")

probe = '''program statement_return_probe

fn direct(x : int) -> int {
    return x + 1
}

fn nested(x : int, ready : bool) -> int {
    if ready {
        return x + 1
    }
    return x
}
'''

if FILES["probe"].exists():
    existing = FILES["probe"].read_text(encoding="utf-8")
    if existing != probe:
        raise RuntimeError(
            f"{FILES['probe']}: already exists with unexpected content; refusing to overwrite"
        )

if CHECK_ONLY:
    print("C5.1 preflight successful: every expected source fragment matched; no files changed.")
    raise SystemExit(0)

for key in ("header", "builder", "astcpp", "validator"):
    FILES[key].write_text(texts[key], encoding="utf-8")

if not FILES["probe"].exists():
    FILES["probe"].write_text(probe, encoding="utf-8")

print("C5.1 local edits applied successfully.")
print("Changed:")
for key in ("header", "builder", "astcpp", "validator", "probe"):
    print(f"  {FILES[key].relative_to(ROOT)}")

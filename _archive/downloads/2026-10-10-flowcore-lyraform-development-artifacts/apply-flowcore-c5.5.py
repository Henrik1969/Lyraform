#!/usr/bin/env python3
from pathlib import Path
import argparse
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--check", action="store_true", help="preflight only; write nothing")
args = parser.parse_args()

ROOT = Path.cwd()

FILES = {
    "header": ROOT / "Flowmini/flowmini_v24_explicit_ast/include/flowmini_ast.h",
    "builder": ROOT / "Flowmini/flowmini_v24_explicit_ast/src/flowmini_ast_builder.cpp",
    "astcpp": ROOT / "Flowmini/flowmini_v24_explicit_ast/src/flowmini_ast.cpp",
    "validator": ROOT / "Flowmini/flowmini_v24_explicit_ast/tools/validate-flowmini-ast-payloads.py",
    "probe": ROOT / "Flowmini/flowmini_v24_explicit_ast/examples/ast/statement_else_probe.flow",
}

def require_repo_root():
    top = subprocess.check_output(
        ["git", "rev-parse", "--show-toplevel"],
        text=True,
        stderr=subprocess.DEVNULL,
    ).strip()
    if Path(top).resolve() != ROOT.resolve():
        raise SystemExit(f"error: run from repo root: {top}")

def replace_once(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly 1 match, found {count}")
    return text.replace(old, new, 1)

require_repo_root()

for key, path in FILES.items():
    if key != "probe" and not path.is_file():
        raise SystemExit(f"error: missing expected file: {path}")

texts = {k: p.read_text(encoding="utf-8") for k, p in FILES.items() if k != "probe"}

# C5.5 structural law:
# - else is not a StatementKind of its own.
# - an If owns an optional else branch.
# - absence of else remains absence; no synthetic empty branch is created.
# - body remains the If true branch (and While loop body).
# - else_body is the If false branch when present.

old = """    bool has_body = false;
    SourceLocation body_location;

    std::vector<Statement> body;

    // Legacy statement-expression storage retained while the remaining
"""
new = """    bool has_body = false;
    SourceLocation body_location;

    std::vector<Statement> body;

    // C5.5: `else` is an optional branch owned by If, not a standalone
    // statement. Presence is canonical in else_location; has_else in JSON is
    // derived. body remains the true branch for If and the loop body for While.
    std::optional<SourceLocation> else_location;
    SourceLocation else_body_location;
    std::vector<Statement> else_body;

    // Legacy statement-expression storage retained while the remaining
"""
texts["header"] = replace_once(texts["header"], old, new, "header else branch fields")

old = """            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                statement.has_body = true;
                statement.body_location = location_from_token(tokens[i]);
                i = parse_body_statement_shells(tokens, i, statement.body, expressionPool);
            }

            body.push_back(std::move(statement));
            return i;
        }

        std::size_t parse_while_statement_shell"""
new = """            if (i < tokens.size() && tokens[i].kind == flowmini::TokenKind::LeftBrace) {
                statement.has_body = true;
                statement.body_location = location_from_token(tokens[i]);
                i = parse_body_statement_shells(tokens, i, statement.body, expressionPool);
            }

            // Historical Flowmini law retained: `else` is optional and belongs
            // to the immediately preceding If. No synthetic else branch exists
            // when the source omits it. `else if` sugar is deliberately not
            // recognized here; an else branch must currently be a block.
            const auto elseIndex = skip_nonsemantic_separators(tokens, i);
            if (elseIndex < tokens.size() &&
                tokens[elseIndex].kind == flowmini::TokenKind::KeywordElse) {
                const auto elseBodyIndex =
                    skip_nonsemantic_separators(tokens, elseIndex + 1);
                if (elseBodyIndex < tokens.size() &&
                    tokens[elseBodyIndex].kind == flowmini::TokenKind::LeftBrace) {
                    statement.else_location = location_from_token(tokens[elseIndex]);
                    statement.else_body_location =
                        location_from_token(tokens[elseBodyIndex]);
                    i = parse_body_statement_shells(
                        tokens, elseBodyIndex, statement.else_body, expressionPool);
                }
            }

            body.push_back(std::move(statement));
            return i;
        }

        std::size_t parse_while_statement_shell"""
texts["builder"] = replace_once(texts["builder"], old, new, "builder else parsing")

old = """                    const bool hasCondition =
                        hasCanonicalCondition || statement.has_condition;

                    std::vector<std::size_t> projectedExpressionIds = statement.expressions;
"""
new = """                    const bool hasCondition =
                        hasCanonicalCondition || statement.has_condition;

                    const bool hasElse =
                        statement.kind == StatementKind::If &&
                        statement.else_location.has_value();

                    std::vector<std::size_t> projectedExpressionIds = statement.expressions;
"""
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp hasElse setup")

old = """                    if (statement.has_body) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_body\\": true";

                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"body_statement_count\\": " << statement.body.size();

                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"body_statements\\": ";
                        dump_statement_array_json(out, statement.body, indent + 4);
                    }

                    out << "\\n";
"""
new = """                    if (statement.has_body) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_body\\": true";

                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"body_statement_count\\": " << statement.body.size();

                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"body_statements\\": ";
                        dump_statement_array_json(out, statement.body, indent + 4);
                    }

                    if (hasElse) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_else\\": true";

                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"else_body_statement_count\\": "
                            << statement.else_body.size();

                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"else_body_statements\\": ";
                        dump_statement_array_json(out, statement.else_body, indent + 4);
                    }

                    out << "\\n";
"""
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp else dump")

old = """        if "type" in statement or "type_ref" in statement:
            canonical = validate_type_ref(statement.get("type_ref"), statement_context)
"""
new = """        has_else = statement.get("has_else") is True
        has_else_fields = (
            "else_body_statement_count" in statement or
            "else_body_statements" in statement
        )
        if statement.get("kind") == "if":
            if has_else:
                else_body = statement.get("else_body_statements")
                else_count = statement.get("else_body_statement_count")
                if not isinstance(else_body, list):
                    raise TypeError(
                        f"{statement_context}: has_else requires else_body_statements"
                    )
                if not isinstance(else_count, int) or else_count != len(else_body):
                    raise ValueError(
                        f"{statement_context}: else_body_statement_count does not match "
                        "else_body_statements"
                    )
                validate_statement_types(
                    else_body, f"{statement_context}: else", expression_count
                )
            elif has_else_fields:
                raise ValueError(
                    f"{statement_context}: else body fields require has_else projection"
                )
        elif has_else or has_else_fields:
            raise ValueError(
                f"{statement_context}: else branch is only valid on an if statement"
            )

        if "type" in statement or "type_ref" in statement:
            canonical = validate_type_ref(statement.get("type_ref"), statement_context)
"""
texts["validator"] = replace_once(texts["validator"], old, new, "validator else ownership")

probe = """program statement_else_probe

fn normalize(value : int) -> int {
    if value > 0 {
        return value
    } else {
        return -value
    }
}

fn keep_or_one(value : int) -> int {
    if value == 0 {
        return 1
    }

    return value
}

main {
    value : int(1)

    if value > 0 {
        value = 2
    } else {
        if value == 0 {
            value = 3
        } else {
            value = 4
        }
    }
}
"""

if FILES["probe"].exists():
    existing = FILES["probe"].read_text(encoding="utf-8")
    if existing != probe:
        raise RuntimeError(
            f"{FILES['probe']}: already exists with unexpected content; refusing to overwrite"
        )

if args.check:
    print("C5.5 preflight successful: optional If-owned else-branch edits match the C5.4 tree; no files changed.")
    raise SystemExit(0)

for key in ("header", "builder", "astcpp", "validator"):
    FILES[key].write_text(texts[key], encoding="utf-8")

if not FILES["probe"].exists():
    FILES["probe"].write_text(probe, encoding="utf-8")

print("C5.5 local edits applied successfully.")
for key in ("header", "builder", "astcpp", "validator", "probe"):
    print(f"  {FILES[key].relative_to(ROOT)}")

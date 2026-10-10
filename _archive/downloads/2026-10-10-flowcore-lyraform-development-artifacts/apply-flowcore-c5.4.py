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
    "probe": ROOT / "Flowmini/flowmini_v24_explicit_ast/examples/ast/statement_condition_probe.flow",
}

def require_repo_root() -> None:
    try:
        top = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        raise SystemExit("error: not inside a Git repository")
    if Path(top).resolve() != ROOT.resolve():
        raise SystemExit(f"error: run from repo root: {top}")

def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly 1 match, found {count}")
    return text.replace(old, new, 1)

require_repo_root()

missing = [str(p) for key, p in FILES.items() if key != "probe" and not p.is_file()]
if missing:
    print("error: expected Flowcore source files are missing:", file=sys.stderr)
    for p in missing:
        print(f"  {p}", file=sys.stderr)
    raise SystemExit(2)

texts = {k: p.read_text(encoding="utf-8") for k, p in FILES.items() if k != "probe"}

old = '''    // C5 canonical statement expression roles.
    // Let owns initializer_expression when initializer syntax contains a value
    // expression. Return and plain-name Assignment own value_expression.
    // expression_ids remains a derived JSON compatibility projection while C5
    // migration is in progress. Assignment target generalization is deferred;
    // the current Assignment name remains the canonical plain identifier target.
    std::optional<std::size_t> initializer_expression;
    std::optional<std::size_t> value_expression;
'''
new = '''    // C5 canonical statement expression roles.
    // Let owns initializer_expression when initializer syntax contains a value
    // expression. Return and plain-name Assignment own value_expression.
    // If and While own condition_expression.
    // expression_ids remains a derived JSON compatibility projection while C5
    // migration is in progress. Assignment target generalization is deferred;
    // the current Assignment name remains the canonical plain identifier target.
    std::optional<std::size_t> initializer_expression;
    std::optional<std::size_t> value_expression;
    std::optional<std::size_t> condition_expression;
'''
texts["header"] = replace_once(texts["header"], old, new, "header canonical roles")

old = '''            ++i; // consume if

            statement.has_condition = has_expression_until_body_or_line_end(tokens, i);
            if (statement.has_condition && i < tokens.size()) {
                add_expression_placeholder_at(expressionPool, statement, tokens, i);
            }
            i = skip_until_body_block_or_line_end(tokens, i);
'''
new = '''            ++i; // consume if

            const bool hasCondition = has_expression_until_body_or_line_end(tokens, i);
            if (hasCondition && i < tokens.size()) {
                statement.condition_expression = add_expression_placeholder_at(
                    expressionPool, statement, tokens, i, false);
            }
            i = skip_until_body_block_or_line_end(tokens, i);
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder if condition")

old = '''            ++i; // consume while

            statement.has_condition = has_expression_until_body_or_line_end(tokens, i);
            if (statement.has_condition && i < tokens.size()) {
                add_expression_placeholder_at(expressionPool, statement, tokens, i);
            }
            i = skip_until_body_block_or_line_end(tokens, i);
'''
new = '''            ++i; // consume while

            const bool hasCondition = has_expression_until_body_or_line_end(tokens, i);
            if (hasCondition && i < tokens.size()) {
                statement.condition_expression = add_expression_placeholder_at(
                    expressionPool, statement, tokens, i, false);
            }
            i = skip_until_body_block_or_line_end(tokens, i);
'''
texts["builder"] = replace_once(texts["builder"], old, new, "builder while condition")

old = '''                    const bool hasCanonicalValue =
                        (statement.kind == StatementKind::Return ||
                         statement.kind == StatementKind::Assignment) &&
                        statement.value_expression.has_value();
                    const bool hasValue = hasCanonicalValue || statement.has_value;

                    std::vector<std::size_t> projectedExpressionIds = statement.expressions;
'''
new = '''                    const bool hasCanonicalValue =
                        (statement.kind == StatementKind::Return ||
                         statement.kind == StatementKind::Assignment) &&
                        statement.value_expression.has_value();
                    const bool hasValue = hasCanonicalValue || statement.has_value;

                    const bool hasCanonicalCondition =
                        (statement.kind == StatementKind::If ||
                         statement.kind == StatementKind::While) &&
                        statement.condition_expression.has_value();
                    const bool hasCondition =
                        hasCanonicalCondition || statement.has_condition;

                    std::vector<std::size_t> projectedExpressionIds = statement.expressions;
'''
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp condition projection setup")

old = '''                    } else if (statement.kind == StatementKind::Return ||
                               statement.kind == StatementKind::Assignment) {
                        projectedExpressionIds.clear();
                        if (statement.value_expression) {
                            projectedExpressionIds.push_back(*statement.value_expression);
                        }
                    }

                    dump_indent(out, indent + 2);
'''
new = '''                    } else if (statement.kind == StatementKind::Return ||
                               statement.kind == StatementKind::Assignment) {
                        projectedExpressionIds.clear();
                        if (statement.value_expression) {
                            projectedExpressionIds.push_back(*statement.value_expression);
                        }
                    } else if (statement.kind == StatementKind::If ||
                               statement.kind == StatementKind::While) {
                        projectedExpressionIds.clear();
                        if (statement.condition_expression) {
                            projectedExpressionIds.push_back(*statement.condition_expression);
                        }
                    }

                    dump_indent(out, indent + 2);
'''
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp condition expression_ids projection")

old = '''                    if (statement.has_condition) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_condition\\": true";
                    }

                    if (!projectedExpressionIds.empty()) {
'''
new = '''                    if (hasCondition) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"has_condition\\": true";
                    }

                    if (hasCanonicalCondition) {
                        out << ",\\n";
                        dump_indent(out, indent + 4);
                        out << "\\"condition_expression_id\\": "
                            << *statement.condition_expression;
                    }

                    if (!projectedExpressionIds.empty()) {
'''
texts["astcpp"] = replace_once(texts["astcpp"], old, new, "ast.cpp condition dump")

old = '''        if "type" in statement or "type_ref" in statement:
            canonical = validate_type_ref(statement.get("type_ref"), statement_context)
'''
new = '''        if statement.get("kind") in {"if", "while"}:
            condition_role = statement["kind"]
            condition_expression_id = statement.get("condition_expression_id")
            if condition_expression_id is None:
                if statement.get("has_condition") is True or expression_ids:
                    raise ValueError(
                        f"{statement_context}: conditionless {condition_role} disagrees with "
                        "compatibility projection"
                    )
            else:
                if not isinstance(condition_expression_id, int):
                    raise TypeError(
                        f"{statement_context}: condition_expression_id must be an integer"
                    )
                if condition_expression_id < 0 or condition_expression_id >= expression_count:
                    raise ValueError(
                        f"{statement_context}: dangling {condition_role} condition expression id "
                        f"{condition_expression_id}"
                    )
                if statement.get("has_condition") is not True:
                    raise ValueError(
                        f"{statement_context}: {condition_role} condition requires "
                        "has_condition projection"
                    )
                if expression_ids != [condition_expression_id]:
                    raise ValueError(
                        f"{statement_context}: {condition_role} condition does not match "
                        "expression_ids projection"
                    )

        if "type" in statement or "type_ref" in statement:
            canonical = validate_type_ref(statement.get("type_ref"), statement_context)
'''
texts["validator"] = replace_once(texts["validator"], old, new, "validator condition ownership")

probe = '''program statement_condition_probe

fn adjust(value : int, limit : int) -> int {
    if value + 1 < limit {
        value = value + 1
    }

    while value < limit {
        if value == 2 {
            value = value + 1
        }
        value = value + 1
    }

    return value
}

main {
    value : int(0)

    if value == 0 {
        while value < 2 {
            value = value + 1
        }
    }
}
'''

if FILES["probe"].exists():
    existing = FILES["probe"].read_text(encoding="utf-8")
    if existing != probe:
        raise RuntimeError(
            f"{FILES['probe']}: already exists with unexpected content; refusing to overwrite"
        )

if args.check:
    print("C5.4 preflight successful: If/While condition ownership edits match the current tree; no files changed.")
    raise SystemExit(0)

for key in ("header", "builder", "astcpp", "validator"):
    FILES[key].write_text(texts[key], encoding="utf-8")

if not FILES["probe"].exists():
    FILES["probe"].write_text(probe, encoding="utf-8")

print("C5.4 local edits applied successfully.")
for key in ("header", "builder", "astcpp", "validator", "probe"):
    print(f"  {FILES[key].relative_to(ROOT)}")

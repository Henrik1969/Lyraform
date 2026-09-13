#!/usr/bin/env python3

"""Validate canonical Flowmini AST payloads and compatibility projections."""

import json
import sys
from pathlib import Path


def fail(path: Path, expression_id: int, message: str) -> None:
    raise ValueError(f"{path}: expression {expression_id}: {message}")


def optional_id(payload: dict, name: str) -> list[int]:
    value = payload.get(name)
    if value is None:
        return []
    if not isinstance(value, int):
        raise TypeError(f"payload field {name!r} must be an expression id or null")
    return [value]


def id_array(payload: dict, name: str) -> list[int]:
    value = payload.get(name)
    if not isinstance(value, list) or not all(isinstance(item, int) for item in value):
        raise TypeError(f"payload field {name!r} must be an array of expression ids")
    return value


def payload_children(kind: str, payload: dict) -> list[int]:
    if kind in {
        "unknown",
        "identifier",
        "integer_literal",
        "float_literal",
        "string_literal",
        "bool_literal",
    }:
        return []
    if kind == "unary":
        return optional_id(payload, "operand")
    if kind == "binary":
        return optional_id(payload, "left") + optional_id(payload, "right")
    if kind == "call":
        return optional_id(payload, "base") + id_array(payload, "arguments")
    if kind == "index":
        return optional_id(payload, "base") + id_array(payload, "indexes")
    if kind == "field_access":
        field = payload.get("field")
        if not isinstance(field, str) or not field:
            raise TypeError("field_access payload requires a non-empty field name")
        return optional_id(payload, "base")
    if kind == "list_literal":
        return id_array(payload, "elements")
    if kind == "record_literal":
        fields = payload.get("fields")
        if not isinstance(fields, list):
            raise TypeError("record_literal payload field 'fields' must be an array")

        children: list[int] = []
        for field in fields:
            if not isinstance(field, dict):
                raise TypeError("record literal fields must be objects")
            if not isinstance(field.get("name"), str) or not field["name"]:
                raise TypeError("record literal fields require non-empty names")
            location = field.get("location")
            if not isinstance(location, dict) or not all(
                isinstance(location.get(component), int) for component in ("line", "column")
            ):
                raise TypeError("record literal fields require line/column locations")
            children.extend(optional_id(field, "value"))
        return children

    raise ValueError(f"unsupported expression kind {kind!r}")


def location(value: object, context: str) -> None:
    if not isinstance(value, dict) or not all(
        isinstance(value.get(component), int) for component in ("line", "column")
    ):
        raise TypeError(f"{context} requires a line/column location")


def string_segments(payload: dict, name: str) -> list[str]:
    value = payload.get(name)
    if not isinstance(value, list) or not value or not all(
        isinstance(segment, str) and segment for segment in value
    ):
        raise TypeError(f"type payload field {name!r} must be a non-empty string array")
    return value


def validate_type_ref(type_ref: object, context: str) -> str:
    if not isinstance(type_ref, dict):
        raise TypeError(f"{context}: canonical type_ref object is required")

    kind = type_ref.get("kind")
    text = type_ref.get("text")
    payload = type_ref.get("payload")
    if not isinstance(kind, str) or not isinstance(text, str) or not isinstance(payload, dict):
        raise TypeError(f"{context}: type_ref requires kind, text, and payload")
    location(type_ref.get("location"), f"{context}: type_ref")

    if kind == "unknown":
        canonical = payload.get("text")
        if not isinstance(canonical, str):
            raise TypeError(f"{context}: unknown type payload requires text")
    elif kind == "named":
        canonical = ".".join(string_segments(payload, "name_segments"))
    elif kind == "generic":
        constructor = ".".join(string_segments(payload, "constructor_segments"))
        arguments = payload.get("arguments")
        if not isinstance(arguments, list) or not arguments:
            raise TypeError(f"{context}: generic type requires one or more arguments")
        canonical = constructor + "<" + ",".join(
            validate_type_ref(argument, f"{context}: generic argument {index}")
            for index, argument in enumerate(arguments)
        ) + ">"
    elif kind == "array":
        element = payload.get("element_type")
        if element is None:
            raise TypeError(f"{context}: array type requires an element_type")
        canonical = "array<" + validate_type_ref(element, f"{context}: array element") + ">"
        extents = payload.get("extents")
        if not isinstance(extents, list):
            raise TypeError(f"{context}: array extents must be an array")
        extent_texts: list[str] = []
        for index, extent in enumerate(extents):
            if not isinstance(extent, dict) or not isinstance(extent.get("text"), str) or not extent["text"]:
                raise TypeError(f"{context}: array extent {index} requires non-empty text")
            location(extent.get("location"), f"{context}: array extent {index}")
            extent_texts.append(extent["text"])
        if extent_texts:
            canonical += "[" + ",".join(extent_texts) + "]"
    else:
        raise ValueError(f"{context}: unsupported type_ref kind {kind!r}")

    if canonical != text:
        raise ValueError(f"{context}: canonical type text {canonical!r} != projection {text!r}")
    return canonical


def validate_statement_types(statements: object, context: str) -> None:
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


def statement_optional_id(payload: dict, field: str, context: str) -> list[int]:
    value = payload.get(field)
    if value is None:
        return []
    if not isinstance(value, int):
        raise TypeError(f"{context}: {field} must be an expression id or null")
    return [value]


def require_exact_fields(payload: dict, fields: set[str], context: str) -> None:
    actual = set(payload)
    if actual != fields:
        raise ValueError(
            f"{context}: payload fields {sorted(actual)} != required {sorted(fields)}"
        )


def validate_assignable_target(target: object, context: str) -> list[int]:
    if not isinstance(target, dict):
        raise TypeError(f"{context}: assignable target must be an object")
    kind = target.get("kind")
    if kind == "identifier":
        require_exact_fields(target, {"kind", "name", "location"}, context)
        if not isinstance(target.get("name"), str) or not target["name"]:
            raise TypeError(f"{context}: identifier target requires a non-empty name")
        location(target.get("location"), f"{context}: identifier target")
        return []
    if kind == "field_path":
        require_exact_fields(
            target, {"kind", "base_identifier", "location", "fields"}, context
        )
        if not isinstance(target.get("base_identifier"), str) or not target["base_identifier"]:
            raise TypeError(f"{context}: field-path target requires a base identifier")
        location(target.get("location"), f"{context}: field-path target")
        fields = target.get("fields")
        if not isinstance(fields, list) or not fields:
            raise TypeError(f"{context}: field-path target requires one or more fields")
        for index, field in enumerate(fields):
            if not isinstance(field, dict) or not isinstance(field.get("name"), str) or not field["name"]:
                raise TypeError(f"{context}: field {index} requires a non-empty name")
            location(field.get("location"), f"{context}: field {index}")
        return []
    if kind == "indexed":
        require_exact_fields(
            target, {"kind", "base_identifier", "location", "indexes"}, context
        )
        if not isinstance(target.get("base_identifier"), str) or not target["base_identifier"]:
            raise TypeError(f"{context}: indexed target requires a base identifier")
        location(target.get("location"), f"{context}: indexed target")
        indexes = target.get("indexes")
        if not isinstance(indexes, list) or not indexes or not all(isinstance(index, int) for index in indexes):
            raise TypeError(f"{context}: indexed target requires one or more expression IDs")
        return indexes
    raise ValueError(f"{context}: unsupported assignable target kind {kind!r}")


def validate_statement_payload(statement: dict, context: str) -> None:
    kind = statement.get("kind")
    payload = statement.get("payload")
    if not isinstance(kind, str) or not isinstance(payload, dict):
        raise TypeError(f"{context}: kind and canonical payload are required")

    canonical_expressions: list[int] | None = None
    if kind == "let":
        require_exact_fields(payload, {"name", "type_ref", "initializer_expression"}, context)
        if not isinstance(payload.get("name"), str) or not payload["name"]:
            raise TypeError(f"{context}: let payload requires a non-empty name")
        canonical_type = validate_type_ref(payload.get("type_ref"), f"{context}: let payload")
        if statement.get("name") != payload["name"]:
            raise ValueError(f"{context}: name projection disagrees with let payload")
        if statement.get("type") != canonical_type or statement.get("type_ref") != payload["type_ref"]:
            raise ValueError(f"{context}: type projection disagrees with let payload")
        canonical_expressions = statement_optional_id(payload, "initializer_expression", context)
        if statement.get("has_initializer", False) != bool(canonical_expressions):
            raise ValueError(f"{context}: has_initializer projection disagrees with let payload")
    elif kind == "assignment":
        require_exact_fields(payload, {"target", "value_expression", "source_form"}, context)
        target_expressions = validate_assignable_target(
            payload.get("target"), f"{context}: assignment payload"
        )
        if payload.get("source_form") != "equals_assignment":
            raise ValueError(f"{context}: assignment requires equals_assignment source form")
        target = payload["target"]
        if target.get("kind") == "identifier" and statement.get("name") != target.get("name"):
            raise ValueError(f"{context}: name projection disagrees with assignment target")
        canonical_expressions = (
            statement_optional_id(payload, "value_expression", context) + target_expressions
        )
        if statement.get("has_value", False) != bool(canonical_expressions):
            raise ValueError(f"{context}: has_value projection disagrees with assignment payload")
    elif kind == "placement":
        require_exact_fields(payload, {"value_expression", "target", "source_form"}, context)
        target_expressions = validate_assignable_target(
            payload.get("target"), f"{context}: placement payload"
        )
        if payload.get("source_form") != "arrow_placement":
            raise ValueError(f"{context}: placement requires arrow_placement source form")
        canonical_expressions = (
            statement_optional_id(payload, "value_expression", context) + target_expressions
        )
        if not statement_optional_id(payload, "value_expression", context):
            raise ValueError(f"{context}: placement requires a value expression")
        if statement.get("has_value", False) is not True:
            raise ValueError(f"{context}: placement must project has_value")
    elif kind == "return":
        require_exact_fields(payload, {"value_expression", "source_form"}, context)
        if payload.get("source_form") not in {"keyword_return", "arrow_placement"}:
            raise ValueError(f"{context}: return has invalid source form")
        canonical_expressions = statement_optional_id(payload, "value_expression", context)
        if statement.get("has_value", False) != bool(canonical_expressions):
            raise ValueError(f"{context}: has_value projection disagrees with return payload")
    elif kind == "if":
        require_exact_fields(payload, {"condition_expression", "then_block", "else_arm"}, context)
        canonical_expressions = statement_optional_id(payload, "condition_expression", context)
        if len(canonical_expressions) != 1:
            raise ValueError(f"{context}: if requires a condition expression")
        if statement.get("condition") != canonical_expressions[0]:
            raise ValueError(f"{context}: condition projection disagrees with if payload")
        if statement.get("has_condition", False) is not True:
            raise ValueError(f"{context}: if must project has_condition")
        if statement.get("then_block") != payload.get("then_block"):
            raise ValueError(f"{context}: then_block projection disagrees with if payload")
        if not isinstance(payload.get("then_block"), int):
            raise TypeError(f"{context}: if requires a then_block ID")
        if statement.get("else_arm") != payload.get("else_arm"):
            raise ValueError(f"{context}: else_arm projection disagrees with if payload")
    elif kind == "while":
        require_exact_fields(payload, {"condition_expression", "body_block"}, context)
        canonical_expressions = statement_optional_id(payload, "condition_expression", context)
        if len(canonical_expressions) != 1:
            raise ValueError(f"{context}: while requires a condition expression")
        if statement.get("has_condition", False) is not True:
            raise ValueError(f"{context}: while must project has_condition")
        if statement.get("body_block") != payload.get("body_block"):
            raise ValueError(f"{context}: body_block projection disagrees with while payload")
        if not isinstance(payload.get("body_block"), int):
            raise TypeError(f"{context}: while requires a body_block ID")
    elif kind in {"break", "continue"}:
        require_exact_fields(payload, set(), context)
    elif kind == "expression":
        require_exact_fields(payload, {"expression", "print"}, context)
        if not isinstance(payload.get("print"), bool):
            raise TypeError(f"{context}: expression payload field 'print' must be boolean")
        canonical_expressions = statement_optional_id(payload, "expression", context)
    elif kind == "flow":
        require_exact_fields(payload, {"expressions"}, context)
        canonical_expressions = id_array(payload, "expressions")
    elif kind == "unknown":
        require_exact_fields(payload, {"text"}, context)
        if not isinstance(payload.get("text"), str):
            raise TypeError(f"{context}: unknown payload requires text")
    else:
        raise ValueError(f"{context}: unsupported statement kind {kind!r}")

    if canonical_expressions is not None and statement.get("expression_ids", []) != canonical_expressions:
        raise ValueError(f"{context}: expression_ids projection disagrees with canonical payload")


def validate_declaration_types(document: dict, path: Path) -> None:
    declarations = document.get("declaration_pool")
    if not isinstance(declarations, list):
        raise TypeError(f"{path}: declaration_pool must be an array")

    for index, declaration in enumerate(declarations):
        context = f"{path}: declaration {index}"
        if not isinstance(declaration, dict):
            raise TypeError(f"{context} must be an object")
        kind = declaration.get("kind")
        if kind == "function":
            parameters = declaration.get("parameters")
            if not isinstance(parameters, list):
                raise TypeError(f"{context}: function parameters must be an array")
            for parameter_index, parameter in enumerate(parameters):
                parameter_context = f"{context}: parameter {parameter_index}"
                canonical = validate_type_ref(parameter.get("type_ref"), parameter_context)
                if parameter.get("type") != canonical:
                    raise ValueError(f"{parameter_context}: type string disagrees with canonical type_ref")
            return_type = validate_type_ref(declaration.get("return_type_ref"), f"{context}: return type")
            if declaration.get("return_type") != return_type:
                raise ValueError(f"{context}: return_type string disagrees with canonical type_ref")
        elif kind == "record":
            fields = declaration.get("fields")
            if not isinstance(fields, list):
                raise TypeError(f"{context}: record fields must be an array")
            for field_index, field in enumerate(fields):
                field_context = f"{context}: field {field_index}"
                canonical = validate_type_ref(field.get("type_ref"), field_context)
                if field.get("type") != canonical:
                    raise ValueError(f"{field_context}: type string disagrees with canonical type_ref")
        elif kind == "refined_type":
            require_exact_fields(
                declaration,
                {"id", "kind", "name", "base_type", "base_type_ref", "invariants", "location"},
                context,
            )
            canonical = validate_type_ref(
                declaration.get("base_type_ref"), f"{context}: refined base type"
            )
            if declaration.get("base_type") != canonical:
                raise ValueError(f"{context}: base_type string disagrees with canonical type_ref")
            invariants = declaration.get("invariants")
            if not isinstance(invariants, list):
                raise TypeError(f"{context}: invariants must be an array")
            for invariant_index, invariant in enumerate(invariants):
                invariant_context = f"{context}: invariant {invariant_index}"
                if not isinstance(invariant, dict):
                    raise TypeError(f"{invariant_context} must be an object")
                require_exact_fields(
                    invariant, {"condition_expression", "location"}, invariant_context
                )
                if not isinstance(invariant.get("condition_expression"), int):
                    raise TypeError(f"{invariant_context}: condition_expression must be an ID")
                location(invariant.get("location"), invariant_context)
            location(declaration.get("location"), context)
        elif kind == "abi":
            require_exact_fields(declaration, {"id", "kind", "name", "members", "location"}, context)
            members = declaration.get("members")
            if not isinstance(members, list):
                raise TypeError(f"{context}: ABI members must be an array")
            for member_index, member in enumerate(members):
                member_context = f"{context}: ABI member {member_index}"
                if not isinstance(member, dict):
                    raise TypeError(f"{member_context} must be an object")
                member_kind = member.get("kind")
                if member_kind in {"library", "convention"}:
                    require_exact_fields(
                        member, {"kind", "spelling", "location"}, member_context
                    )
                    if not isinstance(member.get("spelling"), str):
                        raise TypeError(f"{member_context}: spelling must be a string")
                    location(member.get("location"), member_context)
                elif member_kind == "type":
                    require_exact_fields(
                        member, {"kind", "name", "properties", "location"}, member_context
                    )
                    properties = member.get("properties")
                    if not isinstance(properties, list):
                        raise TypeError(f"{member_context}: properties must be an array")
                    for property_index, prop in enumerate(properties):
                        property_context = f"{member_context}: property {property_index}"
                        if not isinstance(prop, dict):
                            raise TypeError(f"{property_context} must be an object")
                        require_exact_fields(
                            prop, {"kind", "spelling", "location"}, property_context
                        )
                        if prop.get("kind") not in {
                            "repr", "ownership", "access", "lifetime",
                            "nullable", "terminator", "opaque",
                        }:
                            raise ValueError(f"{property_context}: unsupported ABI type property")
                        if not isinstance(prop.get("spelling"), str):
                            raise TypeError(f"{property_context}: spelling must be a string")
                        location(prop.get("location"), property_context)
                    location(member.get("location"), member_context)
                elif member_kind == "struct":
                    require_exact_fields(
                        member, {"kind", "name", "fields", "location"}, member_context
                    )
                    fields = member.get("fields")
                    if not isinstance(fields, list):
                        raise TypeError(f"{member_context}: fields must be an array")
                    for field_index, field in enumerate(fields):
                        field_context = f"{member_context}: field {field_index}"
                        if not isinstance(field, dict):
                            raise TypeError(f"{field_context} must be an object")
                        require_exact_fields(
                            field, {"name", "type", "type_ref", "location"}, field_context
                        )
                        canonical = validate_type_ref(field.get("type_ref"), field_context)
                        if field.get("type") != canonical:
                            raise ValueError(f"{field_context}: type disagrees with type_ref")
                        location(field.get("location"), field_context)
                    location(member.get("location"), member_context)
                elif member_kind == "extern_function":
                    require_exact_fields(
                        member,
                        {"kind", "name", "parameters", "return_type", "return_type_ref", "clauses", "location"},
                        member_context,
                    )
                    parameters = member.get("parameters")
                    if not isinstance(parameters, list):
                        raise TypeError(f"{member_context}: parameters must be an array")
                    for parameter_index, parameter in enumerate(parameters):
                        parameter_context = f"{member_context}: parameter {parameter_index}"
                        if not isinstance(parameter, dict):
                            raise TypeError(f"{parameter_context} must be an object")
                        require_exact_fields(
                            parameter, {"name", "type", "type_ref", "location"}, parameter_context
                        )
                        canonical = validate_type_ref(parameter.get("type_ref"), parameter_context)
                        if parameter.get("type") != canonical:
                            raise ValueError(f"{parameter_context}: type disagrees with type_ref")
                        location(parameter.get("location"), parameter_context)
                    canonical_return = validate_type_ref(
                        member.get("return_type_ref"), f"{member_context}: return type"
                    )
                    if member.get("return_type") != canonical_return:
                        raise ValueError(f"{member_context}: return type disagrees with type_ref")
                    clauses = member.get("clauses")
                    if not isinstance(clauses, list):
                        raise TypeError(f"{member_context}: clauses must be an array")
                    for clause_index, clause in enumerate(clauses):
                        clause_context = f"{member_context}: clause {clause_index}"
                        if not isinstance(clause, dict):
                            raise TypeError(f"{clause_context} must be an object")
                        require_exact_fields(
                            clause, {"kind", "spelling", "location"}, clause_context
                        )
                        if clause.get("kind") not in {"symbol", "effect"}:
                            raise ValueError(f"{clause_context}: unsupported extern clause")
                        if not isinstance(clause.get("spelling"), str):
                            raise TypeError(f"{clause_context}: spelling must be a string")
                        location(clause.get("location"), clause_context)
                    location(member.get("location"), member_context)
                else:
                    raise ValueError(f"{member_context}: unsupported ABI member kind {member_kind!r}")
            location(declaration.get("location"), context)
        elif kind == "target":
            require_exact_fields(
                declaration, {"id", "kind", "name", "declaration_ids", "location"}, context
            )
            if not isinstance(declaration.get("name"), str):
                raise TypeError(f"{context}: target name must be a string")
            children = declaration.get("declaration_ids")
            if not isinstance(children, list) or not all(isinstance(item, int) for item in children):
                raise TypeError(f"{context}: target declaration_ids must be an ID array")
            location(declaration.get("location"), context)


def invariant_expression_ids(document: dict) -> list[int]:
    declarations = document.get("declaration_pool", [])
    result: list[int] = []
    if not isinstance(declarations, list):
        return result
    for declaration in declarations:
        if isinstance(declaration, dict) and declaration.get("kind") == "refined_type":
            for invariant in declaration.get("invariants", []):
                if isinstance(invariant, dict) and isinstance(invariant.get("condition_expression"), int):
                    result.append(invariant["condition_expression"])
    return result


def validate_declaration_arena(document: dict, path: Path) -> None:
    source_unit = document.get("source_unit")
    if not isinstance(source_unit, dict):
        raise TypeError(f"{path}: source_unit must be an object")

    pool = document.get("declaration_pool")
    if not isinstance(pool, list) or document.get("declaration_pool_size") != len(pool):
        raise TypeError(f"{path}: invalid declaration_pool")

    for expected_id, declaration in enumerate(pool):
        if not isinstance(declaration, dict) or declaration.get("id") != expected_id:
            raise ValueError(f"{path}: declaration IDs must match their pool positions")

    ids = source_unit.get("declaration_ids")
    projections = source_unit.get("declarations")
    if not isinstance(ids, list) or not all(isinstance(item, int) for item in ids):
        raise TypeError(f"{path}: source_unit declaration_ids must be an ID array")
    if source_unit.get("declaration_count") != len(ids):
        raise ValueError(f"{path}: declaration_count does not match declaration_ids")
    if len(ids) != len(set(ids)):
        raise ValueError(f"{path}: declaration has multiple structural parents")
    owned = []

    def collect(declaration_id: int, owner: str) -> None:
        if declaration_id < 0 or declaration_id >= len(pool):
            raise ValueError(f"{path}: {owner} references dangling declaration {declaration_id}")
        if declaration_id in owned:
            raise ValueError(f"{path}: declaration {declaration_id} has multiple structural parents")
        owned.append(declaration_id)
        declaration = pool[declaration_id]
        if declaration.get("kind") == "target":
            for child in declaration.get("declaration_ids", []):
                collect(child, f"target {declaration_id}")

    for declaration_id in ids:
        collect(declaration_id, "source_unit")
    if set(owned) != set(range(len(pool))):
        raise ValueError(f"{path}: declaration pool must be wholly owned by source/target structure")
    if not isinstance(projections, list) or projections != [pool[item] for item in ids]:
        raise ValueError(f"{path}: source_unit declaration projections disagree with pool IDs")


def validate_statement_arenas(document: dict, path: Path) -> None:
    statements = document.get("statement_pool")
    blocks = document.get("block_pool")
    if not isinstance(statements, list) or document.get("statement_pool_size") != len(statements):
        raise TypeError(f"{path}: invalid statement_pool")
    if not isinstance(blocks, list) or document.get("block_pool_size") != len(blocks):
        raise TypeError(f"{path}: invalid block_pool")

    parents: dict[int, str] = {}
    for block_id, block in enumerate(blocks):
        if not isinstance(block, dict) or block.get("id") != block_id:
            raise ValueError(f"{path}: block IDs must match their pool positions")
        children = block.get("statements")
        if not isinstance(children, list) or not all(isinstance(child, int) for child in children):
            raise TypeError(f"{path}: block {block_id}: statements must be IDs")
        for child in children:
            if child < 0 or child >= len(statements):
                raise ValueError(f"{path}: block {block_id}: dangling statement {child}")
            if child in parents:
                raise ValueError(f"{path}: statement {child} has multiple structural parents")
            parents[child] = f"block {block_id}"

    for statement_id, statement in enumerate(statements):
        if not isinstance(statement, dict) or statement.get("id") != statement_id:
            raise ValueError(f"{path}: statement IDs must match their pool positions")
        validate_statement_payload(statement, f"{path}: statement {statement_id}")
        validate_statement_types([statement], f"{path}: statement_pool")
        for field in ("then_block", "body_block"):
            if field in statement and statement[field] not in range(len(blocks)):
                raise ValueError(f"{path}: statement {statement_id}: dangling {field}")
        if statement.get("kind") == "if":
            if "then_block" not in statement or not isinstance(statement.get("condition"), int):
                raise ValueError(f"{path}: if statement {statement_id} requires condition and then_block")
            arm = statement.get("else_arm")
            if arm is not None:
                if not isinstance(arm, dict) or arm.get("kind") not in {"else_block", "else_if"}:
                    raise TypeError(f"{path}: if statement {statement_id}: invalid else_arm")
                target = arm.get("block" if arm["kind"] == "else_block" else "if_statement")
                pool_size = len(blocks) if arm["kind"] == "else_block" else len(statements)
                if not isinstance(target, int) or target not in range(pool_size):
                    raise ValueError(f"{path}: if statement {statement_id}: dangling else arm")
                if arm["kind"] == "else_if":
                    if statements[target].get("kind") != "if":
                        raise ValueError(f"{path}: statement {statement_id}: else_if target is not an if")
                    if target in parents:
                        raise ValueError(f"{path}: statement {target} has multiple structural parents")
                    parents[target] = f"if statement {statement_id} else_arm"

    orphaned = sorted(set(range(len(statements))) - set(parents))
    if orphaned:
        raise ValueError(f"{path}: statements without a structural parent: {orphaned}")


def validate(path: Path) -> None:
    with path.open(encoding="utf-8") as stream:
        document = json.load(stream)

    validate_declaration_arena(document, path)
    validate_declaration_types(document, path)
    validate_statement_arenas(document, path)

    expressions = document.get("expression_pool")
    if not isinstance(expressions, list):
        raise TypeError(f"{path}: expression_pool must be an array")
    if document.get("expression_pool_size") != len(expressions):
        raise ValueError(f"{path}: expression_pool_size does not match expression_pool")

    for expression_id in invariant_expression_ids(document):
        if expression_id < 0 or expression_id >= len(expressions):
            raise ValueError(f"{path}: dangling invariant expression id {expression_id}")

    for statement in document["statement_pool"]:
        for expression_id in statement.get("expression_ids", []):
            if expression_id < 0 or expression_id >= len(expressions):
                raise ValueError(
                    f"{path}: statement {statement['id']}: dangling expression id {expression_id}"
                )

    for expected_id, expression in enumerate(expressions):
        if expression.get("id") != expected_id:
            fail(path, expected_id, "expression IDs must match their pool positions")

        kind = expression.get("kind")
        payload = expression.get("payload")
        projected = expression.get("child_expressions")
        if not isinstance(kind, str) or not isinstance(payload, dict):
            fail(path, expected_id, "kind and canonical payload are required")
        if not isinstance(projected, list):
            fail(path, expected_id, "child_expressions compatibility projection is required")

        try:
            canonical = payload_children(kind, payload)
        except (TypeError, ValueError) as error:
            fail(path, expected_id, str(error))

        if canonical != projected:
            fail(path, expected_id, f"payload children {canonical} != projection {projected}")

        for child_id in canonical:
            if child_id < 0 or child_id >= len(expressions):
                fail(path, expected_id, f"dangling child expression id {child_id}")


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} AST_JSON", file=sys.stderr)
        return 2

    path = Path(sys.argv[1])
    try:
        validate(path)
    except (OSError, TypeError, ValueError, json.JSONDecodeError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

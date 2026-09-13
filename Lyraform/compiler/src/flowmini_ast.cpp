// Flowmini AST
// ------------
// This is the language-aware parsed source representation.
//
// It is intentionally separate from TokenTree.
// TokenTree preserves grouped token structure.
// AST represents Flowmini source meaning.

#include "flowmini_ast.h"
#include <type_traits>
#include <variant>

namespace flowmini::ast {
    // Helpers
    namespace {

        void dump_indent(std::ostream& out, std::size_t spaces) {

            for (std::size_t i = 0; i < spaces; ++i) {
                out << ' ';
            }
        }

        void dump_json_string(std::ostream& out, const std::string& value) {
            out << '"';

            for (const char ch : value) {
                switch (ch) {
                    case '\\': out << "\\\\"; break;
                    case '"':  out << "\\\""; break;
                    case '\n': out << "\\n";  break;
                    case '\r': out << "\\r";  break;
                    case '\t': out << "\\t";  break;
                    default:   out << ch;     break;
                }
            }

            out << '"';
        }

        void dump_string_array(std::ostream& out, const std::vector<std::string>& values) {
            out << "[";
            for (std::size_t i = 0; i < values.size(); ++i) {
                if (i != 0) { out << ", "; }
                dump_json_string(out, values[i]);
            }
            out << "]";
        }

        std::string render_qualified_name(const std::vector<std::string>& segments) {
            std::string result;
            for (std::size_t i = 0; i < segments.size(); ++i) {
                if (i != 0) { result += "."; }
                result += segments[i];
            }
            return result;
        }

        std::string render_type_ref(const TypeRef& type, const std::size_t depth = 0) {
            if (depth > 64) { return {}; }
            if (const auto* value = std::get_if<UnknownTypeRef>(&type.payload)) {
                return value->text;
            }
            if (const auto* value = std::get_if<NamedTypeRef>(&type.payload)) {
                return render_qualified_name(value->name_segments);
            }
            if (const auto* value = std::get_if<GenericTypeRef>(&type.payload)) {
                std::string result = render_qualified_name(value->constructor_segments) + "<";
                for (std::size_t i = 0; i < value->arguments.size(); ++i) {
                    if (i != 0) { result += ","; }
                    result += render_type_ref(value->arguments[i], depth + 1);
                }
                return result + ">";
            }
            if (const auto* value = std::get_if<ArrayTypeRef>(&type.payload)) {
                std::string result = "array<";
                if (value->element_type) {
                    result += render_type_ref(*value->element_type, depth + 1);
                }
                result += ">";
                if (!value->extents.empty()) {
                    result += "[";
                    for (std::size_t i = 0; i < value->extents.size(); ++i) {
                        if (i != 0) { result += ","; }
                        result += value->extents[i].text;
                    }
                    result += "]";
                }
                return result;
            }
            return {};
        }

        void dump_type_ref_json(std::ostream& out, const TypeRef& type);

        void dump_type_ref_payload_json(std::ostream& out, const TypeRef& type) {
            out << "{";
            if (const auto* value = std::get_if<UnknownTypeRef>(&type.payload)) {
                out << "\"text\": ";
                dump_json_string(out, value->text);
            } else if (const auto* value = std::get_if<NamedTypeRef>(&type.payload)) {
                out << "\"name_segments\": ";
                dump_string_array(out, value->name_segments);
            } else if (const auto* value = std::get_if<GenericTypeRef>(&type.payload)) {
                out << "\"constructor_segments\": ";
                dump_string_array(out, value->constructor_segments);
                out << ", \"arguments\": [";
                for (std::size_t i = 0; i < value->arguments.size(); ++i) {
                    if (i != 0) { out << ", "; }
                    dump_type_ref_json(out, value->arguments[i]);
                }
                out << "]";
            } else if (const auto* value = std::get_if<ArrayTypeRef>(&type.payload)) {
                out << "\"element_type\": ";
                if (value->element_type) {
                    dump_type_ref_json(out, *value->element_type);
                } else {
                    out << "null";
                }
                out << ", \"extents\": [";
                for (std::size_t i = 0; i < value->extents.size(); ++i) {
                    if (i != 0) { out << ", "; }
                    out << "{\"text\": ";
                    dump_json_string(out, value->extents[i].text);
                    out << ", \"location\": {\"line\": " << value->extents[i].location.line
                        << ", \"column\": " << value->extents[i].location.column << "}}";
                }
                out << "]";
            }
            out << "}";
        }

        void dump_type_ref_json(std::ostream& out, const TypeRef& type) {
            out << "{\"kind\": ";
            dump_json_string(out, to_string(type_ref_kind(type)));
            out << ", \"text\": ";
            dump_json_string(out, type_ref_text(type));
            out << ", \"payload\": ";
            dump_type_ref_payload_json(out, type);
            out << ", \"location\": {\"line\": " << type.location.line
                << ", \"column\": " << type.location.column << "}}";
        }

        std::string render_expression_full(const std::size_t expression_id,
                                           const std::vector<Expression>& expressions,
                                           const std::size_t depth = 0) {
            if (expression_id >= expressions.size() || depth >= 64) {
                return {};
            }

            const auto& expression = expressions[expression_id];
            const auto render_child = [&](const std::optional<std::size_t>& child) {
                return child ? render_expression_full(*child, expressions, depth + 1) : std::string{};
            };
            const auto render_postfix_base = [&](const std::optional<std::size_t>& child) {
                if (!child || *child >= expressions.size()) {
                    return std::string{};
                }
                auto rendered = render_expression_full(*child, expressions, depth + 1);
                const auto& payload = expressions[*child].payload;
                if (std::holds_alternative<BinaryExpr>(payload) ||
                    std::holds_alternative<UnaryExpr>(payload)) {
                    return "(" + rendered + ")";
                }
                return rendered;
            };

            if (const auto* value = std::get_if<UnknownExpr>(&expression.payload)) {
                return value->text;
            }
            if (const auto* value = std::get_if<IdentifierExpr>(&expression.payload)) {
                return value->name;
            }
            if (const auto* value = std::get_if<IntegerLiteralExpr>(&expression.payload)) {
                return value->text;
            }
            if (const auto* value = std::get_if<FloatLiteralExpr>(&expression.payload)) {
                return value->text;
            }
            if (const auto* value = std::get_if<StringLiteralExpr>(&expression.payload)) {
                return value->text;
            }
            if (const auto* value = std::get_if<BoolLiteralExpr>(&expression.payload)) {
                return value->text;
            }
            if (const auto* value = std::get_if<UnaryExpr>(&expression.payload)) {
                return value->op + render_child(value->operand);
            }
            if (const auto* value = std::get_if<BinaryExpr>(&expression.payload)) {
                return render_child(value->left) + value->op + render_child(value->right);
            }
            if (const auto* value = std::get_if<CallExpr>(&expression.payload)) {
                std::string result = render_postfix_base(value->base) + "(";
                for (std::size_t i = 0; i < value->arguments.size(); ++i) {
                    if (i != 0) { result += ","; }
                    result += render_expression_full(value->arguments[i], expressions, depth + 1);
                }
                return result + ")";
            }
            if (const auto* value = std::get_if<IndexExpr>(&expression.payload)) {
                std::string result = render_postfix_base(value->base) + "[";
                for (std::size_t i = 0; i < value->indexes.size(); ++i) {
                    if (i != 0) { result += ","; }
                    result += render_expression_full(value->indexes[i], expressions, depth + 1);
                }
                return result + "]";
            }
            if (const auto* value = std::get_if<FieldAccessExpr>(&expression.payload)) {
                return render_postfix_base(value->base) + "." + value->field;
            }
            if (const auto* value = std::get_if<ListLiteralExpr>(&expression.payload)) {
                std::string result = "[";
                for (std::size_t i = 0; i < value->elements.size(); ++i) {
                    if (i != 0) { result += ","; }
                    result += render_expression_full(value->elements[i], expressions, depth + 1);
                }
                return result + "]";
            }
            if (const auto* value = std::get_if<RecordLiteralExpr>(&expression.payload)) {
                std::string result = "{";
                for (std::size_t i = 0; i < value->fields.size(); ++i) {
                    if (i != 0) { result += ","; }
                    result += value->fields[i].name + ":" + render_child(value->fields[i].value);
                }
                return result + "}";
            }

            return {};
        }

        void dump_id_array(std::ostream& out, const std::vector<std::size_t>& ids) {
            out << "[";
            for (std::size_t i = 0; i < ids.size(); ++i) {
                if (i != 0) { out << ", "; }
                out << ids[i];
            }
            out << "]";
        }

        void dump_optional_id(std::ostream& out, const std::optional<std::size_t>& id) {
            if (id) { out << *id; }
            else { out << "null"; }
        }

        void dump_source_location_json(std::ostream& out, const SourceLocation& location) {
            out << "{\"line\": " << location.line
                << ", \"column\": " << location.column << "}";
        }

        template<typename Clause>
        void dump_spelling_clause_json(std::ostream& out,
                                       const char* kind,
                                       const Clause& clause) {
            out << "{\"kind\": ";
            dump_json_string(out, kind);
            out << ", \"spelling\": ";
            dump_json_string(out, clause.spelling);
            out << ", \"location\": ";
            dump_source_location_json(out, clause.location);
            out << "}";
        }

        void dump_abi_type_property_json(std::ostream& out,
                                         const AbiTypeProperty& property) {
            std::visit([&out](const auto& clause) {
                using Clause = std::decay_t<decltype(clause)>;
                if constexpr (std::is_same_v<Clause, AbiReprClause>) {
                    dump_spelling_clause_json(out, "repr", clause);
                } else if constexpr (std::is_same_v<Clause, AbiOwnershipClause>) {
                    dump_spelling_clause_json(out, "ownership", clause);
                } else if constexpr (std::is_same_v<Clause, AbiAccessClause>) {
                    dump_spelling_clause_json(out, "access", clause);
                } else if constexpr (std::is_same_v<Clause, AbiLifetimeClause>) {
                    dump_spelling_clause_json(out, "lifetime", clause);
                } else if constexpr (std::is_same_v<Clause, AbiNullableClause>) {
                    dump_spelling_clause_json(out, "nullable", clause);
                } else if constexpr (std::is_same_v<Clause, AbiTerminatorClause>) {
                    dump_spelling_clause_json(out, "terminator", clause);
                } else if constexpr (std::is_same_v<Clause, AbiOpaqueClause>) {
                    dump_spelling_clause_json(out, "opaque", clause);
                } else if constexpr (std::is_same_v<Clause, AbiCleanupClause>) {
                    dump_spelling_clause_json(out, "cleanup", clause);
                }
            }, property);
        }

        void dump_extern_clause_json(std::ostream& out, const ExternClause& clause) {
            std::visit([&out](const auto& value) {
                using Clause = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<Clause, ExternSymbolClause>) {
                    dump_spelling_clause_json(out, "symbol", value);
                } else if constexpr (std::is_same_v<Clause, ExternEffectClause>) {
                    dump_spelling_clause_json(out, "effect", value);
                }
            }, clause);
        }

        void dump_parameter_json(std::ostream& out,
                                 const Parameter& parameter,
                                 const std::size_t indent) {
            dump_indent(out, indent);
            out << "{\"name\": ";
            dump_json_string(out, parameter.name);
            out << ", \"type\": ";
            dump_json_string(out, type_ref_text(parameter.type));
            out << ", \"type_ref\": ";
            dump_type_ref_json(out, parameter.type);
            out << ", \"location\": ";
            dump_source_location_json(out, parameter.location);
            out << "}";
        }

        void dump_record_field_json(std::ostream& out,
                                    const RecordField& field,
                                    const std::size_t indent) {
            dump_indent(out, indent);
            out << "{\"name\": ";
            dump_json_string(out, field.name);
            out << ", \"type\": ";
            dump_json_string(out, type_ref_text(field.type));
            out << ", \"type_ref\": ";
            dump_type_ref_json(out, field.type);
            out << ", \"location\": ";
            dump_source_location_json(out, field.location);
            out << "}";
        }

        void dump_abi_member_json(std::ostream& out,
                                  const AbiMember& member,
                                  const std::size_t indent) {
            std::visit([&out, indent](const auto& value) {
                using Member = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<Member, AbiLibraryClause>) {
                    dump_indent(out, indent);
                    dump_spelling_clause_json(out, "library", value);
                } else if constexpr (std::is_same_v<Member, AbiEvidenceClause>) {
                    dump_spelling_clause_json(out, "evidence", value);
                } else if constexpr (std::is_same_v<Member, AbiConventionClause>) {
                    dump_indent(out, indent);
                    dump_spelling_clause_json(out, "convention", value);
                } else if constexpr (std::is_same_v<Member, AbiTypeDecl>) {
                    dump_indent(out, indent);
                    out << "{\"kind\": \"type\", \"name\": ";
                    dump_json_string(out, value.name);
                    out << ", \"properties\": [";
                    for (std::size_t i = 0; i < value.properties.size(); ++i) {
                        if (i != 0) { out << ", "; }
                        dump_abi_type_property_json(out, value.properties[i]);
                    }
                    out << "], \"location\": ";
                    dump_source_location_json(out, value.location);
                    out << "}";
                } else if constexpr (std::is_same_v<Member, AbiStructDecl>) {
                    dump_indent(out, indent);
                    out << "{\"kind\": \"struct\", \"name\": ";
                    dump_json_string(out, value.name);
                    out << ", \"fields\": [";
                    if (!value.fields.empty()) { out << "\n"; }
                    for (std::size_t i = 0; i < value.fields.size(); ++i) {
                        dump_record_field_json(out, value.fields[i], indent + 2);
                        if (i + 1 < value.fields.size()) { out << ","; }
                        out << "\n";
                    }
                    if (!value.fields.empty()) { dump_indent(out, indent); }
                    out << "], \"location\": ";
                    dump_source_location_json(out, value.location);
                    out << "}";
                } else if constexpr (std::is_same_v<Member, ExternFunctionDecl>) {
                    dump_indent(out, indent);
                    out << "{\"kind\": \"extern_function\", \"name\": ";
                    dump_json_string(out, value.name);
                    out << ", \"parameters\": [";
                    if (!value.parameters.empty()) { out << "\n"; }
                    for (std::size_t i = 0; i < value.parameters.size(); ++i) {
                        dump_parameter_json(out, value.parameters[i], indent + 2);
                        if (i + 1 < value.parameters.size()) { out << ","; }
                        out << "\n";
                    }
                    if (!value.parameters.empty()) { dump_indent(out, indent); }
                    out << "], \"return_type\": ";
                    dump_json_string(out, type_ref_text(value.return_type));
                    out << ", \"return_type_ref\": ";
                    dump_type_ref_json(out, value.return_type);
                    out << ", \"clauses\": [";
                    for (std::size_t i = 0; i < value.clauses.size(); ++i) {
                        if (i != 0) { out << ", "; }
                        dump_extern_clause_json(out, value.clauses[i]);
                    }
                    out << "], \"location\": ";
                    dump_source_location_json(out, value.location);
                    out << "}";
                }
            }, member);
        }

        void dump_expression_payload_json(std::ostream& out,
                                          const Expression& expression,
                                          const unsigned indent) {
            out << "{";

            if (const auto* value = std::get_if<UnknownExpr>(&expression.payload)) {
                out << "\"text\": "; dump_json_string(out, value->text);
            } else if (const auto* value = std::get_if<IdentifierExpr>(&expression.payload)) {
                out << "\"name\": "; dump_json_string(out, value->name);
            } else if (const auto* value = std::get_if<IntegerLiteralExpr>(&expression.payload)) {
                out << "\"value_text\": "; dump_json_string(out, value->text);
            } else if (const auto* value = std::get_if<FloatLiteralExpr>(&expression.payload)) {
                out << "\"value_text\": "; dump_json_string(out, value->text);
            } else if (const auto* value = std::get_if<StringLiteralExpr>(&expression.payload)) {
                out << "\"value_text\": "; dump_json_string(out, value->text);
            } else if (const auto* value = std::get_if<BoolLiteralExpr>(&expression.payload)) {
                out << "\"value_text\": "; dump_json_string(out, value->text);
            } else if (const auto* value = std::get_if<UnaryExpr>(&expression.payload)) {
                out << "\"operator\": "; dump_json_string(out, value->op);
                out << ", \"operand\": "; dump_optional_id(out, value->operand);
            } else if (const auto* value = std::get_if<BinaryExpr>(&expression.payload)) {
                out << "\"operator\": "; dump_json_string(out, value->op);
                out << ", \"left\": "; dump_optional_id(out, value->left);
                out << ", \"right\": "; dump_optional_id(out, value->right);
            } else if (const auto* value = std::get_if<CallExpr>(&expression.payload)) {
                out << "\"base\": "; dump_optional_id(out, value->base);
                out << ", \"arguments\": "; dump_id_array(out, value->arguments);
            } else if (const auto* value = std::get_if<IndexExpr>(&expression.payload)) {
                out << "\"base\": "; dump_optional_id(out, value->base);
                out << ", \"indexes\": "; dump_id_array(out, value->indexes);
            } else if (const auto* value = std::get_if<FieldAccessExpr>(&expression.payload)) {
                out << "\"base\": "; dump_optional_id(out, value->base);
                out << ", \"field\": "; dump_json_string(out, value->field);
            } else if (const auto* value = std::get_if<ListLiteralExpr>(&expression.payload)) {
                out << "\"elements\": "; dump_id_array(out, value->elements);
            } else if (const auto* value = std::get_if<RecordLiteralExpr>(&expression.payload)) {
                out << "\"fields\": [";
                if (!value->fields.empty()) { out << "\n"; }
                for (std::size_t i = 0; i < value->fields.size(); ++i) {
                    const auto& field = value->fields[i];
                    dump_indent(out, indent + 2);
                    out << "{\"name\": "; dump_json_string(out, field.name);
                    out << ", \"value\": "; dump_optional_id(out, field.value);
                    out << ", \"location\": {\"line\": " << field.location.line
                        << ", \"column\": " << field.location.column << "}}";
                    if (i + 1 < value->fields.size()) { out << ","; }
                    out << "\n";
                }
                if (!value->fields.empty()) { dump_indent(out, indent); }
                out << "]";
            }

            out << "}";
        }


        void dump_assignable_target_json(std::ostream& out, const AssignableTarget& target) {
            std::visit([&out](const auto& value) {
                using Target = std::decay_t<decltype(value)>;
                out << "{";
                if constexpr (std::is_same_v<Target, IdentifierTarget>) {
                    out << "\"kind\": \"identifier\", \"name\": ";
                    dump_json_string(out, value.name);
                    out << ", \"location\": {\"line\": " << value.location.line
                        << ", \"column\": " << value.location.column << "}";
                } else if constexpr (std::is_same_v<Target, FieldPathTarget>) {
                    out << "\"kind\": \"field_path\", \"base_identifier\": ";
                    dump_json_string(out, value.base_identifier);
                    out << ", \"location\": {\"line\": " << value.location.line
                        << ", \"column\": " << value.location.column << "}, \"fields\": [";
                    for (std::size_t index = 0; index < value.fields.size(); ++index) {
                        if (index > 0) { out << ", "; }
                        out << "{\"name\": ";
                        dump_json_string(out, value.fields[index].name);
                        out << ", \"location\": {\"line\": " << value.fields[index].location.line
                            << ", \"column\": " << value.fields[index].location.column << "}}";
                    }
                    out << "]";
                } else {
                    out << "\"kind\": \"indexed\", \"base_identifier\": ";
                    dump_json_string(out, value.base_identifier);
                    out << ", \"location\": {\"line\": " << value.location.line
                        << ", \"column\": " << value.location.column << "}, \"indexes\": ";
                    dump_id_array(out, value.indexes);
                }
                out << "}";
            }, target);
        }

        void dump_statement_payload_json(std::ostream& out, const Statement& statement) {
            std::visit([&out](const auto& payload) {
                using Payload = std::decay_t<decltype(payload)>;
                out << "{";
                if constexpr (std::is_same_v<Payload, LetStatement>) {
                    out << "\"name\": ";
                    dump_json_string(out, payload.name);
                    out << ", \"type_ref\": ";
                    dump_type_ref_json(out, payload.type);
                    out << ", \"initializer_expression\": ";
                    dump_optional_id(out, payload.initializer_expression);
                } else if constexpr (std::is_same_v<Payload, AssignmentStatement>) {
                    out << "\"target\": ";
                    dump_assignable_target_json(out, payload.target);
                    out << ", \"value_expression\": ";
                    out << payload.value_expression;
                    out << ", \"source_form\": ";
                    dump_json_string(out, to_string(payload.source_form));
                } else if constexpr (std::is_same_v<Payload, PlacementStatement>) {
                    out << "\"value_expression\": ";
                    out << payload.value_expression;
                    out << ", \"target\": ";
                    dump_assignable_target_json(out, payload.target);
                    out << ", \"source_form\": ";
                    dump_json_string(out, to_string(payload.source_form));
                } else if constexpr (std::is_same_v<Payload, IfStatement>) {
                    out << "\"condition_expression\": ";
                    out << payload.condition_expression;
                    out << ", \"then_block\": ";
                    out << payload.then_block;
                    out << ", \"else_arm\": ";
                    if (!payload.else_arm) {
                        out << "null";
                    } else if (const auto* arm = std::get_if<ElseBlock>(&*payload.else_arm)) {
                        out << "{\"kind\": \"else_block\", \"block\": " << arm->block << "}";
                    } else {
                        const auto& elseIfArm = std::get<ElseIf>(*payload.else_arm);
                        out << "{\"kind\": \"else_if\", \"if_statement\": "
                            << elseIfArm.if_statement << "}";
                    }
                } else if constexpr (std::is_same_v<Payload, WhileStatement>) {
                    out << "\"condition_expression\": ";
                    out << payload.condition_expression;
                    out << ", \"body_block\": ";
                    out << payload.body_block;
                } else if constexpr (std::is_same_v<Payload, ReturnStatement>) {
                    out << "\"value_expression\": ";
                    dump_optional_id(out, payload.value_expression);
                    out << ", \"source_form\": ";
                    dump_json_string(out, to_string(payload.source_form));
                } else if constexpr (std::is_same_v<Payload, ExpressionStatement>) {
                    out << "\"expression\": ";
                    out << payload.expression << ", \"print\": "
                        << (payload.print ? "true" : "false");
                } else if constexpr (std::is_same_v<Payload, FlowStatement>) {
                    out << "\"expressions\": ";
                    dump_id_array(out, payload.expressions);
                } else if constexpr (std::is_same_v<Payload, UnknownStatement>) {
                    out << "\"text\": ";
                    dump_json_string(out, payload.text);
                }
                out << "}";
            }, statement.payload);
        }

        void dump_statement_pool_json(std::ostream& out,
                                      const std::vector<Statement>& statements,
                                      const unsigned indent) {
            out << "[";

            if (!statements.empty()) {
                out << "\n";

                for (std::size_t i = 0; i < statements.size(); ++i) {
                    const auto& statement = statements[i];
                    const auto kind = statement_kind(statement);
                    const auto* let = std::get_if<LetStatement>(&statement.payload);
                    const auto* assignment = std::get_if<AssignmentStatement>(&statement.payload);
                    const auto* placement = std::get_if<PlacementStatement>(&statement.payload);
                    const auto* ifStatement = std::get_if<IfStatement>(&statement.payload);
                    const auto* whileStatement = std::get_if<WhileStatement>(&statement.payload);
                    const auto* returnStatement = std::get_if<ReturnStatement>(&statement.payload);
                    const auto* expressionStatement = std::get_if<ExpressionStatement>(&statement.payload);
                    const auto* flowStatement = std::get_if<FlowStatement>(&statement.payload);
                    std::vector<std::size_t> expressionIds;
                    if (let) {
                        if (let->initializer_expression) {
                            expressionIds.push_back(*let->initializer_expression);
                        }
                    } else if (assignment) {
                        expressionIds.push_back(assignment->value_expression);
                        if (const auto* target = std::get_if<IndexedTarget>(&assignment->target)) {
                            expressionIds.insert(expressionIds.end(),
                                                 target->indexes.begin(), target->indexes.end());
                        }
                    } else if (placement) {
                        expressionIds.push_back(placement->value_expression);
                        if (const auto* target = std::get_if<IndexedTarget>(&placement->target)) {
                            expressionIds.insert(expressionIds.end(),
                                                 target->indexes.begin(), target->indexes.end());
                        }
                    } else if (ifStatement) {
                        expressionIds.push_back(ifStatement->condition_expression);
                    } else if (whileStatement) {
                        expressionIds.push_back(whileStatement->condition_expression);
                    } else if (returnStatement) {
                        if (returnStatement->value_expression) {
                            expressionIds.push_back(*returnStatement->value_expression);
                        }
                    } else if (expressionStatement) {
                        expressionIds.push_back(expressionStatement->expression);
                    } else if (flowStatement) {
                        expressionIds = flowStatement->expressions;
                    }

                    dump_indent(out, indent + 2);
                    out << "{\n";

                    dump_indent(out, indent + 4);
                    out << "\"id\": " << i << ",\n";
                    dump_indent(out, indent + 4);
                    out << "\"kind\": ";
                    dump_json_string(out, to_string(kind));
                    out << ",\n";
                    dump_indent(out, indent + 4);
                    out << "\"payload\": ";
                    dump_statement_payload_json(out, statement);

                    const std::string* name = nullptr;
                    if (let && !let->name.empty()) {
                        name = &let->name;
                    } else if (assignment) {
                        if (const auto* target = std::get_if<IdentifierTarget>(&assignment->target)) {
                            name = &target->name;
                        }
                    }
                    if (name && !name->empty()) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"name\": ";
                        dump_json_string(out, *name);
                    }

                    if (let && type_ref_kind(let->type) != TypeRefKind::Unknown) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"type\": ";
                        dump_json_string(out, type_ref_text(let->type));
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"type_ref\": ";
                        dump_type_ref_json(out, let->type);
                    }

                    if (let && let->initializer_expression) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"has_initializer\": true";
                    }

                    if (assignment || placement ||
                        (returnStatement && returnStatement->value_expression)) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"has_value\": true";
                    }

                    if (ifStatement || whileStatement) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"has_condition\": true";
                    }

                    if (!expressionIds.empty()) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"expression_ids\": [";
                        for (std::size_t exprIndex = 0; exprIndex < expressionIds.size(); ++exprIndex) {
                            if (exprIndex > 0) {
                                out << ", ";
                            }
                            out << expressionIds[exprIndex];
                        }
                        out << "]";
                    }

                    if (ifStatement) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"condition\": " << ifStatement->condition_expression;
                    }

                    if (ifStatement) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"then_block\": " << ifStatement->then_block;
                    } else if (whileStatement) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"body_block\": " << whileStatement->body_block;
                    }

                    if (ifStatement) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"else_arm\": ";
                        if (!ifStatement->else_arm) {
                            out << "null";
                        } else if (const auto* arm = std::get_if<ElseBlock>(&*ifStatement->else_arm)) {
                            out << "{\"kind\": \"else_block\", \"block\": " << arm->block << "}";
                        } else {
                            const auto& elseIfArm = std::get<ElseIf>(*ifStatement->else_arm);
                            out << "{\"kind\": \"else_if\", \"if_statement\": "
                                << elseIfArm.if_statement << "}";
                        }
                    }

                    out << ",\n";
                    dump_indent(out, indent + 4);
                    out << "\"location\": ";
                    dump_source_location_json(out, statement.location);

                    out << "\n";

                    dump_indent(out, indent + 2);
                    out << "}";

                    if (i + 1 < statements.size()) {
                        out << ",";
                    }

                    out << "\n";
                }

                dump_indent(out, indent);
            }

            out << "]";
        }

        void dump_block_pool_json(std::ostream& out,
                                  const std::vector<Block>& blocks,
                                  const unsigned indent) {
            out << "[";
            if (!blocks.empty()) { out << "\n"; }
            for (std::size_t i = 0; i < blocks.size(); ++i) {
                dump_indent(out, indent + 2);
                out << "{\"id\": " << i << ", \"statements\": ";
                dump_id_array(out, blocks[i].statements);
                out << "}";
                if (i + 1 < blocks.size()) { out << ","; }
                out << "\n";
            }
            if (!blocks.empty()) { dump_indent(out, indent); }
            out << "]";
        }


        void dump_expression_pool_json(std::ostream& out,
                                       const std::vector<Expression>& expressions,
                                       const unsigned indent) {
            out << "[";

            if (!expressions.empty()) {
                out << "\n";

                for (std::size_t i = 0; i < expressions.size(); ++i) {
                    const auto& expression = expressions[i];

                    dump_indent(out, indent + 2);
                    out << "{\n";

                    dump_indent(out, indent + 4);
                    out << "\"id\": " << i << ",\n";

                    dump_indent(out, indent + 4);
                    out << "\"kind\": ";
                    dump_json_string(out, to_string(expression_kind(expression)));

                    const auto text = expression_text(expression, expressions);
                    if (!text.empty()) {
                        out << ",\n";
                        dump_indent(out, indent + 4);
                        out << "\"text\": ";
                        dump_json_string(out, text);
                    }

                    out << ",\n";
                    dump_indent(out, indent + 4);
                    out << "\"payload\": ";
                    dump_expression_payload_json(out, expression, indent + 4);

                    out << ",\n";
                    dump_indent(out, indent + 4);
                    out << "\"child_expressions\": ";
                    dump_id_array(out, expression_children(expression));
                    out << "\n";

                    dump_indent(out, indent + 2);
                    out << "}";

                    if (i + 1 < expressions.size()) {
                        out << ",";
                    }

                    out << "\n";
                }

                dump_indent(out, indent);
            }

            out << "]";
        }

        void dump_top_level_decl_json(std::ostream& out,
                                      const TopLevelDecl& decl,
                                      const DeclarationId id,
                                      const std::size_t indent) {
            dump_indent(out, indent);
            out << "{\n";

            const TopLevelKind kind = top_level_kind(decl);

            dump_indent(out, indent + 2);
            out << "\"id\": " << id << ",\n";

            dump_indent(out, indent + 2);
            out << "\"kind\": \"" << to_string(kind) << "\"";

            if (const auto* importDecl = std::get_if<ImportDecl>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"module_name\": ";
                dump_json_string(out, importDecl->module_name);
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"alias\": ";
                dump_json_string(out, importDecl->alias);
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, importDecl->location);
                out << "\n";
            } else if (const auto* functionDecl = std::get_if<FunctionDecl>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"name\": ";
                dump_json_string(out, functionDecl->name);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"parameter_count\": " << functionDecl->parameters.size() << ",\n";

                dump_indent(out, indent + 2);
                out << "\"parameters\": [";

                if (!functionDecl->parameters.empty()) {
                    out << "\n";

                    for (std::size_t i = 0; i < functionDecl->parameters.size(); ++i) {
                        const auto& parameter = functionDecl->parameters[i];

                        dump_indent(out, indent + 4);
                        out << "{\n";

                        dump_indent(out, indent + 6);
                        out << "\"name\": ";
                        dump_json_string(out, parameter.name);
                        out << ",\n";

                        dump_indent(out, indent + 6);
                        out << "\"type\": ";
                        dump_json_string(out, type_ref_text(parameter.type));
                        out << ",\n";

                        dump_indent(out, indent + 6);
                        out << "\"type_ref\": ";
                        dump_type_ref_json(out, parameter.type);
                        out << ",\n";

                        dump_indent(out, indent + 6);
                        out << "\"location\": ";
                        dump_source_location_json(out, parameter.location);
                        out << "\n";

                        dump_indent(out, indent + 4);
                        out << "}";

                        if (i + 1 < functionDecl->parameters.size()) {
                            out << ",";
                        }

                        out << "\n";
                    }

                    dump_indent(out, indent + 2);
                }

                out << "],\n";

                dump_indent(out, indent + 2);
                out << "\"return_type\": ";
                dump_json_string(out, type_ref_text(functionDecl->return_type));
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"return_type_ref\": ";
                dump_type_ref_json(out, functionDecl->return_type);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"has_body\": " << (functionDecl->has_body ? "true" : "false") << ",\n";

                dump_indent(out, indent + 2);
                out << "\"body_block\": ";
                dump_optional_id(out, functionDecl->body);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, functionDecl->location);
                out << "\n";
            } else if (const auto* mainBlock = std::get_if<MainBlock>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"parameters\": [";
                if (!mainBlock->parameters.empty()) { out << "\n"; }
                for (std::size_t i = 0; i < mainBlock->parameters.size(); ++i) {
                    dump_parameter_json(out, mainBlock->parameters[i], indent + 2);
                    if (i + 1 < mainBlock->parameters.size()) { out << ","; }
                }
                if (!mainBlock->parameters.empty()) { dump_indent(out, indent + 2); }
                out << "],\n";
                dump_indent(out, indent + 2);
                out << "\"has_body\": " << (mainBlock->has_body ? "true" : "false") << ",\n";

                dump_indent(out, indent + 2);
                out << "\"body_block\": ";
                dump_optional_id(out, mainBlock->body);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, mainBlock->location);
                out << "\n";
            } else if (const auto* targetDecl = std::get_if<TargetDecl>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"name\": ";
                dump_json_string(out, targetDecl->name);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"declaration_ids\": [";
                for (std::size_t i = 0; i < targetDecl->declarations.size(); ++i) {
                    if (i != 0) { out << ", "; }
                    out << targetDecl->declarations[i];
                }
                out << "],\n";

                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, targetDecl->location);
                out << "\n";
            } else if (const auto* recordDecl = std::get_if<RecordDecl>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"name\": ";
                dump_json_string(out, recordDecl->name);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"field_count\": " << recordDecl->fields.size() << ",\n";

                dump_indent(out, indent + 2);
                out << "\"fields\": [";

                if (!recordDecl->fields.empty()) {
                    out << "\n";

                    for (std::size_t i = 0; i < recordDecl->fields.size(); ++i) {
                        const auto& field = recordDecl->fields[i];

                        dump_indent(out, indent + 4);
                        out << "{\n";

                        dump_indent(out, indent + 6);
                        out << "\"name\": ";
                        dump_json_string(out, field.name);
                        out << ",\n";

                        dump_indent(out, indent + 6);
                        out << "\"type\": ";
                        dump_json_string(out, type_ref_text(field.type));
                        out << ",\n";

                        dump_indent(out, indent + 6);
                        out << "\"type_ref\": ";
                        dump_type_ref_json(out, field.type);
                        out << ",\n";

                        dump_indent(out, indent + 6);
                        out << "\"location\": ";
                        dump_source_location_json(out, field.location);
                        out << "\n";

                        dump_indent(out, indent + 4);
                        out << "}";

                        if (i + 1 < recordDecl->fields.size()) {
                            out << ",";
                        }

                        out << "\n";
                    }

                    dump_indent(out, indent + 2);
                }

                out << "],\n";

                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, recordDecl->location);
                out << "\n";
            } else if (const auto* refinedTypeDecl = std::get_if<RefinedTypeDecl>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"name\": ";
                dump_json_string(out, refinedTypeDecl->name);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"base_type\": ";
                dump_json_string(out, type_ref_text(refinedTypeDecl->base_type));
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"base_type_ref\": ";
                dump_type_ref_json(out, refinedTypeDecl->base_type);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"invariants\": [";
                for (std::size_t i = 0; i < refinedTypeDecl->invariants.size(); ++i) {
                    if (i != 0) { out << ", "; }
                    out << "{\"condition_expression\": "
                        << refinedTypeDecl->invariants[i].condition_expression
                        << ", \"location\": ";
                    dump_source_location_json(out, refinedTypeDecl->invariants[i].location);
                    out << "}";
                }
                out << "],\n";
                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, refinedTypeDecl->location);
                out << "\n";
            } else if (const auto* abiDecl = std::get_if<AbiDecl>(&decl)) {
                out << ",\n";
                dump_indent(out, indent + 2);
                out << "\"name\": ";
                dump_json_string(out, abiDecl->name);
                out << ",\n";

                dump_indent(out, indent + 2);
                out << "\"members\": [";
                if (!abiDecl->members.empty()) { out << "\n"; }
                for (std::size_t i = 0; i < abiDecl->members.size(); ++i) {
                    dump_abi_member_json(out, abiDecl->members[i], indent + 4);
                    if (i + 1 < abiDecl->members.size()) { out << ","; }
                    out << "\n";
                }
                if (!abiDecl->members.empty()) { dump_indent(out, indent + 2); }
                out << "],\n";
                dump_indent(out, indent + 2);
                out << "\"location\": ";
                dump_source_location_json(out, abiDecl->location);
                out << "\n";
            } else {
                out << "\n";
            }

            dump_indent(out, indent);
            out << "}";
        }

        void dump_declaration_pool_json(std::ostream& out,
                                        const std::vector<TopLevelDecl>& declarations,
                                        const std::size_t indent) {
            out << "[";
            if (!declarations.empty()) { out << "\n"; }
            for (DeclarationId id = 0; id < declarations.size(); ++id) {
                dump_top_level_decl_json(out, declarations[id], id, indent + 2);
                if (id + 1 < declarations.size()) { out << ","; }
                out << "\n";
            }
            if (!declarations.empty()) { dump_indent(out, indent); }
            out << "]";
        }
    }// end namespace for helpers

    const char* to_string(SourceUnitKind kind) {
        switch (kind) {
            case SourceUnitKind::Program:   return "program";
            case SourceUnitKind::Unit:      return "unit";
            case SourceUnitKind::Unknown:   return "unknown";
        }
        return "unknown";
    }

    const char* to_string(TopLevelKind kind) {
        switch (kind) {
            case TopLevelKind::Import:      return "import";
            case TopLevelKind::Function:    return "function";
            case TopLevelKind::Record:      return "record";
            case TopLevelKind::RefinedType: return "refined_type";
            case TopLevelKind::Abi:         return "abi";
            case TopLevelKind::Target:      return "target";
            case TopLevelKind::MainBlock:   return "main_block";
            case TopLevelKind::Unknown:     return "unknown";
        }
        return "unknown";
    }

    const char* to_string(StatementKind kind) {
        switch (kind) {
            case StatementKind::Let:        return "let";
            case StatementKind::Assignment: return "assignment";
            case StatementKind::Placement:  return "placement";
            case StatementKind::If:         return "if";
            case StatementKind::While:      return "while";
            case StatementKind::Break:      return "break";
            case StatementKind::Continue:   return "continue";
            case StatementKind::Return:     return "return";
            case StatementKind::Expression: return "expression";
            case StatementKind::Flow:       return "flow";
            case StatementKind::Unknown:    return "unknown";
        }
        return "unknown";
    }

    const char* to_string(StatementSourceForm form) {
        switch (form) {
            case StatementSourceForm::EqualsAssignment: return "equals_assignment";
            case StatementSourceForm::ArrowPlacement:   return "arrow_placement";
            case StatementSourceForm::KeywordReturn:    return "keyword_return";
        }
        return "unknown";
    }

    StatementKind statement_kind(const Statement& statement) {
        return std::visit([](const auto& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, LetStatement>) {
                return StatementKind::Let;
            } else if constexpr (std::is_same_v<Payload, AssignmentStatement>) {
                return StatementKind::Assignment;
            } else if constexpr (std::is_same_v<Payload, PlacementStatement>) {
                return StatementKind::Placement;
            } else if constexpr (std::is_same_v<Payload, IfStatement>) {
                return StatementKind::If;
            } else if constexpr (std::is_same_v<Payload, WhileStatement>) {
                return StatementKind::While;
            } else if constexpr (std::is_same_v<Payload, BreakStatement>) {
                return StatementKind::Break;
            } else if constexpr (std::is_same_v<Payload, ContinueStatement>) {
                return StatementKind::Continue;
            } else if constexpr (std::is_same_v<Payload, ReturnStatement>) {
                return StatementKind::Return;
            } else if constexpr (std::is_same_v<Payload, ExpressionStatement>) {
                return StatementKind::Expression;
            } else if constexpr (std::is_same_v<Payload, FlowStatement>) {
                return StatementKind::Flow;
            } else {
                return StatementKind::Unknown;
            }
        }, statement.payload);
    }

    const char* to_string(ExpressionKind kind) {
        switch (kind) {
            case ExpressionKind::Identifier:        return "identifier";
            case ExpressionKind::IntegerLiteral:    return "integer_literal";
            case ExpressionKind::FloatLiteral:      return "float_literal";
            case ExpressionKind::StringLiteral:     return "string_literal";
            case ExpressionKind::BoolLiteral:       return "bool_literal";
            case ExpressionKind::Call:              return "call";
            case ExpressionKind::Unary:             return "unary";
            case ExpressionKind::Binary:            return "binary";
            case ExpressionKind::Index:             return "index";
            case ExpressionKind::FieldAccess:       return "field_access";
            case ExpressionKind::ListLiteral:       return "list_literal";
            case ExpressionKind::RecordLiteral:     return "record_literal";
            case ExpressionKind::Unknown:           return "unknown";
        }
        return "unknown";
    }

    const char* to_string(TypeRefKind kind) {
        switch (kind) {
            case TypeRefKind::Named:   return "named";
            case TypeRefKind::Generic: return "generic";
            case TypeRefKind::Array:   return "array";
            case TypeRefKind::Unknown: return "unknown";
        }
        return "unknown";
    }

    TypeRefKind type_ref_kind(const TypeRef& type) {
        if (std::holds_alternative<NamedTypeRef>(type.payload))   { return TypeRefKind::Named; }
        if (std::holds_alternative<GenericTypeRef>(type.payload)) { return TypeRefKind::Generic; }
        if (std::holds_alternative<ArrayTypeRef>(type.payload))   { return TypeRefKind::Array; }
        return TypeRefKind::Unknown;
    }

    std::string type_ref_text(const TypeRef& type) {
        return render_type_ref(type);
    }

    ExpressionKind expression_kind(const Expression& expression) {
        if (std::holds_alternative<IdentifierExpr>(expression.payload))     { return ExpressionKind::Identifier; }
        if (std::holds_alternative<IntegerLiteralExpr>(expression.payload)) { return ExpressionKind::IntegerLiteral; }
        if (std::holds_alternative<FloatLiteralExpr>(expression.payload))   { return ExpressionKind::FloatLiteral; }
        if (std::holds_alternative<StringLiteralExpr>(expression.payload))  { return ExpressionKind::StringLiteral; }
        if (std::holds_alternative<BoolLiteralExpr>(expression.payload))    { return ExpressionKind::BoolLiteral; }
        if (std::holds_alternative<CallExpr>(expression.payload))           { return ExpressionKind::Call; }
        if (std::holds_alternative<UnaryExpr>(expression.payload))          { return ExpressionKind::Unary; }
        if (std::holds_alternative<BinaryExpr>(expression.payload))         { return ExpressionKind::Binary; }
        if (std::holds_alternative<IndexExpr>(expression.payload))          { return ExpressionKind::Index; }
        if (std::holds_alternative<FieldAccessExpr>(expression.payload))    { return ExpressionKind::FieldAccess; }
        if (std::holds_alternative<ListLiteralExpr>(expression.payload))    { return ExpressionKind::ListLiteral; }
        if (std::holds_alternative<RecordLiteralExpr>(expression.payload))  { return ExpressionKind::RecordLiteral; }
        return ExpressionKind::Unknown;
    }

    std::string expression_text(const Expression& expression,
                                const std::vector<Expression>& expression_pool) {
        const auto render_postfix_base = [&](const std::optional<std::size_t>& child) {
            if (!child || *child >= expression_pool.size()) {
                return std::string{};
            }
            auto rendered = render_expression_full(*child, expression_pool);
            const auto& payload = expression_pool[*child].payload;
            if (std::holds_alternative<BinaryExpr>(payload) ||
                std::holds_alternative<UnaryExpr>(payload)) {
                return "(" + rendered + ")";
            }
            return rendered;
        };

        if (const auto* value = std::get_if<UnknownExpr>(&expression.payload)) {
            return value->text;
        }
        if (const auto* value = std::get_if<IdentifierExpr>(&expression.payload)) {
            return value->name;
        }
        if (const auto* value = std::get_if<IntegerLiteralExpr>(&expression.payload)) {
            return value->text;
        }
        if (const auto* value = std::get_if<FloatLiteralExpr>(&expression.payload)) {
            return value->text;
        }
        if (const auto* value = std::get_if<StringLiteralExpr>(&expression.payload)) {
            return value->text;
        }
        if (const auto* value = std::get_if<BoolLiteralExpr>(&expression.payload)) {
            return value->text;
        }
        if (const auto* value = std::get_if<UnaryExpr>(&expression.payload)) {
            return value->op;
        }
        if (const auto* value = std::get_if<BinaryExpr>(&expression.payload)) {
            return value->op;
        }
        if (const auto* value = std::get_if<CallExpr>(&expression.payload)) {
            return render_postfix_base(value->base);
        }
        if (const auto* value = std::get_if<IndexExpr>(&expression.payload)) {
            return render_postfix_base(value->base);
        }
        if (const auto* value = std::get_if<FieldAccessExpr>(&expression.payload)) {
            const auto base = render_postfix_base(value->base);
            return base.empty() ? value->field : base + "." + value->field;
        }
        if (std::holds_alternative<ListLiteralExpr>(expression.payload)) {
            return "[";
        }
        if (std::holds_alternative<RecordLiteralExpr>(expression.payload)) {
            return "{";
        }
        return {};
    }

    std::string expression_tree_text(const std::size_t expression_id,
                                     const std::vector<Expression>& expression_pool) {
        return render_expression_full(expression_id, expression_pool);
    }

    std::vector<std::size_t> expression_children(const Expression& expression) {
        std::vector<std::size_t> result;
        const auto append_optional = [&](const std::optional<std::size_t>& child) {
            if (child) { result.push_back(*child); }
        };

        if (const auto* value = std::get_if<UnaryExpr>(&expression.payload)) {
            append_optional(value->operand);
        } else if (const auto* value = std::get_if<BinaryExpr>(&expression.payload)) {
            append_optional(value->left);
            append_optional(value->right);
        } else if (const auto* value = std::get_if<CallExpr>(&expression.payload)) {
            append_optional(value->base);
            result.insert(result.end(), value->arguments.begin(), value->arguments.end());
        } else if (const auto* value = std::get_if<IndexExpr>(&expression.payload)) {
            append_optional(value->base);
            result.insert(result.end(), value->indexes.begin(), value->indexes.end());
        } else if (const auto* value = std::get_if<FieldAccessExpr>(&expression.payload)) {
            append_optional(value->base);
        } else if (const auto* value = std::get_if<ListLiteralExpr>(&expression.payload)) {
            result = value->elements;
        } else if (const auto* value = std::get_if<RecordLiteralExpr>(&expression.payload)) {
            for (const auto& field : value->fields) {
                append_optional(field.value);
            }
        }

        return result;
    }

    TopLevelKind top_level_kind(const TopLevelDecl& decl) {
        if (std::holds_alternative<ImportDecl>(decl))       { return TopLevelKind::Import;}
        if (std::holds_alternative<FunctionDecl>(decl))     { return TopLevelKind::Function;}
        if (std::holds_alternative<RecordDecl>(decl))       { return TopLevelKind::Record;}
        if (std::holds_alternative<RefinedTypeDecl>(decl))  { return TopLevelKind::RefinedType;}
        if (std::holds_alternative<AbiDecl>(decl))          { return TopLevelKind::Abi;}
        if (std::holds_alternative<TargetDecl>(decl))       { return TopLevelKind::Target;}
        if (std::holds_alternative<MainBlock>(decl))        { return TopLevelKind::MainBlock;}
        return TopLevelKind::Unknown;
    }

    AstModule make_empty_ast_module() {
        AstModule module;
        module.source_unit.kind     = SourceUnitKind::Unknown;
        module.source_unit.name     = "";
        module.source_unit.location = SourceLocation{};
        return module;
    }

    void dump_ast_json(std::ostream& out, const AstModule& module) {
        out << "{\n";
        out << "  \"format\": \"flowmini.ast.v2\",\n";
        out << "  \"source_unit\": {\n";
        out << "    \"kind\": \"" << to_string(module.source_unit.kind) << "\",\n";
        out << "    \"name\": "; dump_json_string(out, module.source_unit.name);out << ",\n";
        out << "    \"location\": ";
        dump_source_location_json(out, module.source_unit.location);
        out << ",\n";

        out << "    \"declaration_count\": " << module.source_unit.declarations.size() << ",\n";
        out << "    \"declaration_ids\": ";
        dump_id_array(out, module.source_unit.declarations);
        out << ",\n";
        out << "    \"declarations\": [\n";

        for (std::size_t i = 0; i < module.source_unit.declarations.size(); ++i) {
            const auto id = module.source_unit.declarations[i];
            if (id < module.declaration_pool.size()) {
                dump_top_level_decl_json(out, module.declaration_pool[id], id, 6);
            } else {
                dump_indent(out, 6);
                out << "null";
            }

            if (i + 1 < module.source_unit.declarations.size()) {
                out << ",";
            }

            out << "\n";
        }
        out << "    ]\n";

        out << "  },\n";
        out << "  \"declaration_pool_size\": " << module.declaration_pool.size() << ",\n";
        out << "  \"declaration_pool\": ";
        dump_declaration_pool_json(out, module.declaration_pool, 2);
        out << ",\n";
        out << "  \"expression_pool_size\": " << module.expression_pool.size() << ",\n";
    out << "  \"expression_pool\": ";
    dump_expression_pool_json(out, module.expression_pool, 2);
    out << ",\n";
    out << "  \"statement_pool_size\": " << module.statement_pool.size() << ",\n";
    out << "  \"statement_pool\": ";
    dump_statement_pool_json(out, module.statement_pool, 2);
    out << ",\n";
    out << "  \"block_pool_size\": " << module.block_pool.size() << ",\n";
    out << "  \"block_pool\": ";
    dump_block_pool_json(out, module.block_pool, 2);
    out << "\n";
    out << "}\n";
    }

} // namespace flowmini::ast

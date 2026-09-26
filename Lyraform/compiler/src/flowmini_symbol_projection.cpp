#include "flowmini_symbol_projection.h"

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace flowmini::ast {

const char* to_string(const AstOriginEntityKind kind) {
    switch (kind) {
        case AstOriginEntityKind::SourceUnit:  return "source_unit";
        case AstOriginEntityKind::Declaration: return "declaration";
        case AstOriginEntityKind::Statement:   return "statement";
        case AstOriginEntityKind::Block:       return "block";
        case AstOriginEntityKind::Parameter:   return "parameter";
        case AstOriginEntityKind::Field:       return "field";
        case AstOriginEntityKind::AbiMember:   return "abi_member";
    }
    return "source_unit";
}

const char* to_string(const AstOriginRole role) {
    switch (role) {
        case AstOriginRole::SourceUnit:          return "source_unit";
        case AstOriginRole::ImportDeclaration:   return "import_declaration";
        case AstOriginRole::FunctionDeclaration: return "function_declaration";
        case AstOriginRole::FunctionParameter:   return "function_parameter";
        case AstOriginRole::RecordDeclaration:   return "record_declaration";
        case AstOriginRole::RecordField:         return "record_field";
        case AstOriginRole::RefinedTypeDeclaration: return "refined_type_declaration";
        case AstOriginRole::AbiDeclaration:      return "abi_declaration";
        case AstOriginRole::AbiType:             return "abi_type";
        case AstOriginRole::AbiStruct:           return "abi_struct";
        case AstOriginRole::AbiStructField:      return "abi_struct_field";
        case AstOriginRole::ExternFunction:      return "extern_function";
        case AstOriginRole::ExternParameter:     return "extern_parameter";
        case AstOriginRole::MainDeclaration:     return "main_declaration";
        case AstOriginRole::TargetDeclaration:   return "target_declaration";
        case AstOriginRole::LocalBinding:        return "local_binding";
        case AstOriginRole::LocalGuard:          return "local_guard";
        case AstOriginRole::ModuleScope:         return "module_scope";
        case AstOriginRole::FunctionScope:       return "function_scope";
        case AstOriginRole::RecordScope:         return "record_scope";
        case AstOriginRole::AbiScope:            return "abi_scope";
        case AstOriginRole::AbiStructScope:      return "abi_struct_scope";
        case AstOriginRole::ExternFunctionScope: return "extern_function_scope";
        case AstOriginRole::MainScope:           return "main_scope";
        case AstOriginRole::TargetScope:         return "target_scope";
        case AstOriginRole::IfThenScope:         return "if_then_scope";
        case AstOriginRole::WhileBodyScope:      return "while_body_scope";
        case AstOriginRole::ElseBlockScope:      return "else_block_scope";
    }
    return "source_unit";
}

namespace {

void record_symbol_origin(std::vector<SymbolAstOrigin>& origins,
                          const symboltable::SymbolId symbol,
                          std::string astPath,
                          AstOriginDescriptor descriptor) {
    origins.push_back(SymbolAstOrigin{symbol, std::move(astPath), std::move(descriptor)});
}

void record_scope_origin(std::vector<ScopeAstOrigin>& origins,
                         const symboltable::ScopeId scope,
                         std::string astPath,
                         AstOriginDescriptor descriptor) {
    origins.push_back(ScopeAstOrigin{scope, std::move(astPath), std::move(descriptor)});
}

AstOriginDescriptor make_origin(const AstOriginEntityKind entityKind,
                                const AstOriginRole role,
                                const SourceLocation location,
                                const std::optional<std::size_t> astId = std::nullopt) {
    return AstOriginDescriptor{entityKind, role, astId, location};
}

std::string declaration_ast_path(const DeclarationId declarationId) {
    return "/declaration_pool/" + std::to_string(declarationId);
}

std::string statement_ast_path(const StatementId statementId) {
    return "/statement_pool/" + std::to_string(statementId);
}

std::string block_ast_path(const BlockId blockId) {
    return "/block_pool/" + std::to_string(blockId);
}

symboltable::SourceLocation project_source_location(const SourceLocation& location) {
    return symboltable::SourceLocation{
        .file = {},
        .line = static_cast<std::uint32_t>(location.line),
        .column = static_cast<std::uint32_t>(location.column),
    };
}

void set_declaration_location(symboltable::SymbolTable& table,
                              const symboltable::SymbolId symbol,
                              const SourceLocation& location) {
    table.symbol(symbol).declarationLocation = project_source_location(location);
}

void add_string_fact(symboltable::SymbolTable& table,
                     const symboltable::SymbolId symbol,
                     const symboltable::FactoidKind kind,
                     std::string key,
                     std::string value) {
    table.addFact(symbol, symboltable::Factoid{
        .kind = kind,
        .key = std::move(key),
        .value = std::move(value),
    });
}

void add_type_spelling_fact(symboltable::SymbolTable& table,
                            const symboltable::SymbolId symbol,
                            std::string key,
                            const TypeRef& type) {
    auto spelling = type_ref_text(type);
    if (spelling.empty()) {
        return;
    }

    add_string_fact(table,
                    symbol,
                    symboltable::FactoidKind::TypeReference,
                    std::move(key),
                    std::move(spelling));
}

std::string source_unit_debug_name(const SourceUnit& unit) {
    if (!unit.name.empty()) {
        return unit.name;
    }

    switch (unit.kind) {
        case SourceUnitKind::Program: return "<anonymous-program>";
        case SourceUnitKind::Unit:    return "<anonymous-unit>";
        default:                      return "<anonymous-source-unit>";
    }
}

symboltable::SymbolKind source_unit_symbol_kind(const SourceUnitKind kind) {
    switch (kind) {
        case SourceUnitKind::Program:
        case SourceUnitKind::Unit:
            return symboltable::SymbolKind::Module;

        default:
            return symboltable::SymbolKind::Unknown;
    }
}

void project_import_decl(symboltable::SymbolTable& table,
                         const symboltable::ScopeId moduleScope,
                         const ImportDecl& decl,
                         std::vector<SymbolAstOrigin>& symbolOrigins,
                         const std::string& astPath,
                         const DeclarationId declarationId) {
    const auto name = !decl.alias.empty()
        ? std::string{"<import:"} + decl.alias + ">"
        : (decl.module_name.empty()
        ? std::string{"<anonymous-import>"}
        : decl.module_name);

    const auto importSymbol =
        table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Import);
    set_declaration_location(table, importSymbol, decl.location);
    record_symbol_origin(symbolOrigins,
                         importSymbol,
                         astPath,
                         make_origin(AstOriginEntityKind::Declaration,
                                     AstOriginRole::ImportDeclaration,
                                     decl.location,
                                     declarationId));
}


std::string statement_scope_debug_name(const Statement& statement) {
    switch (statement_kind(statement)) {
        case StatementKind::If:    return "if";
        case StatementKind::While: return "while";
        default:                   return "block";
    }
}

void project_block(symboltable::SymbolTable& table,
                   const AstModule& module,
                   symboltable::ScopeId owningScope,
                   BlockId blockId,
                   std::vector<SymbolAstOrigin>& symbolOrigins,
                   std::vector<ScopeAstOrigin>& scopeOrigins);

void project_statement_binding(symboltable::SymbolTable& table,
                               const AstModule& module,
                               const symboltable::ScopeId owningScope,
                               const StatementId statementId,
                               std::vector<SymbolAstOrigin>& symbolOrigins,
                               std::vector<ScopeAstOrigin>& scopeOrigins) {
    if (statementId >= module.statement_pool.size()) {
        return;
    }
    const auto& statement = module.statement_pool[statementId];

    if (const auto* binding = std::get_if<LetStatement>(&statement.payload);
        binding && !binding->name.empty()) {
        const auto variableSymbol =
            table.insertSymbol(owningScope,
                               binding->name,
                               symboltable::SymbolKind::Variable);
        set_declaration_location(table, variableSymbol, statement.location);
        record_symbol_origin(symbolOrigins,
                             variableSymbol,
                             statement_ast_path(statementId),
                             make_origin(AstOriginEntityKind::Statement,
                                         AstOriginRole::LocalBinding,
                                         statement.location,
                                         statementId));
        add_type_spelling_fact(table,
                               variableSymbol,
                               "declared_type_spelling",
                               binding->type);
    }
    if (const auto* guard = std::get_if<GuardActivateStatement>(&statement.payload);
        guard && !guard->name.empty()) {
        const auto guardSymbol = table.insertSymbol(owningScope, guard->name,
                                                     symboltable::SymbolKind::Contract);
        set_declaration_location(table, guardSymbol, statement.location);
        record_symbol_origin(symbolOrigins, guardSymbol, statement_ast_path(statementId),
            make_origin(AstOriginEntityKind::Statement, AstOriginRole::LocalGuard,
                        statement.location, statementId));
        add_string_fact(table, guardSymbol, symboltable::FactoidKind::Custom,
                        "contract_role", "guard");
    }

    std::optional<BlockId> body;
    const std::optional<ElseArm>* elseArm = nullptr;
    if (const auto* ifStatement = std::get_if<IfStatement>(&statement.payload)) {
        body = ifStatement->then_block;
        elseArm = &ifStatement->else_arm;
    } else if (const auto* whileStatement = std::get_if<WhileStatement>(&statement.payload)) {
        body = whileStatement->body_block;
    }

    if (body && *body < module.block_pool.size()) {
        const auto blockScope =
            table.createScope(symboltable::ScopeKind::Block,
                              owningScope,
                              std::nullopt,
                              statement_scope_debug_name(statement));

        const auto role = std::holds_alternative<IfStatement>(statement.payload)
            ? AstOriginRole::IfThenScope
            : AstOriginRole::WhileBodyScope;
        record_scope_origin(scopeOrigins,
                            blockScope,
                            block_ast_path(*body),
                            make_origin(AstOriginEntityKind::Block,
                                        role,
                                        statement.location,
                                        *body));

        project_block(table, module, blockScope, *body, symbolOrigins, scopeOrigins);
    }

    if (elseArm && *elseArm) {
        if (const auto* elseBlock = std::get_if<ElseBlock>(&**elseArm)) {
            if (elseBlock->block < module.block_pool.size()) {
                const auto elseScope =
                    table.createScope(symboltable::ScopeKind::Block,
                                      owningScope,
                                      std::nullopt,
                                      "else");
                record_scope_origin(scopeOrigins,
                                    elseScope,
                                    block_ast_path(elseBlock->block),
                                    make_origin(AstOriginEntityKind::Block,
                                                AstOriginRole::ElseBlockScope,
                                                statement.location,
                                                elseBlock->block));
                project_block(table,
                              module,
                              elseScope,
                              elseBlock->block,
                              symbolOrigins,
                              scopeOrigins);
            }
        } else if (const auto* elseIf = std::get_if<ElseIf>(&**elseArm)) {
            project_statement_binding(table,
                                      module,
                                      owningScope,
                                      elseIf->if_statement,
                                      symbolOrigins,
                                      scopeOrigins);
        }
    }
}

void project_block(symboltable::SymbolTable& table,
                   const AstModule& module,
                   const symboltable::ScopeId owningScope,
                   const BlockId blockId,
                   std::vector<SymbolAstOrigin>& symbolOrigins,
                   std::vector<ScopeAstOrigin>& scopeOrigins) {
    if (blockId >= module.block_pool.size()) {
        return;
    }
    for (const auto statementId : module.block_pool[blockId].statements) {
        project_statement_binding(table,
                                  module,
                                  owningScope,
                                  statementId,
                                  symbolOrigins,
                                  scopeOrigins);
    }
}


void project_function_decl(symboltable::SymbolTable& table,
                           const AstModule& module,
                           const symboltable::ScopeId moduleScope,
                           const FunctionDecl& decl,
                           std::vector<SymbolAstOrigin>& symbolOrigins,
                           std::vector<ScopeAstOrigin>& scopeOrigins,
                           const std::string& astPath,
                           const DeclarationId declarationId) {
    const auto name = decl.name.empty()
        ? std::string{"<anonymous-function>"}
        : decl.name;

    const auto fnSymbol =
        table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Function);
    set_declaration_location(table, fnSymbol, decl.location);
    record_symbol_origin(symbolOrigins,
                         fnSymbol,
                         astPath,
                         make_origin(AstOriginEntityKind::Declaration,
                                     AstOriginRole::FunctionDeclaration,
                                     decl.location,
                                     declarationId));
    add_type_spelling_fact(table,
                           fnSymbol,
                           "return_type_spelling",
                           decl.return_type);

    const auto fnScope =
        table.createScope(symboltable::ScopeKind::Function,
                          moduleScope,
                          fnSymbol,
                          name);
    record_scope_origin(scopeOrigins,
                        fnScope,
                        astPath,
                        make_origin(AstOriginEntityKind::Declaration,
                                    AstOriginRole::FunctionScope,
                                    decl.location,
                                    declarationId));

    for (std::size_t parameterIndex = 0; parameterIndex < decl.parameters.size(); ++parameterIndex) {
        const auto& parameter = decl.parameters[parameterIndex];
        if (parameter.name.empty()) {
            continue;
        }

        const auto parameterSymbol =
            table.insertSymbol(fnScope,
                               parameter.name,
                               symboltable::SymbolKind::Parameter);
        set_declaration_location(table, parameterSymbol, parameter.location);
        record_symbol_origin(symbolOrigins,
                             parameterSymbol,
                             astPath + "/parameters/" + std::to_string(parameterIndex),
                             make_origin(AstOriginEntityKind::Parameter,
                                         AstOriginRole::FunctionParameter,
                                         parameter.location));
        add_type_spelling_fact(table,
                               parameterSymbol,
                               "declared_type_spelling",
                               parameter.type);
    }

    if (decl.body) {
        project_block(table,
                      module,
                      fnScope,
                      *decl.body,
                      symbolOrigins,
                      scopeOrigins);
    }
}

void project_record_decl(symboltable::SymbolTable& table,
                         const symboltable::ScopeId moduleScope,
                         const RecordDecl& decl,
                         std::vector<SymbolAstOrigin>& symbolOrigins,
                         std::vector<ScopeAstOrigin>& scopeOrigins,
                         const std::string& astPath,
                         const DeclarationId declarationId) {
    const auto name = decl.name.empty()
        ? std::string{"<anonymous-record>"}
        : decl.name;

    const auto recordSymbol =
        table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Struct);
    set_declaration_location(table, recordSymbol, decl.location);
    record_symbol_origin(symbolOrigins,
                         recordSymbol,
                         astPath,
                         make_origin(AstOriginEntityKind::Declaration,
                                     AstOriginRole::RecordDeclaration,
                                     decl.location,
                                     declarationId));

    const auto recordScope =
        table.createScope(symboltable::ScopeKind::Struct,
                          moduleScope,
                          recordSymbol,
                          name);
    record_scope_origin(scopeOrigins,
                        recordScope,
                        astPath,
                        make_origin(AstOriginEntityKind::Declaration,
                                    AstOriginRole::RecordScope,
                                    decl.location,
                                    declarationId));

    for (std::size_t fieldIndex = 0; fieldIndex < decl.fields.size(); ++fieldIndex) {
        const auto& field = decl.fields[fieldIndex];
        if (field.name.empty()) {
            continue;
        }

        const auto fieldSymbol =
            table.insertSymbol(recordScope,
                               field.name,
                               symboltable::SymbolKind::Field);
        set_declaration_location(table, fieldSymbol, field.location);
        record_symbol_origin(symbolOrigins,
                             fieldSymbol,
                             astPath + "/fields/" + std::to_string(fieldIndex),
                             make_origin(AstOriginEntityKind::Field,
                                         AstOriginRole::RecordField,
                                         field.location));
        add_type_spelling_fact(table,
                               fieldSymbol,
                               "declared_type_spelling",
                               field.type);
    }
}

void project_refined_type_decl(symboltable::SymbolTable& table,
                               const AstModule& module,
                               const symboltable::ScopeId moduleScope,
                               const RefinedTypeDecl& decl,
                               std::vector<SymbolAstOrigin>& symbolOrigins,
                               const std::string& astPath,
                               const DeclarationId declarationId) {
    const auto name = decl.name.empty()
        ? std::string{"<anonymous-refined-type>"}
        : decl.name;

    const auto typeSymbol =
        table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Type);
    set_declaration_location(table, typeSymbol, decl.location);
    record_symbol_origin(symbolOrigins,
                         typeSymbol,
                         astPath,
                         make_origin(AstOriginEntityKind::Declaration,
                                     AstOriginRole::RefinedTypeDeclaration,
                                     decl.location,
                                     declarationId));
    add_type_spelling_fact(table,
                           typeSymbol,
                           "base_type_spelling",
                           decl.base_type);

    for (const auto& invariant : decl.invariants) {
        const auto spelling = expression_tree_text(invariant.condition_expression,
                                                   module.expression_pool);
        if (!spelling.empty()) {
            add_string_fact(table,
                            typeSymbol,
                            symboltable::FactoidKind::Custom,
                            "invariant_spelling",
                            spelling);
        }
    }
}

void project_abi_decl(symboltable::SymbolTable& table,
                      const symboltable::ScopeId moduleScope,
                      const AbiDecl& decl,
                      std::vector<SymbolAstOrigin>& symbolOrigins,
                      std::vector<ScopeAstOrigin>& scopeOrigins,
                      const std::string& astPath,
                      const DeclarationId declarationId) {
    const auto name = decl.name.empty()
        ? std::string{"<anonymous-abi>"}
        : decl.name;

    const auto abiSymbol =
        table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Contract);
    set_declaration_location(table, abiSymbol, decl.location);
    record_symbol_origin(symbolOrigins,
                         abiSymbol,
                         astPath,
                         make_origin(AstOriginEntityKind::Declaration,
                                     AstOriginRole::AbiDeclaration,
                                     decl.location,
                                     declarationId));

    const auto abiScope =
        table.createScope(symboltable::ScopeKind::Contract,
                          moduleScope,
                          abiSymbol,
                          name);
    record_scope_origin(scopeOrigins,
                        abiScope,
                        astPath,
                        make_origin(AstOriginEntityKind::Declaration,
                                    AstOriginRole::AbiScope,
                                    decl.location,
                                    declarationId));

    for (std::size_t memberIndex = 0; memberIndex < decl.members.size(); ++memberIndex) {
        const auto& member = decl.members[memberIndex];
        const auto memberPath = astPath + "/members/" + std::to_string(memberIndex);
        std::visit([&](const auto& value) {
            using Member = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Member, AbiLibraryClause>) {
                add_string_fact(table,
                                abiSymbol,
                                symboltable::FactoidKind::AbiTag,
                                "library_spelling",
                                value.spelling);
            } else if constexpr (std::is_same_v<Member, AbiEvidenceClause>) {
                add_string_fact(table, abiSymbol, symboltable::FactoidKind::AbiTag,
                                "evidence_spelling", value.spelling);
            } else if constexpr (std::is_same_v<Member, AbiConventionClause>) {
                add_string_fact(table,
                                abiSymbol,
                                symboltable::FactoidKind::AbiTag,
                                "convention_spelling",
                                value.spelling);
            } else if constexpr (std::is_same_v<Member, AbiTypeDecl>) {
                const auto typeName = value.name.empty()
                    ? std::string{"<anonymous-abi-type>"}
                    : value.name;
                const auto typeSymbol =
                    table.insertSymbol(abiScope, typeName, symboltable::SymbolKind::Type);
                set_declaration_location(table, typeSymbol, value.location);
                record_symbol_origin(symbolOrigins,
                                     typeSymbol,
                                     memberPath,
                                     make_origin(AstOriginEntityKind::AbiMember,
                                                 AstOriginRole::AbiType,
                                                 value.location));

                for (const auto& property : value.properties) {
                    std::visit([&](const auto& clause) {
                        using Clause = std::decay_t<decltype(clause)>;
                        const char* key = "";
                        if constexpr (std::is_same_v<Clause, AbiReprClause>) {
                            key = "repr_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiOwnershipClause>) {
                            key = "ownership_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiAccessClause>) {
                            key = "access_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiLifetimeClause>) {
                            key = "lifetime_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiNullableClause>) {
                            key = "nullable_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiTerminatorClause>) {
                            key = "terminator_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiOpaqueClause>) {
                            key = "opaque_spelling";
                        } else if constexpr (std::is_same_v<Clause, AbiCleanupClause>) {
                            key = "cleanup_spelling";
                        }
                        add_string_fact(table,
                                        typeSymbol,
                                        symboltable::FactoidKind::AbiTag,
                                        key,
                                        clause.spelling);
                    }, property);
                }
            } else if constexpr (std::is_same_v<Member, AbiStructDecl>) {
                const auto structName = value.name.empty()
                    ? std::string{"<anonymous-abi-struct>"}
                    : value.name;
                const auto structSymbol =
                    table.insertSymbol(abiScope, structName, symboltable::SymbolKind::Struct);
                set_declaration_location(table, structSymbol, value.location);
                record_symbol_origin(symbolOrigins,
                                     structSymbol,
                                     memberPath,
                                     make_origin(AstOriginEntityKind::AbiMember,
                                                 AstOriginRole::AbiStruct,
                                                 value.location));
                const auto structScope =
                    table.createScope(symboltable::ScopeKind::Struct,
                                      abiScope,
                                      structSymbol,
                                      structName);
                record_scope_origin(scopeOrigins,
                                    structScope,
                                    memberPath,
                                    make_origin(AstOriginEntityKind::AbiMember,
                                                AstOriginRole::AbiStructScope,
                                                value.location));
                for (std::size_t fieldIndex = 0; fieldIndex < value.fields.size(); ++fieldIndex) {
                    const auto& field = value.fields[fieldIndex];
                    if (field.name.empty()) {
                        continue;
                    }
                    const auto fieldSymbol =
                        table.insertSymbol(structScope,
                                           field.name,
                                           symboltable::SymbolKind::Field);
                    set_declaration_location(table, fieldSymbol, field.location);
                    record_symbol_origin(symbolOrigins,
                                         fieldSymbol,
                                         memberPath + "/fields/" + std::to_string(fieldIndex),
                                         make_origin(AstOriginEntityKind::Field,
                                                     AstOriginRole::AbiStructField,
                                                     field.location));
                    add_type_spelling_fact(table,
                                           fieldSymbol,
                                           "declared_type_spelling",
                                           field.type);
                }
            } else if constexpr (std::is_same_v<Member, ExternFunctionDecl>) {
                const auto functionName = value.name.empty()
                    ? std::string{"<anonymous-extern-function>"}
                    : value.name;
                const auto functionSymbol =
                    table.insertSymbol(abiScope,
                                       functionName,
                                       symboltable::SymbolKind::Function);
                set_declaration_location(table, functionSymbol, value.location);
                record_symbol_origin(symbolOrigins,
                                     functionSymbol,
                                     memberPath,
                                     make_origin(AstOriginEntityKind::AbiMember,
                                                 AstOriginRole::ExternFunction,
                                                 value.location));
                add_type_spelling_fact(table,
                                       functionSymbol,
                                       "return_type_spelling",
                                       value.return_type);
                for (const auto& clause : value.clauses) {
                    std::visit([&](const auto& item) {
                        using Clause = std::decay_t<decltype(item)>;
                        const char* key = std::is_same_v<Clause, ExternSymbolClause>
                            ? "external_symbol_spelling"
                            : "effect_spelling";
                        add_string_fact(table,
                                        functionSymbol,
                                        symboltable::FactoidKind::AbiTag,
                                        key,
                                        item.spelling);
                    }, clause);
                }

                const auto functionScope =
                    table.createScope(symboltable::ScopeKind::Function,
                                      abiScope,
                                      functionSymbol,
                                      functionName);
                record_scope_origin(scopeOrigins,
                                    functionScope,
                                    memberPath,
                                    make_origin(AstOriginEntityKind::AbiMember,
                                                AstOriginRole::ExternFunctionScope,
                                                value.location));
                for (std::size_t parameterIndex = 0;
                     parameterIndex < value.parameters.size();
                     ++parameterIndex) {
                    const auto& parameter = value.parameters[parameterIndex];
                    if (parameter.name.empty()) {
                        continue;
                    }
                    const auto parameterSymbol =
                        table.insertSymbol(functionScope,
                                           parameter.name,
                                           symboltable::SymbolKind::Parameter);
                    set_declaration_location(table, parameterSymbol, parameter.location);
                    record_symbol_origin(symbolOrigins,
                                         parameterSymbol,
                                         memberPath + "/parameters/" +
                                             std::to_string(parameterIndex),
                                         make_origin(AstOriginEntityKind::Parameter,
                                                     AstOriginRole::ExternParameter,
                                                     parameter.location));
                    add_type_spelling_fact(table,
                                           parameterSymbol,
                                           "declared_type_spelling",
                                           parameter.type);
                }
            }
        }, member);
    }
}

void project_main_block(symboltable::SymbolTable& table,
                        const AstModule& module,
                        const symboltable::ScopeId moduleScope,
                        const MainBlock& decl,
                        std::vector<SymbolAstOrigin>& symbolOrigins,
                        std::vector<ScopeAstOrigin>& scopeOrigins,
                        const std::string& astPath,
                        const DeclarationId declarationId) {
    constexpr auto name = "main";

    const auto mainSymbol =
        table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Procedure);
    set_declaration_location(table, mainSymbol, decl.location);
    record_symbol_origin(symbolOrigins,
                         mainSymbol,
                         astPath,
                         make_origin(AstOriginEntityKind::Declaration,
                                     AstOriginRole::MainDeclaration,
                                     decl.location,
                                     declarationId));

    const auto mainScope =
        table.createScope(symboltable::ScopeKind::Function,
                          moduleScope,
                          mainSymbol,
                          name);
    record_scope_origin(scopeOrigins,
                        mainScope,
                        astPath,
                        make_origin(AstOriginEntityKind::Declaration,
                                    AstOriginRole::MainScope,
                                    decl.location,
                                    declarationId));

    for (std::size_t parameterIndex = 0;
         parameterIndex < decl.parameters.size();
         ++parameterIndex) {
        const auto& parameter = decl.parameters[parameterIndex];
        if (parameter.name.empty()) {
            continue;
        }
        const auto parameterSymbol =
            table.insertSymbol(mainScope,
                               parameter.name,
                               symboltable::SymbolKind::Parameter);
        set_declaration_location(table, parameterSymbol, parameter.location);
        record_symbol_origin(symbolOrigins,
                             parameterSymbol,
                             astPath + "/parameters/" + std::to_string(parameterIndex),
                             make_origin(AstOriginEntityKind::Parameter,
                                         AstOriginRole::FunctionParameter,
                                         parameter.location));
        add_type_spelling_fact(table,
                               parameterSymbol,
                               "declared_type_spelling",
                               parameter.type);
    }

    if (decl.body) {
        project_block(table,
                      module,
                      mainScope,
                      *decl.body,
                      symbolOrigins,
                      scopeOrigins);
    }
}

struct ProjectTopLevelDecl {
    symboltable::SymbolTable& table;
    const AstModule& module;
    symboltable::ScopeId moduleScope;
    std::vector<SymbolAstOrigin>& symbolOrigins;
    std::vector<ScopeAstOrigin>& scopeOrigins;
    std::string astPath;
    DeclarationId declarationId;

    void operator()(const ImportDecl& decl) const {
        project_import_decl(table,
                            moduleScope,
                            decl,
                            symbolOrigins,
                            astPath,
                            declarationId);
    }

    void operator()(const FunctionDecl& decl) const {
        project_function_decl(table,
                              module,
                              moduleScope,
                              decl,
                              symbolOrigins,
                              scopeOrigins,
                              astPath,
                              declarationId);
    }

    void operator()(const RecordDecl& decl) const {
        project_record_decl(table,
                            moduleScope,
                            decl,
                            symbolOrigins,
                            scopeOrigins,
                            astPath,
                            declarationId);
    }

    void operator()(const RefinedTypeDecl& decl) const {
        project_refined_type_decl(table,
                                  module,
                                  moduleScope,
                                  decl,
                                  symbolOrigins,
                                  astPath,
                                  declarationId);
    }

    void operator()(const AbiDecl& decl) const {
        project_abi_decl(table,
                         moduleScope,
                         decl,
                         symbolOrigins,
                         scopeOrigins,
                         astPath,
                         declarationId);
    }

    void operator()(const MainBlock& decl) const {
        project_main_block(table,
                           module,
                           moduleScope,
                           decl,
                           symbolOrigins,
                           scopeOrigins,
                           astPath,
                           declarationId);
    }

    void operator()(const TargetDecl& decl) const {
        const auto name = decl.name.empty()
            ? std::string{"<anonymous-target>"}
            : decl.name;
        const auto targetSymbol =
            table.insertSymbol(moduleScope, name, symboltable::SymbolKind::Namespace);
        set_declaration_location(table, targetSymbol, decl.location);
        record_symbol_origin(symbolOrigins,
                             targetSymbol,
                             astPath,
                             make_origin(AstOriginEntityKind::Declaration,
                                         AstOriginRole::TargetDeclaration,
                                         decl.location,
                                         declarationId));

        const auto targetScope =
            table.createScope(symboltable::ScopeKind::Namespace,
                              moduleScope,
                              targetSymbol,
                              name);
        record_scope_origin(scopeOrigins,
                            targetScope,
                            astPath,
                            make_origin(AstOriginEntityKind::Declaration,
                                        AstOriginRole::TargetScope,
                                        decl.location,
                                        declarationId));

        for (const auto childId : decl.declarations) {
            if (childId >= module.declaration_pool.size()) {
                continue;
            }
            std::visit(ProjectTopLevelDecl{
                           table,
                           module,
                           targetScope,
                           symbolOrigins,
                           scopeOrigins,
                           declaration_ast_path(childId),
                           childId,
                       },
                       module.declaration_pool[childId]);
        }
    }
};

} // namespace

SymbolProjection build_symbol_projection(const AstModule& module) {
    SymbolProjection projection;
    auto& table = projection.table;

    const auto global = table.globalScope();
    const auto moduleName = source_unit_debug_name(module.source_unit);

    const auto moduleSymbol =
        table.insertSymbol(global,
                           moduleName,
                           source_unit_symbol_kind(module.source_unit.kind));
    set_declaration_location(table, moduleSymbol, module.source_unit.location);
    record_symbol_origin(projection.symbol_origins,
                         moduleSymbol,
                         "/source_unit",
                         make_origin(AstOriginEntityKind::SourceUnit,
                                     AstOriginRole::SourceUnit,
                                     module.source_unit.location));
    add_string_fact(table,
                    moduleSymbol,
                    symboltable::FactoidKind::Custom,
                    "source_unit_kind",
                    to_string(module.source_unit.kind));

    const auto moduleScope =
        table.createScope(symboltable::ScopeKind::Module,
                          global,
                          moduleSymbol,
                          moduleName);
    record_scope_origin(projection.scope_origins,
                        moduleScope,
                        "/source_unit",
                        make_origin(AstOriginEntityKind::SourceUnit,
                                    AstOriginRole::ModuleScope,
                                    module.source_unit.location));

    for (const auto declarationId : module.source_unit.declarations) {
        if (declarationId >= module.declaration_pool.size()) {
            continue;
        }
        std::visit(ProjectTopLevelDecl{
                       table,
                       module,
                       moduleScope,
                       projection.symbol_origins,
                       projection.scope_origins,
                       declaration_ast_path(declarationId),
                       declarationId,
                   },
                   module.declaration_pool[declarationId]);
    }

    return projection;
}

symboltable::SymbolTable build_symbol_table_projection(const AstModule& module) {
    return build_symbol_projection(module).table;
}

} // namespace flowmini::ast

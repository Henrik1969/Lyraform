#ifndef FLOWMINI_SYMBOL_PROJECTION_H
#define FLOWMINI_SYMBOL_PROJECTION_H

#include "flowmini_ast.h"

#include "symboltable/SymbolTable.hpp"

#include <optional>
#include <string>
#include <vector>

namespace flowmini::ast {

enum class AstOriginEntityKind {
    SourceUnit,
    Declaration,
    Statement,
    Block,
    Parameter,
    Field,
    AbiMember,
};

enum class AstOriginRole {
    SourceUnit,
    ImportDeclaration,
    FunctionDeclaration,
    FunctionParameter,
    RecordDeclaration,
    RecordField,
    RefinedTypeDeclaration,
    AbiDeclaration,
    AbiType,
    AbiStruct,
    AbiStructField,
    ExternFunction,
    ExternParameter,
    MainDeclaration,
    TargetDeclaration,
    LocalBinding,
    LocalGuard,
    ModuleScope,
    FunctionScope,
    RecordScope,
    AbiScope,
    AbiStructScope,
    ExternFunctionScope,
    MainScope,
    TargetScope,
    IfThenScope,
    WhileBodyScope,
    ElseBlockScope,
};

struct AstOriginDescriptor final {
    AstOriginEntityKind entity_kind {AstOriginEntityKind::SourceUnit};
    AstOriginRole role {AstOriginRole::SourceUnit};
    std::optional<std::size_t> ast_id;
    SourceLocation location;
};

struct SymbolAstOrigin final {
    symboltable::SymbolId symbol_id {};
    std::string ast_path;
    AstOriginDescriptor descriptor;
};

struct ScopeAstOrigin final {
    symboltable::ScopeId scope_id {};
    std::string ast_path;
    AstOriginDescriptor descriptor;
};

struct SymbolProjection final {
    symboltable::SymbolTable table;
    std::vector<SymbolAstOrigin> symbol_origins;
    std::vector<ScopeAstOrigin> scope_origins;
};

SymbolProjection build_symbol_projection(const AstModule& module);
symboltable::SymbolTable build_symbol_table_projection(const AstModule& module);

const char* to_string(AstOriginEntityKind kind);
const char* to_string(AstOriginRole role);

} // namespace flowmini::ast

#endif // FLOWMINI_SYMBOL_PROJECTION_H

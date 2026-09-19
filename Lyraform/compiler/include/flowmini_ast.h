//
// Created by henrik on 29.06.2026.
//
#ifndef FLOWMINI_AST_H
#define FLOWMINI_AST_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <ostream>
#include <variant>
#include <vector>

namespace flowmini::ast {

struct SourceLocation {
    std::size_t line = 0;
    std::size_t column = 0;
};

enum class SourceUnitKind {
    Program,
    Unit,
    Unknown
};

enum class TopLevelKind {
    Import,
    Function,
    Record,
    RefinedType,
    Abi,
    Target,
    MainBlock,
    Unknown
};

enum class StatementKind {
    Let,
    Assignment,
    Placement,
    If,
    While,
    Break,
    Continue,
    Return,
    Expression,
    Flow,
    Unknown
};

enum class ExpressionKind {
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    BoolLiteral,
    Call,
    Unary,
    Binary,
    Index,
    FieldAccess,
    ListLiteral,
    RecordLiteral,
    Unknown
};

enum class TypeRefKind {
    Named,
    Generic,
    Array,
    Unknown
};

struct TypeRef;

struct UnknownTypeRef {
    std::string text;
};

struct NamedTypeRef {
    std::vector<std::string> name_segments;
};

struct GenericTypeRef {
    std::vector<std::string> constructor_segments;
    std::vector<TypeRef> arguments;
};

struct ArrayExtent {
    std::string text;
    SourceLocation location;
};

struct ArrayTypeRef {
    std::shared_ptr<const TypeRef> element_type;
    std::vector<ArrayExtent> extents;
};

struct TypeRef {
    SourceLocation location;

    using Payload = std::variant<
        UnknownTypeRef,
        NamedTypeRef,
        GenericTypeRef,
        ArrayTypeRef
    >;

    Payload payload = UnknownTypeRef{};
};

struct Parameter {
    std::string name;
    TypeRef type;
    SourceLocation location;
};

struct Expression;

struct IdentifierExpr {
    std::string name;
};

struct IntegerLiteralExpr {
    std::string text;
};

struct FloatLiteralExpr {
    std::string text;
};

struct StringLiteralExpr {
    std::string text;
};

struct BoolLiteralExpr {
    std::string text;
};

struct UnaryExpr {
    std::string op;
    std::optional<std::size_t> operand;
};

struct BinaryExpr {
    std::string op;
    std::optional<std::size_t> left;
    std::optional<std::size_t> right;
};

struct CallExpr {
    std::optional<std::size_t> base;
    std::vector<std::size_t> arguments;
};

struct IndexExpr {
    std::optional<std::size_t> base;
    std::vector<std::size_t> indexes;
};

struct FieldAccessExpr {
    std::optional<std::size_t> base;
    std::string field;
};

struct ListLiteralExpr {
    std::vector<std::size_t> elements;
};

struct RecordLiteralFieldExpr {
    std::string name;
    std::optional<std::size_t> value;
    SourceLocation location;
};

struct RecordLiteralExpr {
    std::vector<RecordLiteralFieldExpr> fields;
};

struct UnknownExpr {
    std::string text;
};

struct Expression {
    SourceLocation location;

    using Payload = std::variant<
        UnknownExpr,
        IdentifierExpr,
        IntegerLiteralExpr,
        FloatLiteralExpr,
        StringLiteralExpr,
        BoolLiteralExpr,
        UnaryExpr,
        BinaryExpr,
        CallExpr,
        IndexExpr,
        FieldAccessExpr,
        ListLiteralExpr,
        RecordLiteralExpr
    >;

    Payload payload = UnknownExpr{};
};

using StatementId = std::size_t;
using BlockId = std::size_t;
using DeclarationId = std::size_t;

struct ElseBlock {
    BlockId block;
};

struct ElseIf {
    StatementId if_statement;
};

using ElseArm = std::variant<ElseBlock, ElseIf>;

enum class StatementSourceForm {
    EqualsAssignment,
    ArrowPlacement,
    KeywordReturn
};

struct FieldPathSegment {
    std::string name;
    SourceLocation location;
};

struct IdentifierTarget {
    std::string name;
    SourceLocation location;
};

struct FieldPathTarget {
    std::string base_identifier;
    SourceLocation location;
    std::vector<FieldPathSegment> fields;
};

struct IndexedTarget {
    std::string base_identifier;
    SourceLocation location;
    std::vector<std::size_t> indexes;
};

using AssignableTarget = std::variant<IdentifierTarget, FieldPathTarget, IndexedTarget>;

struct UnknownStatement { std::string text; };
struct LetStatement {
    std::string name;
    TypeRef type;
    std::optional<std::size_t> initializer_expression;
};
struct AssignmentStatement {
    AssignableTarget target;
    std::size_t value_expression;
    StatementSourceForm source_form = StatementSourceForm::EqualsAssignment;
};
struct PlacementStatement {
    std::size_t value_expression;
    AssignableTarget target;
    StatementSourceForm source_form = StatementSourceForm::ArrowPlacement;
};
struct IfStatement {
    std::size_t condition_expression;
    BlockId then_block;
    std::optional<ElseArm> else_arm;
};
struct WhileStatement {
    std::size_t condition_expression;
    BlockId body_block;
};
struct BreakStatement {};
struct ContinueStatement {};
struct ReturnStatement {
    std::optional<std::size_t> value_expression;
    StatementSourceForm source_form = StatementSourceForm::KeywordReturn;
};
struct ExpressionStatement {
    std::size_t expression;
    bool print = false;
};
struct FlowStatement { std::vector<std::size_t> expressions; };

struct Statement {
    SourceLocation location;

    using Payload = std::variant<
        LetStatement,
        AssignmentStatement,
        PlacementStatement,
        IfStatement,
        WhileStatement,
        BreakStatement,
        ContinueStatement,
        ReturnStatement,
        ExpressionStatement,
        FlowStatement,
        UnknownStatement
    >;

    Payload payload = UnknownStatement{};
};

struct Block {
    SourceLocation location;
    std::vector<StatementId> statements;
};

struct FunctionDecl {
    std::string name;
    std::vector<Parameter> parameters;
    TypeRef return_type;
    std::optional<BlockId> body;
    SourceLocation location;

    bool has_body = false;
    SourceLocation body_location;

    bool is_extern = false;
    bool is_imported = false;
    std::string origin_module;
};

struct ImportDecl {
    std::string module_name;
    // Source-level namespace alias. Empty means legacy unaliased import.
    std::string alias;
    SourceLocation location;
};

struct RecordField {
    std::string name;
    TypeRef type;
    SourceLocation location;
};

struct RecordDecl {
    std::string name;
    std::vector<RecordField> fields;
    SourceLocation location;
};

struct InvariantClause {
    std::size_t condition_expression = 0;
    SourceLocation location;
};

struct RefinedTypeDecl {
    std::string name;
    TypeRef base_type;
    std::vector<InvariantClause> invariants;
    SourceLocation location;
};

struct AbiLibraryClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiEvidenceClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiConventionClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiReprClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiOwnershipClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiAccessClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiLifetimeClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiNullableClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiTerminatorClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiOpaqueClause {
    std::string spelling;
    SourceLocation location;
};

struct AbiCleanupClause {
    std::string spelling;
    SourceLocation location;
};

using AbiTypeProperty = std::variant<
    AbiReprClause,
    AbiOwnershipClause,
    AbiAccessClause,
    AbiLifetimeClause,
    AbiNullableClause,
    AbiTerminatorClause,
    AbiOpaqueClause,
    AbiCleanupClause
>;

struct AbiTypeDecl {
    std::string name;
    std::vector<AbiTypeProperty> properties;
    SourceLocation location;
};

struct AbiStructDecl {
    std::string name;
    std::vector<RecordField> fields;
    SourceLocation location;
};

struct ExternSymbolClause {
    std::string spelling;
    SourceLocation location;
};

struct ExternEffectClause {
    std::string spelling;
    SourceLocation location;
};

using ExternClause = std::variant<ExternSymbolClause, ExternEffectClause>;

struct ExternFunctionDecl {
    std::string name;
    std::vector<Parameter> parameters;
    TypeRef return_type;
    std::vector<ExternClause> clauses;
    SourceLocation location;
};

using AbiMember = std::variant<
    AbiLibraryClause,
    AbiConventionClause,
    AbiEvidenceClause,
    AbiTypeDecl,
    AbiStructDecl,
    ExternFunctionDecl
>;

struct AbiDecl {
    std::string name;
    std::vector<AbiMember> members;
    SourceLocation location;
};

struct MainBlock {
    std::vector<Parameter> parameters;
    std::optional<BlockId> body;
    SourceLocation location;

    bool has_body = false;
    SourceLocation body_location;
};

struct TargetDecl {
    std::string name;
    std::vector<DeclarationId> declarations;
    SourceLocation location;
};

using TopLevelDecl = std::variant<
    ImportDecl,
    FunctionDecl,
    RecordDecl,
    RefinedTypeDecl,
    AbiDecl,
    TargetDecl,
    MainBlock
>;

struct SourceUnit {
    SourceUnitKind kind = SourceUnitKind::Unknown;
    std::string name;
    std::vector<DeclarationId> declarations;
    SourceLocation location;
};

struct GraphNodeSyntax {
    std::string role;
    std::string name;
    std::string implementation;
    bool source_function = false;
    bool persistent = false;
    SourceLocation location;
};

struct GraphEndpointSyntax {
    std::string node;
    std::string port;
    SourceLocation location;
};

struct GraphWireSyntax {
    GraphEndpointSyntax from;
    GraphEndpointSyntax to;
    SourceLocation location;
};

struct GraphPolicySyntax {
    std::string node;
    std::string key;
    std::string value_kind;
    std::string value_text;
    SourceLocation location;
};

struct GraphStateSyntax {
    std::string node;
    std::string type;
    std::string value_text;
    SourceLocation location;
};

struct ParseValidity {
    std::string state = "outside_scope";
    std::string scope = "compatibility";
    std::string coverage = "unassessed";
    bool recovery = false;
    std::string message;
};

struct AstModule {
    ParseValidity parse_validity;
    SourceUnit source_unit;
    std::vector<TopLevelDecl> declaration_pool;
    std::vector<Expression> expression_pool;
    std::vector<Statement> statement_pool;
    std::vector<Block> block_pool;
    // Syntax admitted by the compatibility interpreter but not represented in
    // the structured AST must never disappear into an executable plan.
    std::vector<SourceLocation> unsupported_graph_locations;
    std::vector<GraphNodeSyntax> graph_nodes;
    std::vector<GraphWireSyntax> graph_wires;
    std::vector<GraphPolicySyntax> graph_policies;
    std::vector<GraphStateSyntax> graph_states;
};

const char* to_string(SourceUnitKind kind);
const char* to_string(TopLevelKind kind);
const char* to_string(StatementKind kind);
const char* to_string(StatementSourceForm form);
const char* to_string(ExpressionKind kind);
const char* to_string(TypeRefKind kind);

TypeRefKind type_ref_kind(const TypeRef& type);
std::string type_ref_text(const TypeRef& type);

ExpressionKind expression_kind(const Expression& expression);
StatementKind statement_kind(const Statement& statement);
std::string expression_text(const Expression& expression,
                            const std::vector<Expression>& expression_pool);
std::string expression_tree_text(std::size_t expression_id,
                                 const std::vector<Expression>& expression_pool);
std::vector<std::size_t> expression_children(const Expression& expression);

TopLevelKind top_level_kind(const TopLevelDecl& decl);

void dump_ast_json(std::ostream& out, const AstModule& module);

AstModule make_empty_ast_module();

} // namespace flowmini::ast

#endif // FLOWMINI_AST_H

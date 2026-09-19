#pragma once
#include <flowcontracts/json.hpp>

namespace flowcontracts {
inline const std::string& target_string(const json::Value& v,std::string_view p="$.target_fact") { return json::string(v,p); }
inline json::Integer target_integer(const json::Value& v,std::string_view p="$.target_fact") { return json::integer(v,p); }
inline const json::Array& target_array(const json::Value& v) { return json::array(v,"$.target_fact"); }
inline bool target_null(const json::Value& value) { return std::holds_alternative<std::nullptr_t>(value); }
inline void target_fail(const std::string& reason="contradictory or unsupported target fact") { throw json::Error("$.target_fact",reason); }

// Versions 1 and 2 remain readable compatibility evidence. Version 3 adds
// bounded member reconstruction and root-rebinding authority.
inline void validate_target_fact(const json::Value& raw) {
    using namespace json; const auto& f=object(raw,"$.target_fact");
    const auto s=[&](const char* key)->const std::string& { return target_string(required(f,key)); };
    const auto n=[&](const char* key) { return target_integer(required(f,key)); };
    if (s("format")!="lyraform.target_fact") target_fail();
    const auto version=n("version"); const auto kind=s("kind");
    if (kind!="identifier" && kind!="field_path" && kind!="indexed") target_fail();
    if (n("operation_id")<0 || n("statement_id")<0 || n("base_symbol_id") < -1) target_fail();
    if (version==1) {
        if (f.size()!=14) target_fail();
        const auto& base=required(f,"base_type"), &destination=required(f,"destination_type");
        if (!target_null(base) && target_string(base).empty()) target_fail();
        const bool typed=!target_null(destination);
        if (typed && (kind!="identifier" || n("base_symbol_id")<0 ||
            (target_string(destination)!="int" && target_string(destination)!="Bool") || base!=destination)) target_fail();
        if (s("resolution")!=(n("base_symbol_id")<0?"unsupported_semantics":typed?"resolved":"partially_resolved")) target_fail();
        if (s("execution")!=(kind=="identifier"?"existing_policy":"unsupported")) target_fail();
    } else if (version==2 || version==3) {
        if (f.size()!=18 ||
            s("execution")!=(kind=="identifier"?"existing_policy":"unsupported")) target_fail();
        if (version==2 && s("assignability")!="unresolved") target_fail();
        const auto resolution=s("resolution");
        if (resolution!="resolved" && resolution!="partially_resolved" && resolution!="refused") target_fail();
        const auto& base=required(f,"base_type"), &base_symbol=required(f,"base_type_symbol_id");
        const auto& destination=required(f,"destination_type"), &destination_symbol=required(f,"destination_type_symbol_id");
        if (!target_null(base) && target_string(base).empty()) target_fail();
        if (!target_null(base_symbol) && target_integer(base_symbol)<0) target_fail();
        if (!target_null(destination) && target_string(destination).empty()) target_fail();
        if (!target_null(destination_symbol) && target_integer(destination_symbol)<0) target_fail();
        std::string assignability="unresolved";
        if (version==3) {
            if (kind!="field_path") target_fail("v3 fact is not a field path");
            const auto& a=object(required(f,"assignability"));
            if (a.size()!=8) target_fail("v3 assignability has an unexpected field set");
            assignability=target_string(required(a,"state"));
            if (assignability!="assignable" && assignability!="refused" && assignability!="unresolved") target_fail("v3 assignability state is invalid");
            const bool decided=assignability!="unresolved";
            for (const auto key:{"authority","root_authority","update","source_expression_id","source_type","destination_type","root_declaration_statement_id"})
                if (decided==target_null(required(a,key))) target_fail("v3 decided and nullable fields disagree");
            if (decided) {
                if (target_string(required(a,"authority"))!="lyraform.aggregate_reconstruction/v1" ||
                    target_string(required(a,"root_authority"))!="established_local_rebinding/v1" ||
                    target_string(required(a,"update"))!="reconstruct_and_rebind" ||
                    target_integer(required(a,"source_expression_id"))<0 ||
                    target_integer(required(a,"root_declaration_statement_id"))<0 ||
                    (target_string(required(a,"source_type"))!="int" && target_string(required(a,"source_type"))!="Bool") ||
                    required(a,"destination_type")!=required(f,"destination_type")) target_fail("v3 reconstruction authority is invalid");
                const bool compatible=required(a,"source_type")==required(a,"destination_type");
                if ((assignability=="assignable")!=compatible) target_fail("member compatibility result contradicts scalar types");
                if (resolution!="resolved") target_fail("v3 decided assignability lacks a resolved target");
            }
        }
        const auto& failure=required(f,"failure");
        const bool refused=resolution=="refused" || assignability=="refused";
        if (refused != !target_null(failure)) target_fail();
        if (!target_null(failure)) {
            const auto& detail=object(failure);
            if (detail.size()!=2 || target_string(required(detail,"code")).empty() || target_string(required(detail,"message")).empty()) target_fail();
            if (!target_null(destination) || !target_null(destination_symbol)) target_fail();
        }
        if (kind=="identifier" && resolution=="resolved") {
            if (n("base_symbol_id")<0 || target_null(base) || target_null(destination) || base!=destination ||
                (target_string(destination)!="int" && target_string(destination)!="Bool") ||
                !target_null(base_symbol) || !target_null(destination_symbol)) target_fail();
        }
    } else target_fail();

    const auto& members=target_array(required(f,"members"));
    const auto& indexes=target_array(required(f,"indices"));
    if ((kind=="field_path") != !members.empty() || (kind=="indexed") != !indexes.empty() ||
        n("index_count")!=static_cast<Integer>(indexes.size())) target_fail();
    for (std::size_t i=0;i<indexes.size();++i) {
        const auto& index=object(indexes[i]);
        if (index.size()!=2 || target_integer(required(index,"ordinal"))!=static_cast<Integer>(i) ||
            target_integer(required(index,"expression_id"))<0) target_fail();
    }
    for (std::size_t i=0;i<members.size();++i) {
        const auto& member=object(members[i]);
        if (target_integer(required(member,"ordinal"))!=static_cast<Integer>(i) ||
            target_string(required(member,"name")).empty() || target_string(required(member,"ast_path")).empty()) target_fail();
        const auto& location=object(required(member,"location"));
        if (location.size()!=2 || target_integer(required(location,"line"))<=0 || target_integer(required(location,"column"))<=0) target_fail();
        if (version==1) { if (member.size()!=4) target_fail(); continue; }
        if (member.size()!=12) target_fail();
        const bool semantic=!target_null(required(member,"member_symbol_id"));
        for (const auto key:{"owner_type","owner_type_symbol_id","member_declaration_name","member_declaration_path",
                            "member_declaration_location","member_type"})
            if (semantic==target_null(required(member,key))) target_fail();
        if (semantic) {
            if (target_integer(required(member,"owner_type_symbol_id"))<0 || target_integer(required(member,"member_symbol_id"))<0 ||
                target_string(required(member,"owner_type")).empty() || target_string(required(member,"member_type")).empty() ||
                target_string(required(member,"member_declaration_path")).rfind("/declaration_pool/",0)!=0) target_fail();
            const auto& declaration_location=object(required(member,"member_declaration_location"));
            static_cast<void>(target_string(required(declaration_location,"file")));
            if (declaration_location.size()!=3 ||
                target_integer(required(declaration_location,"line"))<=0 ||
                target_integer(required(declaration_location,"column"))<=0) target_fail();
            if (required(member,"name")!=required(member,"member_declaration_name")) target_fail("member name contradicts declaration identity");
            if (i==0) {
                if (required(member,"owner_type")!=required(f,"base_type") ||
                    required(member,"owner_type_symbol_id")!=required(f,"base_type_symbol_id")) target_fail("first member owner contradicts base type identity");
            } else {
                const auto& previous=object(members[i-1]);
                if (required(member,"owner_type")!=required(previous,"member_type") ||
                    required(member,"owner_type_symbol_id")!=required(previous,"member_type_symbol_id")) target_fail("member chain type identity mismatch");
            }
        }
    }
    if ((version==2 || version==3) && kind=="field_path" && s("resolution")=="resolved") {
        const auto& last=object(members.back());
        if (target_null(required(last,"member_symbol_id")) || required(last,"member_type")!=required(f,"destination_type") ||
            required(last,"member_type_symbol_id")!=required(f,"destination_type_symbol_id")) target_fail("field result type contradicts final member");
    }
    const auto& origin=object(required(f,"provenance"));
    if (origin.size()!=4 || target_string(required(origin,"source")).empty() ||
        target_string(required(origin,"ast_path"))!="/statement_pool/"+std::to_string(n("statement_id"))+"/payload/target" ||
        target_integer(required(origin,"line"))<=0 || target_integer(required(origin,"column"))<=0) target_fail();
}

inline void validate_target_facts(const json::Value& plan) {
    using namespace json; const auto& root=object(plan);
    const auto* raw_status=optional(root,"status");
    const auto plan_status=raw_status?target_string(*raw_status,"$.lowering_plan.status"):std::string{};
    for (const auto& value:target_array(required(root,"operations"))) {
        const auto& operation=object(value); const auto* raw=optional(operation,"target_fact"); if (!raw) continue;
        validate_target_fact(*raw); const auto& fact=object(*raw);
        if (required(fact,"operation_id")!=required(operation,"id") || required(fact,"statement_id")!=required(operation,"statement_id"))
            target_fail("target operation identity mismatch");
        const auto version=target_integer(required(fact,"version"));
        bool refused=version>=2 && target_string(required(fact,"resolution"))=="refused";
        if (version==3) refused=refused || target_string(required(object(required(fact,"assignability")),"state"))=="refused";
        if (refused && plan_status=="ready") target_fail("refused target cannot appear in a ready plan");
        if (version==3) {
            const auto& assignability=object(required(fact,"assignability"));
            if (!target_null(required(assignability,"source_expression_id")) &&
                required(assignability,"source_expression_id")!=required(operation,"expression_id"))
                target_fail("member source expression identity mismatch");
        }
        if (target_string(required(fact,"kind"))=="identifier") {
            if (const auto* result=optional(operation,"result_symbol_id")) if (*result!=required(fact,"base_symbol_id")) target_fail("target base identity mismatch");
            if (const auto* scalar=optional(operation,"scalar_fact")) {
                const auto& evidence=object(*scalar);
                if (required(evidence,"destination_symbol_id")!=required(fact,"base_symbol_id") ||
                    required(evidence,"destination_type")!=required(fact,"destination_type")) target_fail("target contradicts scalar type evidence");
            }
        } else if (optional(operation,"result_symbol_id")) target_fail("non-identifier target collapsed to result symbol");
    }
}
inline void require_executable_targets(const json::Value& plan) {
    validate_target_facts(plan); using namespace json;
    for (const auto& value:target_array(required(object(plan),"operations")))
        if (const auto* fact=optional(object(value),"target_fact"))
            if (target_string(required(object(*fact),"kind"))!="identifier")
                target_fail("unsupported target kind: member/index execution is not admitted");
}
} // namespace flowcontracts

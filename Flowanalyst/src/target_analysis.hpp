#pragma once
#include <flowcontracts/scalar_analysis.hpp>
#include <map>
#include <vector>

namespace flowanalyst::target {
using Json = flowcontracts::json::Value;
using Index = lyraform::scalar::frontend::Index;
struct Field {
    int symbol=-1;
    std::string name,type;
    int type_symbol=-1;
    std::string declaration_path;
    Json declaration_location=nullptr;
};
struct Aggregate { int symbol=-1, scope=-1; std::string name; std::map<std::string,std::vector<Field>> fields; };
struct Catalog { std::map<std::string,std::vector<Aggregate>> aggregates; };
struct Result { Json evidence; bool admitted=true; std::string code,message; };

inline std::string fact_value(const Json& symbol, const std::string& key) {
    using namespace lyraform::scalar::frontend;
    for (const auto& fact:items(get(&symbol,"facts")))
        if (text(get(&fact,"key"))==key) return text(get(get(&fact,"value"),"value"));
    return {};
}
inline Catalog catalog(const Index& symbols, const Index& scopes, const Index& origins) {
    using namespace lyraform::scalar::frontend;
    Catalog result; std::map<std::string,std::vector<int>> type_symbols;
    for (const auto& [key,symbol]:symbols) if (text(get(symbol,"kind"))=="Struct")
        type_symbols[text(get(symbol,"name"))].push_back(key);
    for (const auto& [key,symbol]:symbols) {
        if (text(get(symbol,"kind"))!="Struct") continue;
        Aggregate aggregate{key,id(get(symbol,"introduced_scope_id")),text(get(symbol,"name")),{}};
        if (scopes.count(aggregate.scope)) for (const auto& child:items(get(scopes.at(aggregate.scope),"symbol_ids"))) {
            const int member=id(&child);
            if (!symbols.count(member) || text(get(symbols.at(member),"kind"))!="Field") continue;
            const auto type=fact_value(*symbols.at(member),"declared_type_spelling");
            const auto found=type_symbols.find(type);
            const int type_symbol=found!=type_symbols.end() && found->second.size()==1 ? found->second.front() : -1;
            aggregate.fields[text(get(symbols.at(member),"name"))].push_back(Field{member,
                text(get(symbols.at(member),"name")),type,type_symbol,
                origins.count(member)?text(get(origins.at(member),"ast_path")):std::string{},
                required(object(*symbols.at(member)),"declaration_location")});
        }
        result.aggregates[aggregate.name].push_back(std::move(aggregate));
    }
    return result;
}

inline Result resolve(const Json& bundle,const Json& target,int statement,int operation,int base,
    const std::string& base_type,const std::string& scalar_destination_type,const Catalog& types,
    int source_expression=-1,lyraform::scalar::Type source_type=lyraform::scalar::Type::outside_slice,
    int root_declaration_statement=-1) {
    using namespace flowcontracts::json; using namespace lyraform::scalar::frontend;
    const auto kind=text(get(&target,"kind"));
    const auto path="/statement_pool/"+std::to_string(statement)+"/payload/target";
    Array members,indices;
    for (const auto& entry:items(get(&target,"fields"))) {
        const auto ordinal=static_cast<Integer>(members.size());
        members.emplace_back(Object{{"ordinal",ordinal},{"name",text(get(&entry,"name"))},
            {"ast_path",path+"/fields/"+std::to_string(ordinal)},
            {"location",required(object(entry),"location")},{"owner_type",nullptr},
            {"owner_type_symbol_id",nullptr},{"member_symbol_id",nullptr},
            {"member_declaration_name",nullptr},{"member_declaration_path",nullptr},
            {"member_declaration_location",nullptr},
            {"member_type",nullptr},{"member_type_symbol_id",nullptr}});
    }
    for (const auto& entry:items(get(&target,"indexes")))
        indices.emplace_back(Object{{"ordinal",static_cast<Integer>(indices.size())},{"expression_id",entry}});
    auto line=id(get(get(&target,"location"),"line")); const auto column=id(get(get(&target,"location"),"column"));
    auto source=text(get(get(&bundle,"source"),"path")); const auto* map=get(&bundle,"source_map");
    for (const auto& entry:items(get(map,"lines"))) if (id(get(&entry,"expanded_line"))==line) {
        line=id(get(&entry,"source_line"));
        for (const auto& file:items(get(map,"files"))) if (id(get(&file,"id"))==id(get(&entry,"source_id"))) source=text(get(&file,"path"));
        break;
    }
    int base_type_symbol=-1;
    if (const auto found=types.aggregates.find(base_type); found!=types.aggregates.end() && found->second.size()==1)
        base_type_symbol=found->second.front().symbol;
    Result result; std::string resolution=base<0?"refused":scalar_destination_type.empty()?"partially_resolved":"resolved";
    std::string resulting_type=scalar_destination_type; int resulting_type_symbol=-1;
    if (kind=="field_path" && base<0) {
        result.admitted=false; result.code="FLOWANALYST_TARGET_BASE_UNRESOLVED";
        result.message="target base declaration is unresolved"; resolution="refused"; resulting_type.clear();
    } else if (kind=="field_path") {
        resolution="resolved"; resulting_type=base_type;
        for (std::size_t ordinal=0;ordinal<members.size();++ordinal) {
            auto& segment=std::get<Object>(members[ordinal]);
            const auto owner=types.aggregates.find(resulting_type);
            if (owner==types.aggregates.end() || owner->second.empty()) {
                result.admitted=false; result.code="FLOWANALYST_FIELD_OWNER_NOT_AGGREGATE";
                result.message="field segment requires aggregate owner type '"+resulting_type+"'"; resolution="refused"; break;
            }
            if (owner->second.size()!=1) {
                result.admitted=false; result.code="FLOWANALYST_FIELD_OWNER_AMBIGUOUS";
                result.message="field owner type '"+resulting_type+"' is ambiguous"; resolution="refused"; break;
            }
            const auto& aggregate=owner->second.front(); segment["owner_type"]=aggregate.name;
            segment["owner_type_symbol_id"]=Integer{aggregate.symbol};
            const auto name=string(required(segment,"name"),"$.target.member.name"); const auto candidates=aggregate.fields.find(name);
            if (candidates==aggregate.fields.end() || candidates->second.empty()) {
                result.admitted=false; result.code="FLOWANALYST_FIELD_NOT_FOUND";
                result.message="member '"+name+"' does not exist in '"+aggregate.name+"'"; resolution="refused"; break;
            }
            if (candidates->second.size()!=1) {
                result.admitted=false; result.code="FLOWANALYST_FIELD_AMBIGUOUS";
                result.message="member '"+name+"' is ambiguous in '"+aggregate.name+"'"; resolution="refused"; break;
            }
            const auto& member=candidates->second.front(); segment["member_symbol_id"]=Integer{member.symbol};
            segment["member_declaration_name"]=member.name; segment["member_declaration_path"]=member.declaration_path;
            segment["member_declaration_location"]=member.declaration_location;
            segment["member_type"]=member.type;
            segment["member_type_symbol_id"]=member.type_symbol>=0?Json{Integer{member.type_symbol}}:Json{nullptr};
            resulting_type=member.type; resulting_type_symbol=member.type_symbol;
        }
        if (!result.admitted) { resulting_type.clear(); resulting_type_symbol=-1; }
    } else if (base<0) {
        result.admitted=false; result.code="FLOWANALYST_TARGET_BASE_UNRESOLVED";
        result.message="target base declaration is unresolved";
    }
    const bool member_candidate=kind=="field_path" && result.admitted && resolution=="resolved" &&
        root_declaration_statement>=0 && lyraform::scalar::selected(source_type) &&
        lyraform::scalar::selected(lyraform::scalar::type(resulting_type));
    std::string assignability="unresolved";
    if (member_candidate) {
        assignability=lyraform::scalar::compatible(source_type,lyraform::scalar::type(resulting_type))?"assignable":"refused";
        if (assignability=="refused") {
            result.admitted=false; result.code="FLOWANALYST_MEMBER_ASSIGNABILITY_REFUSED";
            result.message="member flow requires source and destination members of the same scalar type (source "+
                std::string(lyraform::scalar::name(source_type))+", destination "+resulting_type+")";
        }
    }
    const auto failure=result.admitted?Json{nullptr}:Json{Object{{"code",result.code},{"message",result.message}}};
    Json assignability_fact=assignability;
    Integer version=2;
    if (kind=="field_path") {
        version=3;
        const bool decided=assignability!="unresolved";
        assignability_fact=Object{{"state",assignability},
            {"authority",decided?Json{"lyraform.aggregate_reconstruction/v1"}:Json{nullptr}},
            {"root_authority",decided?Json{"established_local_rebinding/v1"}:Json{nullptr}},
            {"update",decided?Json{"reconstruct_and_rebind"}:Json{nullptr}},
            {"source_expression_id",decided?Json{Integer{source_expression}}:Json{nullptr}},
            {"source_type",decided?Json{std::string(lyraform::scalar::name(source_type))}:Json{nullptr}},
            {"destination_type",decided?Json{resulting_type}:Json{nullptr}},
            {"root_declaration_statement_id",decided?Json{Integer{root_declaration_statement}}:Json{nullptr}}};
    }
    result.evidence=Object{{"format","lyraform.target_fact"},{"version",version},{"kind",kind},
        {"operation_id",Integer{operation}},{"statement_id",Integer{statement}},{"base_symbol_id",Integer{base}},
        {"base_type",base_type.empty()?Json{nullptr}:Json{base_type}},
        {"base_type_symbol_id",base_type_symbol>=0?Json{Integer{base_type_symbol}}:Json{nullptr}},
        {"destination_type",resulting_type.empty()?Json{nullptr}:Json{resulting_type}},
        {"destination_type_symbol_id",resulting_type_symbol>=0?Json{Integer{resulting_type_symbol}}:Json{nullptr}},
        {"resolution",resolution},{"assignability",assignability_fact},
        {"execution",kind=="identifier"?"existing_policy":"unsupported"},
        {"index_count",static_cast<Integer>(indices.size())},{"members",members},{"indices",indices},{"failure",failure},
        {"provenance",Object{{"source",source},{"ast_path",path},{"line",Integer{line}},{"column",Integer{column}}}}};
    return result;
}
} // namespace flowanalyst::target

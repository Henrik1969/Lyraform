#pragma once
#include <flowcontracts/json.hpp>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace flowcontracts {

// Uniform owned-value law. These projections contain resolved identities;
// neither source spellings nor backend carrier names select the semantics.
struct OwnershipOperation {
    long long id=-1, statement=-1, block=-1, function=-1, result=-1, callee=-1;
    std::string kind;
    std::set<long long> reads;
    std::size_t argument_count=0;
};
struct OwnershipFunction {
    long long id=-1, block=-1;
    std::string result_type;
    bool entry=false;
    std::size_t parameters=0;
};
struct OwnershipTransfer {
    long long producer=-1, returned=-1, call=-1, source_owner=-1, destination_owner=-1;
    long long source_function=-1, destination_function=-1;
    std::string value_type, obligation;
};

inline json::Value ownership_transfer_fact(const OwnershipTransfer& t) {
    using namespace json;
    return Object{{"format", "lyraform.ownership_transfer"}, {"version", 1},
        {"kind", "function_return"}, {"mode", "unique"}, {"obligation_identity", t.obligation},
        {"value_type", t.value_type}, {"producer_operation_id", Integer(t.producer)},
        {"return_operation_id", Integer(t.returned)}, {"call_operation_id", Integer(t.call)},
        {"source_owner_symbol_id", Integer(t.source_owner)},
        {"destination_owner_symbol_id", Integer(t.destination_owner)},
        {"source_function_symbol_id", Integer(t.source_function)},
        {"destination_function_symbol_id", Integer(t.destination_function)}};
}

inline OwnershipTransfer read_ownership_transfer(const json::Value& raw, const std::string& path) {
    using namespace json;
    const auto& fact=object(raw,path);
    auto number=[&](std::string_view key){return integer(required(fact,key,path),path+"."+std::string(key));};
    auto text=[&](std::string_view key){return string(required(fact,key,path),path+"."+std::string(key));};
    if (text("format")!="lyraform.ownership_transfer" || number("version")!=1 ||
        text("kind")!="function_return" || text("mode")!="unique")
        throw Error(path,"unsupported ownership transfer contract");
    return {number("producer_operation_id"),number("return_operation_id"),number("call_operation_id"),
        number("source_owner_symbol_id"),number("destination_owner_symbol_id"),
        number("source_function_symbol_id"),number("destination_function_symbol_id"),
        text("value_type"),text("obligation_identity")};
}

inline std::string ownership_transfer_refusal(const OwnershipTransfer& t,
    const std::vector<OwnershipOperation>& operations, const std::vector<OwnershipFunction>& functions) {
    std::map<long long,const OwnershipOperation*> ops;
    std::map<long long,const OwnershipFunction*> fns;
    for (const auto& op:operations) if(!ops.emplace(op.id,&op).second) return "duplicate ownership operation";
    for (const auto& fn:functions) if(!fns.emplace(fn.id,&fn).second) return "duplicate ownership function";
    if (t.source_owner<0 || t.destination_owner<0 || t.source_owner==t.destination_owner ||
        t.source_function==t.destination_function || t.value_type.empty() || t.obligation.empty())
        return "invalid ownership transfer identity";
    if (!ops.count(t.producer)||!ops.count(t.returned)||!ops.count(t.call)||
        !fns.count(t.source_function)||!fns.count(t.destination_function))
        return "ownership transfer relation is missing";
    const auto& producer=*ops.at(t.producer);
    const auto& returned=*ops.at(t.returned);
    const auto& call=*ops.at(t.call);
    const auto& source=*fns.at(t.source_function);
    const auto& destination=*fns.at(t.destination_function);
    if (source.entry || !destination.entry || source.parameters!=0 || source.result_type!=t.value_type ||
        source.block<0 || destination.block<0)
        return "ownership transfer requires one nullary producer and one entry caller";
    if (producer.function!=source.id || producer.block!=source.block || producer.result!=t.source_owner ||
        returned.function!=source.id || returned.block!=source.block || returned.kind!="return_value" ||
        returned.statement<=producer.statement || returned.reads!=std::set<long long>{t.source_owner} ||
        call.kind!="call" || call.callee!=source.id || call.function!=destination.id ||
        call.block!=destination.block || call.result!=t.destination_owner || !call.reads.empty() || call.argument_count!=0)
        return "ownership producer, return, call and destination disagree";
    int callers=0;
    for (const auto& op:operations) {
        if(op.callee==source.id && op.kind=="call") ++callers;
        if(op.reads.count(t.source_owner) && op.id!=t.returned)
            return "transferred owner is used outside its unique return";
        if(op.result==t.source_owner && op.id!=t.producer &&
            !(op.kind=="value_definition" && op.statement==producer.statement && op.function==source.id))
            return "transferred source owner is overwritten";
        if(op.result==t.destination_owner && op.id!=t.call &&
            !(op.kind=="value_definition" && op.statement==call.statement && op.function==destination.id))
            return "transferred destination owner is overwritten";
        if(op.function==source.id && op.id!=t.producer && op.id!=t.returned &&
            !(op.kind=="value_definition" && op.statement==producer.statement && op.result==t.source_owner))
            return "producer function is outside the direct owned-return slice";
    }
    if(callers!=1) return "owned return requires exactly one static caller in this slice";
    return {};
}

inline void ownership_operand_reads(const json::Value& value, std::set<long long>& reads) {
    using namespace json;
    if(const auto* obj=std::get_if<Object>(&value)) {
        const auto* kind=optional(*obj,"kind"); const auto* symbol=optional(*obj,"symbol_id");
        if(kind && symbol && string(*kind,"$.kind")=="identifier") reads.insert(integer(*symbol,"$.symbol_id"));
        for(const auto& [key,child]:*obj) { (void)key; ownership_operand_reads(child,reads); }
    } else if(const auto* items=std::get_if<Array>(&value))
        for(const auto& child:*items) ownership_operand_reads(child,reads);
}

inline std::string validate_ownership_transfer_projection(const OwnershipTransfer& transfer,
    const json::Object& plan) {
    using namespace json;
    std::vector<OwnershipOperation> operations;
    std::vector<OwnershipFunction> functions;
    for(const auto& raw:array(required(plan,"operations"),"$.operations")) {
        const auto& op=object(raw);
        auto number=[&](std::string_view key)->long long {
            const auto* value=optional(op,key); return value?integer(*value,"$.operation"): -1;
        };
        std::set<long long> reads;
        if(const auto* operands=optional(op,"operands")) ownership_operand_reads(*operands,reads);
        operations.push_back({number("id"),number("statement_id"),number("block_id"),number("function_symbol_id"),
            number("result_symbol_id"),number("callee_symbol_id"),string(required(op,"kind"),"$.kind"),reads,array(required(op,"operands"),"$.operands").size()});
    }
    for(const auto& raw:array(required(plan,"functions"),"$.functions")) {
        const auto& fn=object(raw);
        functions.push_back({integer(required(fn,"symbol_id"),"$.symbol_id"),integer(required(fn,"body_block_id"),"$.body_block_id"),
            string(required(fn,"return_type"),"$.return_type"),boolean(required(fn,"entry"),"$.entry"),
            array(required(fn,"parameters"),"$.parameters").size()});
    }
    const auto error=ownership_transfer_refusal(transfer,operations,functions);
    if(!error.empty()) return error;
    // A symbol buried inside a larger expression is not a direct transfer.
    for(const auto& raw:array(required(plan,"operations"),"$.operations")) {
        const auto& op=object(raw);
        if(integer(required(op,"id"),"$.id")!=transfer.returned) continue;
        const auto& operands=array(required(op,"operands"),"$.operands");
        if(operands.size()!=1 || string(required(object(operands[0]),"kind"),"$.kind")!="identifier")
            return "owned return must directly transfer its owner";
    }
    return {};
}
} // namespace flowcontracts

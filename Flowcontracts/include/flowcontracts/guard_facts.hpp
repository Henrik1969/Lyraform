#pragma once

#include <flowcontracts/json.hpp>

#include <map>
#include <set>
#include <string>
#include <string_view>

namespace flowcontracts {

inline void validate_guard_facts(const json::Value& value, std::string_view base) {
    using namespace json;
    const auto& plan = object(value, base);
    const auto* raw_facts = optional(plan, "guard_facts");
    if (!raw_facts) return; // Artifacts captured before guard authority make no guard claim.
    const auto& facts = array(*raw_facts, std::string(base) + ".guard_facts");
    const auto& operations = array(required(plan, "operations", base), std::string(base) + ".operations");
    std::map<Integer, const Object*> operations_by_id;
    for (const auto& raw_operation : operations) {
        const auto& operation = object(raw_operation);
        operations_by_id.emplace(integer(required(operation, "id"), "$.operation.id"), &operation);
    }
    const auto status = string(required(plan, "status", base), std::string(base) + ".status");
    std::set<Integer> active;
    std::set<Integer> identities;
    for (std::size_t index = 0; index < facts.size(); ++index) {
        const auto path = std::string(base) + ".guard_facts[" + std::to_string(index) + "]";
        const auto& fact = object(facts[index], path);
        if (string(required(fact, "format", path), path + ".format") != "lyraform.guard_fact" ||
            integer(required(fact, "version", path), path + ".version") != 1)
            throw Error(path, "unsupported guard fact contract");
        const auto fact_id = integer(required(fact, "fact_id", path), path + ".fact_id");
        if (fact_id < 0 || !identities.insert(fact_id).second)
            throw Error(path + ".fact_id", "invalid or duplicate guard fact identity");
        const auto event = string(required(fact, "event", path), path + ".event");
        const auto classification = string(required(fact, "classification", path), path + ".classification");
        const auto execution = string(required(fact, "execution", path), path + ".execution");
        const bool accepted = classification == "proven_safe" || classification == "deactivated";
        const bool refused = classification == "proven_violation" || classification == "not_provable";
        const auto guard = integer(required(fact, "guard_symbol_id", path), path + ".guard_symbol_id");
        const auto statement = integer(required(fact, "statement_id", path), path + ".statement_id");
        const auto scope = integer(required(fact, "scope_id", path), path + ".scope_id");
        const auto predicate = integer(required(fact, "predicate_expression_id", path), path + ".predicate_expression_id");
        if (guard < 0 || statement < 0 || scope < 0 || predicate < 0 ||
            string(required(fact, "guard_name", path), path + ".guard_name").empty())
            throw Error(path, "guard identity is invalid");
        const auto& dependencies = array(required(fact, "dependencies", path), path + ".dependencies");
        Integer previous = -1;
        if (dependencies.empty()) throw Error(path + ".dependencies", "guard dependency set is empty");
        for (const auto& raw_dependency : dependencies) {
            const auto dependency = integer(raw_dependency, path + ".dependencies");
            if (dependency < 0 || dependency <= previous)
                throw Error(path + ".dependencies", "guard dependencies must be sorted unique identities");
            previous = dependency;
        }
        const auto& provenance = object(required(fact, "provenance", path), path + ".provenance");
        if (string(required(provenance, "source", path), path + ".provenance.source").empty() ||
            string(required(provenance, "ast_path", path), path + ".provenance.ast_path") !=
                "/statement_pool/" + std::to_string(statement) ||
            integer(required(provenance, "line", path), path + ".provenance.line") < 1 ||
            integer(required(provenance, "column", path), path + ".provenance.column") < 1)
            throw Error(path, "invalid guard fact provenance");
        const auto* affected = optional(fact, "affected_operation_id");
        if (!affected) throw Error(path, "guard fact lacks affected operation field");
        if (event == "activate") {
            if (classification == "deactivated") throw Error(path, "guard activation classification mismatch");
            if (!std::holds_alternative<std::nullptr_t>(*affected) ||
                (accepted && !active.insert(guard).second))
                throw Error(path, "invalid guard activation lifecycle");
        } else if (event == "deactivate") {
            if (!std::holds_alternative<std::nullptr_t>(*affected) || !active.erase(guard))
                throw Error(path, "invalid guard deactivation lifecycle");
            if (classification != "deactivated") throw Error(path, "guard deactivation classification mismatch");
        } else if (event == "transition") {
            if (classification == "deactivated") throw Error(path, "guard transition classification mismatch");
            if (!active.count(guard))
                throw Error(path, "guard transition is outside its active lifecycle");
            if (std::holds_alternative<std::nullptr_t>(*affected)) {
                if (!refused) throw Error(path, "admitted guard transition lacks an operation");
            } else {
                const auto operation_id = integer(*affected, path + ".affected_operation_id");
                const auto found = operations_by_id.find(operation_id);
                if (found == operations_by_id.end()) throw Error(path, "guard transition operation is absent");
                const auto& operation = *found->second;
                const auto operation_kind = string(required(operation, "kind"), path + ".operation.kind");
                if ((operation_kind != "assignment" && operation_kind != "call") ||
                    integer(required(operation, "statement_id"), path + ".operation.statement_id") != statement)
                    throw Error(path, "guard transition operation linkage mismatch");
                const auto destination = integer(required(operation, "result_symbol_id"), path + ".operation.result_symbol_id");
                bool dependency = false;
                for (const auto& raw_dependency : dependencies)
                    dependency = dependency || integer(raw_dependency, path + ".dependencies") == destination;
                if (!dependency) throw Error(path, "guard transition does not affect a declared dependency");
            }
        } else throw Error(path + ".event", "unsupported guard event");
        if ((!accepted && !refused) || (accepted && execution != "elided_static") ||
            (refused && execution != "unsupported"))
            throw Error(path, "guard classification and execution disagree");
        if (status == "ready" && refused)
            throw Error(path, "ready plan contains an unenforced or violating guard fact");
    }
}

} // namespace flowcontracts

#pragma once

#include <flowcontracts/json.hpp>

#include <set>
#include <map>
#include <string>
#include <string_view>

namespace flowcontracts {

inline void validate_source_operation_coverage(const json::Value& value,
                                               std::string_view path = "$.source_operation_coverage") {
    const auto& coverage = json::object(value, path);
    auto member_path = [&](std::string_view name) {
        return std::string(path) + "." + std::string(name);
    };
    if (json::string(json::required(coverage, "format", path), member_path("format")) !=
            "lyraform.source_operation_coverage" ||
        json::integer(json::required(coverage, "version", path), member_path("version")) != 1)
        throw json::Error(std::string(path), "unsupported source-operation coverage contract");
    const auto status = json::string(json::required(coverage, "status", path), member_path("status"));
    if (status != "complete" && status != "refused")
        throw json::Error(member_path("status"), "unsupported source-operation coverage status");
    const auto statement_count = json::integer(
        json::required(coverage, "statement_count", path), member_path("statement_count"));
    const auto refused_count = json::integer(
        json::required(coverage, "refused_count", path), member_path("refused_count"));
    if (statement_count < 0 || refused_count < 0 || refused_count > statement_count)
        throw json::Error(std::string(path), "invalid source-operation coverage counts");
    const auto& statements = json::array(
        json::required(coverage, "statements", path), member_path("statements"));
    if (static_cast<json::Integer>(statements.size()) != statement_count)
        throw json::Error(member_path("statement_count"), "statement count does not match evidence");
    std::set<json::Integer> statement_ids;
    json::Integer observed_refusals = 0;
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto item_path = member_path("statements") + "[" + std::to_string(index) + "]";
        const auto& item = json::object(statements[index], item_path);
        const auto statement_id = json::integer(
            json::required(item, "statement_id", item_path), item_path + ".statement_id");
        if (statement_id < 0 || !statement_ids.insert(statement_id).second)
            throw json::Error(item_path + ".statement_id", "invalid or duplicate statement identity");
        (void)json::string(json::required(item, "kind", item_path), item_path + ".kind");
        const auto disposition = json::string(
            json::required(item, "disposition", item_path), item_path + ".disposition");
        if (disposition != "lowered" && disposition != "static_semantic" &&
            disposition != "declaration_only" && disposition != "elided_pure" &&
            disposition != "graph_projection" && disposition != "refused")
            throw json::Error(item_path + ".disposition", "unsupported statement disposition");
        if (disposition == "refused") ++observed_refusals;
        const auto& operations = json::array(
            json::required(item, "operation_ids", item_path), item_path + ".operation_ids");
        std::set<json::Integer> operation_ids;
        for (std::size_t operation = 0; operation < operations.size(); ++operation) {
            const auto operation_path = item_path + ".operation_ids[" + std::to_string(operation) + "]";
            const auto operation_id = json::integer(operations[operation], operation_path);
            if (operation_id < 0 || !operation_ids.insert(operation_id).second)
                throw json::Error(operation_path, "invalid or duplicate operation identity");
        }
        if ((disposition == "lowered") != !operations.empty())
            throw json::Error(item_path, "lowered disposition must name at least one operation and only lowered statements may name operations");
    }
    if (observed_refusals != refused_count)
        throw json::Error(member_path("refused_count"), "refused count does not match evidence");
    if ((status == "complete") != (refused_count == 0))
        throw json::Error(member_path("status"), "coverage status does not match refused count");
}

inline void validate_source_operation_links(const json::Value& value,
                                            const json::Array& operations,
                                            std::string_view path = "$.source_operation_coverage") {
    const auto& coverage = json::object(value, path);
    std::map<json::Integer, json::Integer> operation_statements;
    for (std::size_t index = 0; index < operations.size(); ++index) {
        const auto operation_path = "$.operations[" + std::to_string(index) + "]";
        const auto& operation = json::object(operations[index], operation_path);
        const auto id = json::integer(json::required(operation, "id", operation_path), operation_path + ".id");
        const auto statement = json::integer(
            json::required(operation, "statement_id", operation_path), operation_path + ".statement_id");
        operation_statements.emplace(id, statement);
    }
    const auto& statements = json::array(
        json::required(coverage, "statements", path), std::string(path) + ".statements");
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto item_path = std::string(path) + ".statements[" + std::to_string(index) + "]";
        const auto& item = json::object(statements[index], item_path);
        const auto statement = json::integer(
            json::required(item, "statement_id", item_path), item_path + ".statement_id");
        const auto& ids = json::array(
            json::required(item, "operation_ids", item_path), item_path + ".operation_ids");
        for (std::size_t operation = 0; operation < ids.size(); ++operation) {
            const auto operation_path = item_path + ".operation_ids[" + std::to_string(operation) + "]";
            const auto id = json::integer(ids[operation], operation_path);
            const auto found = operation_statements.find(id);
            if (found == operation_statements.end())
                throw json::Error(operation_path, "source coverage references an unknown operation");
            if (found->second != statement)
                throw json::Error(operation_path, "source coverage operation belongs to a different statement");
        }
    }
}

} // namespace flowcontracts

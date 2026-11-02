#pragma once

#include <flowcontracts/json.hpp>
#include <cctype>
#include <string_view>

namespace flowcontracts {

// Check type spelling, never arbitrary source text or value names. Qualified
// library names are not the bare primitive-family spelling.
inline bool contains_bare_int(std::string_view spelling) {
    const auto identifier = [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '.' || c == ':';
    };
    for (std::size_t at = 0; (at = spelling.find("int", at)) != std::string_view::npos; at += 3) {
        if ((at == 0 || !identifier(spelling[at - 1])) &&
            (at + 3 == spelling.size() || !identifier(spelling[at + 3]))) return true;
    }
    return false;
}

inline void validate_explicit_integer_width(const json::Value& value,
                                           const std::string& path, unsigned depth = 0) {
    if (depth >= 256) throw json::Error(path, "integer type inspection depth limit exceeded");
    if (const auto* entries = std::get_if<json::Array>(&value)) {
        for (std::size_t i = 0; i < entries->size(); ++i)
            validate_explicit_integer_width((*entries)[i], path + "[" + std::to_string(i) + "]", depth + 1);
    } else if (const auto* item = std::get_if<json::Object>(&value)) {
        const auto kind = item->find("kind");
        const auto* kind_name = kind == item->end() ? nullptr : std::get_if<std::string>(&kind->second);
        for (const auto& [key, child] : *item) {
            const bool type_field = (key == "name" && kind_name && *kind_name == "scalar") || key == "type" || key == "from_type" || key == "source_type" ||
                key == "destination_type" || key == "return_type" || key == "parameter_types" ||
                key == "payload_type" || key == "success_type" || key == "failure_type" || key == "base_type";
            if (type_field) if (const auto* spelling = std::get_if<std::string>(&child))
                if (contains_bare_int(*spelling))
                    throw json::Error(path + "." + key,
                        "bare int has no executable width; choose an explicit integer type such as int32 or int64");
            validate_explicit_integer_width(child, path + "." + key, depth + 1);
        }
    }
}

} // namespace flowcontracts

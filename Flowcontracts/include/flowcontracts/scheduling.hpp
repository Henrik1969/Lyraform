#pragma once

#include <flowcontracts/json.hpp>

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace flowcontracts {

struct SchedulingRefusal {
    std::string field;
    std::string request;
    std::string reason;
};

inline std::optional<SchedulingRefusal> scheduling_refusal(
    const json::Object& root,
    bool allow_parallel_independent
) {
    if (const auto* item = json::optional(root, "schedule_policy")) {
        const auto value = json::string(*item, "$.schedule_policy");
        if (value != "serial" && (!allow_parallel_independent || value != "parallel_independent_v1"))
            return SchedulingRefusal{"schedule_policy", value,
                                     "schedule policy '" + value + "' is not admitted at this boundary"};
    }

    constexpr std::array<std::pair<std::string_view, std::string_view>, 8> controls{{
        {"cancellation", "cancellation"},
        {"async", "asynchronous execution"},
        {"backpressure", "backpressure"},
        {"reentrancy", "reentrant execution"},
        {"nested", "nested execution"},
        {"distributed", "distributed execution"},
        {"retry", "automatic retry"},
        {"irreversible", "irreversible effects"},
    }};
    for (const auto& [field, description] : controls) {
        if (const auto* item = json::optional(root, field)) {
            const auto value = json::string(*item, "$." + std::string(field));
            if (value != "none")
                return SchedulingRefusal{std::string(field), std::string(field),
                                         std::string(description) + " is not admitted at this boundary"};
        }
    }
    return std::nullopt;
}

inline void validate_scheduling_request(const json::Object& root, bool allow_parallel_independent) {
    if (const auto refusal = scheduling_refusal(root, allow_parallel_independent))
        throw json::Error("$." + refusal->field, refusal->reason);
}

} // namespace flowcontracts

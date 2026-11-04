#pragma once

#include <charconv>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// Shared float64 admission and value classification.  This policy is about
// language values, not host-provider selection or backend instruction names.
namespace flowcontracts::floating {

enum class Class { finite, positive_infinity, negative_infinity, nan };

inline constexpr std::string_view format = "IEEE-754 binary64";
inline constexpr std::string_view rounding = "nearest_even";

// Parse a source/artifact decimal as one binary64 value.  from_chars gives us
// a locale-independent conversion and rejects overflow as a range error;
// non-finite spellings are not source literals in the bounded language slice.
inline std::optional<double> finite_literal(std::string_view spelling) {
    if (spelling.empty()) return std::nullopt;
    double value = 0.0;
    const auto result = std::from_chars(
        spelling.data(), spelling.data() + spelling.size(), value,
        std::chars_format::general);
    if (result.ec != std::errc{} || result.ptr != spelling.data() + spelling.size() ||
        !std::isfinite(value)) return std::nullopt;
    return value;
}

inline Class classify(double value) noexcept {
    if (std::isnan(value)) return Class::nan;
    if (std::isinf(value)) return std::signbit(value) ? Class::negative_infinity : Class::positive_infinity;
    return Class::finite;
}

inline bool exceptional(double value) noexcept {
    return classify(value) != Class::finite;
}

} // namespace flowcontracts::floating

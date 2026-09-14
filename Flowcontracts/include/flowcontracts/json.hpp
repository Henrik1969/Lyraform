#pragma once

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace flowcontracts::json {

struct Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value, std::less<>>;
using Integer = std::int64_t;
using Number = std::variant<Integer, double>;

struct Value : std::variant<std::nullptr_t, bool, Number, std::string, Array, Object> {
    using variant::variant;
    Value(int value) : variant(Number{static_cast<Integer>(value)}) {}
    Value(unsigned value) : variant(Number{static_cast<Integer>(value)}) {}
    Value(Integer value) : variant(Number{value}) {}
    Value(double value) : variant(Number{value}) {}
};

class Error : public std::runtime_error {
public:
    Error(std::string path, std::string reason)
        : std::runtime_error(path + ": " + reason), path_(std::move(path)), reason_(std::move(reason)) {}
    const std::string& path() const noexcept { return path_; }
    const std::string& reason() const noexcept { return reason_; }
private:
    std::string path_;
    std::string reason_;
};

class Parser {
public:
    explicit Parser(std::string_view input) : input_(input) {}

    Value parse() {
        skip_space();
        auto result = value("$");
        skip_space();
        if (!done()) fail("$", "trailing input at byte " + std::to_string(position_));
        return result;
    }

private:
    static constexpr std::size_t max_nodes_ = 1000000;
    static constexpr std::size_t max_depth_ = 256;
    static constexpr std::size_t max_collection_entries_ = 100000;

    struct DepthScope {
        Parser& parser;
        explicit DepthScope(Parser& owner, const std::string& path) : parser(owner) {
            if (parser.depth_ >= max_depth_) parser.fail(path, "JSON nesting exceeds the 256-level limit");
            ++parser.depth_;
        }
        ~DepthScope() { --parser.depth_; }
    };

    std::string_view input_;
    std::size_t position_ = 0;
    std::size_t nodes_ = 0;
    std::size_t depth_ = 0;

    bool done() const { return position_ == input_.size(); }
    char current() const { return done() ? '\0' : input_[position_]; }
    void skip_space() {
        while (!done() && (current() == ' ' || current() == '\t' || current() == '\n' || current() == '\r')) ++position_;
    }
    [[noreturn]] void fail(const std::string& path, const std::string& reason) const { throw Error(path, reason); }
    void expect(char wanted, const std::string& path) {
        if (current() != wanted) fail(path, std::string("expected '") + wanted + "' at byte " + std::to_string(position_));
        ++position_;
    }
    bool consume(std::string_view token) {
        if (input_.substr(position_, token.size()) != token) return false;
        position_ += token.size();
        return true;
    }
    static std::string child_path(const std::string& path, std::string_view key) {
        return path + "." + std::string(key);
    }
    Value value(const std::string& path) {
        if (++nodes_ > max_nodes_) fail(path, "JSON node count exceeds the 1000000-node limit");
        skip_space();
        switch (current()) {
            case '{': return object(path);
            case '[': return array(path);
            case '"': return string(path);
            case 't': if (consume("true")) return true; break;
            case 'f': if (consume("false")) return false; break;
            case 'n': if (consume("null")) return nullptr; break;
            default: if (current() == '-' || (current() >= '0' && current() <= '9')) return number(path);
        }
        fail(path, "invalid JSON value at byte " + std::to_string(position_));
    }
    Object object(const std::string& path) {
        DepthScope depth(*this, path);
        Object result;
        expect('{', path); skip_space();
        if (current() == '}') { ++position_; return result; }
        for (;;) {
            if (current() != '"') fail(path, "object key must be a string at byte " + std::to_string(position_));
            if (result.size() >= max_collection_entries_) fail(path, "JSON object exceeds the 100000-entry limit");
            auto key = string(path);
            skip_space(); expect(':', child_path(path, key));
            if (result.contains(key)) fail(child_path(path, key), "duplicate object key");
            result.emplace(key, value(child_path(path, key)));
            skip_space();
            if (current() == '}') { ++position_; return result; }
            expect(',', path); skip_space();
        }
    }
    Array array(const std::string& path) {
        DepthScope depth(*this, path);
        Array result;
        expect('[', path); skip_space();
        if (current() == ']') { ++position_; return result; }
        for (;;) {
            if (result.size() >= max_collection_entries_) fail(path, "JSON array exceeds the 100000-entry limit");
            result.push_back(value(path + "[" + std::to_string(result.size()) + "]"));
            skip_space();
            if (current() == ']') { ++position_; return result; }
            expect(',', path); skip_space();
        }
    }
    static unsigned hex_digit(char c) {
        if (c >= '0' && c <= '9') return static_cast<unsigned>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<unsigned>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<unsigned>(c - 'A' + 10);
        return 16;
    }
    std::uint32_t hex_quad(const std::string& path) {
        if (position_ + 4 > input_.size()) fail(path, "truncated Unicode escape");
        std::uint32_t value = 0;
        for (int index = 0; index < 4; ++index) {
            const auto digit = hex_digit(input_[position_++]);
            if (digit == 16) fail(path, "invalid Unicode escape");
            value = value * 16 + digit;
        }
        return value;
    }
    static void append_utf8(std::string& output, std::uint32_t codepoint) {
        if (codepoint <= 0x7f) output.push_back(static_cast<char>(codepoint));
        else if (codepoint <= 0x7ff) {
            output.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else if (codepoint <= 0xffff) {
            output.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else {
            output.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
            output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            output.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
    }
    std::string string(const std::string& path) {
        expect('"', path); std::string result;
        while (!done() && current() != '"') {
            const auto byte = static_cast<unsigned char>(current());
            if (byte < 0x20) fail(path, "unescaped control character in string");
            if (current() != '\\') { result.push_back(current()); ++position_; continue; }
            ++position_;
            if (done()) fail(path, "truncated string escape");
            const char escaped = current(); ++position_;
            switch (escaped) {
                case '"': result.push_back('"'); break; case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break; case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break; case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break; case 't': result.push_back('\t'); break;
                case 'u': {
                    auto codepoint = hex_quad(path);
                    if (codepoint >= 0xd800 && codepoint <= 0xdbff) {
                        if (!consume("\\u")) fail(path, "high surrogate is not followed by a low surrogate");
                        const auto low = hex_quad(path);
                        if (low < 0xdc00 || low > 0xdfff) fail(path, "invalid low surrogate");
                        codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (low - 0xdc00);
                    } else if (codepoint >= 0xdc00 && codepoint <= 0xdfff) fail(path, "unpaired low surrogate");
                    append_utf8(result, codepoint); break;
                }
                default: fail(path, "unsupported string escape");
            }
        }
        if (done()) fail(path, "unterminated string");
        ++position_; return result;
    }
    Number number(const std::string& path) {
        const auto begin = position_;
        if (current() == '-') ++position_;
        if (current() == '0') ++position_;
        else {
            if (current() < '1' || current() > '9') fail(path, "invalid number");
            while (current() >= '0' && current() <= '9') ++position_;
        }
        bool integral = true;
        if (current() == '.') {
            integral = false; ++position_;
            if (current() < '0' || current() > '9') fail(path, "fraction has no digits");
            while (current() >= '0' && current() <= '9') ++position_;
        }
        if (current() == 'e' || current() == 'E') {
            integral = false; ++position_;
            if (current() == '+' || current() == '-') ++position_;
            if (current() < '0' || current() > '9') fail(path, "exponent has no digits");
            while (current() >= '0' && current() <= '9') ++position_;
        }
        const auto token = input_.substr(begin, position_ - begin);
        if (integral) {
            Integer result = 0;
            const auto conversion = std::from_chars(token.data(), token.data() + token.size(), result);
            if (conversion.ec != std::errc{} || conversion.ptr != token.data() + token.size()) fail(path, "integer is outside signed 64-bit range");
            return result;
        }
        std::string owned(token);
        char* end = nullptr;
        const auto result = std::strtod(owned.c_str(), &end);
        if (end != owned.c_str() + owned.size() || !std::isfinite(result)) fail(path, "invalid or non-finite number");
        return result;
    }
};

inline Value parse(std::string_view input) { return Parser(input).parse(); }

inline const Object& object(const Value& value, std::string_view path = "$") {
    if (const auto* result = std::get_if<Object>(&value)) return *result;
    throw Error(std::string(path), "expected object");
}
inline const Array& array(const Value& value, std::string_view path) {
    if (const auto* result = std::get_if<Array>(&value)) return *result;
    throw Error(std::string(path), "expected array");
}
inline const Value& required(const Object& value, std::string_view key, std::string_view path = "$") {
    const auto found = value.find(key);
    if (found == value.end()) throw Error(std::string(path) + "." + std::string(key), "required field is missing");
    return found->second;
}
inline const Value* optional(const Object& value, std::string_view key) {
    const auto found = value.find(key); return found == value.end() ? nullptr : &found->second;
}
inline const std::string& string(const Value& value, std::string_view path) {
    if (const auto* result = std::get_if<std::string>(&value)) return *result;
    throw Error(std::string(path), "expected string");
}
inline Integer integer(const Value& value, std::string_view path) {
    if (const auto* number = std::get_if<Number>(&value))
        if (const auto* result = std::get_if<Integer>(number)) return *result;
    throw Error(std::string(path), "expected signed 64-bit integer");
}
inline double number(const Value& value, std::string_view path) {
    if (const auto* item = std::get_if<Number>(&value)) {
        if (const auto* integer_value = std::get_if<Integer>(item)) return static_cast<double>(*integer_value);
        return std::get<double>(*item);
    }
    throw Error(std::string(path), "expected number");
}
inline bool boolean(const Value& value, std::string_view path) {
    if (const auto* result = std::get_if<bool>(&value)) return *result;
    throw Error(std::string(path), "expected boolean");
}

inline void emit_string(std::ostringstream& output, std::string_view value) {
    output << '"';
    static constexpr char hex[] = "0123456789abcdef";
    for (const auto byte : value) {
        const auto c = static_cast<unsigned char>(byte);
        switch (c) {
            case '"': output << "\\\""; break; case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break; case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break; case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (c < 0x20) output << "\\u00" << hex[c >> 4] << hex[c & 0x0f];
                else output << byte;
        }
    }
    output << '"';
}
inline void emit(std::ostringstream& output, const Value& value) {
    if (std::holds_alternative<std::nullptr_t>(value)) output << "null";
    else if (const auto* item = std::get_if<bool>(&value)) output << (*item ? "true" : "false");
    else if (const auto* item = std::get_if<Number>(&value)) {
        if (const auto* integer_value = std::get_if<Integer>(item)) output << *integer_value;
        else {
            std::ostringstream number; number << std::setprecision(std::numeric_limits<double>::max_digits10) << std::get<double>(*item);
            output << number.str();
        }
    } else if (const auto* item = std::get_if<std::string>(&value)) emit_string(output, *item);
    else if (const auto* items = std::get_if<Array>(&value)) {
        output << '['; bool first = true;
        for (const auto& item : *items) { if (!first) output << ','; first = false; emit(output, item); }
        output << ']';
    } else {
        output << '{'; bool first = true;
        for (const auto& [key, item] : std::get<Object>(value)) {
            if (!first) output << ',';
            first = false;
            emit_string(output, key); output << ':'; emit(output, item);
        }
        output << '}';
    }
}
inline std::string serialize(const Value& value) { std::ostringstream output; emit(output, value); return output.str(); }

} // namespace flowcontracts::json

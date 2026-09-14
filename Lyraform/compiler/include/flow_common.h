#ifndef FLOW_COMMON_H
#define FLOW_COMMON_H

#include <exception>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace flow {

enum class Severity {
    Info,
    Warning,
    Error,
    Fatal
};

struct Diagnostic {
    Severity severity;
    std::string stage;
    std::string message;
};

class DiagnosticError : public std::exception {
public:
    DiagnosticError(std::string stage, std::string message);

    [[nodiscard]] const char* what() const noexcept override;
    [[nodiscard]] const std::string& stage() const noexcept;
    [[nodiscard]] const char* code() const noexcept;

private:
    std::string stage_;
    std::string message_;
};

using PolicyValue = std::variant<bool, int, std::string>;

class PolicyBag {
public:
    void set(std::string key, PolicyValue value);

    [[nodiscard]] bool getBoolOrDefault(
        const std::string& key,
        bool fallback,
        std::vector<Diagnostic>& diagnostics,
        const std::string& stage
    ) const;

    [[nodiscard]] int getIntOrDefault(
        const std::string& key,
        int fallback,
        std::vector<Diagnostic>& diagnostics,
        const std::string& stage
    ) const;

    [[nodiscard]] std::string getStringOrDefault(
        const std::string& key,
        std::string fallback,
        std::vector<Diagnostic>& diagnostics,
        const std::string& stage
    ) const;

private:
    std::map<std::string, PolicyValue> values_;
};

struct PipelineContext {
    PolicyBag policies;
    std::vector<Diagnostic> diagnostics;
};

struct ByteBuffer {
    std::string bytes;
};

template <typename Payload>
struct Envelope {
    Payload payload;
    PipelineContext* ctx = nullptr;
    // Graph routing metadata is carried separately from the payload. Providers
    // may inspect the destination port and stable wire identity without
    // polluting the semantic value being transported.
    std::string input_port;
    std::string wire_id;
    // One output activation creates one signal identity; fan-out deliveries
    // retain it while each destination keeps its own wire identity.
    std::string signal_id;
    // A delivery is distinct even when fan-out shares one output signal.
    std::string delivery_id{};
};

class StdinProducer {
public:
    explicit StdinProducer(PipelineContext& ctx);

    [[nodiscard]] Envelope<ByteBuffer> produce() const;

private:
    PipelineContext* ctx_;
};

class StdoutSink {
public:
    explicit StdoutSink(std::ostream& out);

    void run(const Envelope<ByteBuffer> &env) const;

private:
    std::ostream* out_;
};

class LogSink {
public:
    explicit LogSink(std::ostream& err);

    void write(const PipelineContext& ctx) const;
    void writeFatal(const DiagnosticError& err) const;

private:
    [[nodiscard]] static const char* severityName(Severity severity);

    std::ostream* err_;
};

[[nodiscard]] bool parseBool(const std::string& value);
[[nodiscard]] int parseInt(const std::string& value);

std::string requireArgValue(int argc, char** argv, int& index, const std::string& option);

// Pipeline operators. Kept header-only because they are templates.
template <typename Producer, typename Stage>
auto operator>>(Producer& producer, const Stage& stage)
    -> decltype(stage.run(producer.produce())) {
    return stage.run(producer.produce());
}

template <typename Payload, typename Stage>
auto operator>>(Envelope<Payload> env, const Stage& stage)
    -> decltype(stage.run(std::move(env))) {
    return stage.run(std::move(env));
}

inline void operator>>(Envelope<ByteBuffer> env, const StdoutSink& sink) {
    sink.run(std::move(env));
}

} // namespace flow

#endif // FLOW_COMMON_H

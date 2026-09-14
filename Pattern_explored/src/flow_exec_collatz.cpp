// flow_exec_collatz.cpp
//
// Flow Policy Envelope Pattern - Routed execution graph proof-of-concept
//
// Build:
//   g++ -std=c++17 -Wall -Wextra -pedantic flow_exec_collatz.cpp -o flowexec
//
// Usage:
//   echo 27 | ./flowexec
//   echo 27 | ./flowexec --max-steps 10000
//   echo 27 | ./flowexec --trace true
//
// Purpose:
//   Demonstrates Producer -> Routed Nodes -> Sink
//   with named transitions, static graph wires, policy, diagnostics,
//   loop execution, and real computation.
//
// Model:
//   Producer creates Envelope<TextPayload>
//   ParseUInt        : TextPayload    -> CollatzPayload
//   CheckDone        : CollatzPayload -> route done|continue
//   ParityBranch     : CollatzPayload -> route even|odd
//   EvenStep/OddStep : CollatzPayload -> CollatzPayload
//   FormatOutput     : CollatzPayload -> OutputPayload
//   StdoutSink       : consumes OutputPayload
//
// Static graph:
//
//   stdin.out      -> parse.in
//   parse.ok       -> check.in
//   check.continue -> parity.in
//   check.done     -> format.in
//   parity.even    -> even.in
//   parity.odd     -> odd.in
//   even.out       -> check.in
//   odd.out        -> check.in
//   format.out     -> stdout.in
//
// The loop is explicit:
//   even.out -> check.in
//   odd.out  -> check.in

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// ============================================================
// Diagnostics
// ============================================================

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
    DiagnosticError(std::string stage, std::string message)
        : stage_(std::move(stage)),
          message_(std::move(message)) {}

    [[nodiscard]] const char* what() const noexcept override {
        return message_.c_str();
    }

    [[nodiscard]] const std::string& stage() const noexcept {
        return stage_;
    }

private:
    std::string stage_;
    std::string message_;
};

// ============================================================
// Policy
// ============================================================

using PolicyValue = std::variant<bool, int, std::string>;

class PolicyBag {
public:
    void set(std::string key, PolicyValue value) {
        values_[std::move(key)] = std::move(value);
    }

    [[nodiscard]] bool getBoolOrDefault(
        const std::string& key,
        bool fallback,
        std::vector<Diagnostic>& diagnostics,
        const std::string& stage
    ) const {
        const auto it = values_.find(key);

        if (it == values_.end()) {
            return fallback;
        }

        if (const auto* value = std::get_if<bool>(&it->second)) {
            return *value;
        }

        diagnostics.push_back({
            Severity::Warning,
            stage,
            "policy '" + key + "' expected bool; using fallback"
        });

        return fallback;
    }

    [[nodiscard]] int getIntOrDefault(
        const std::string& key,
        int fallback,
        std::vector<Diagnostic>& diagnostics,
        const std::string& stage
    ) const {
        const auto it = values_.find(key);

        if (it == values_.end()) {
            return fallback;
        }

        if (const auto* value = std::get_if<int>(&it->second)) {
            return *value;
        }

        diagnostics.push_back({
            Severity::Warning,
            stage,
            "policy '" + key + "' expected int; using fallback"
        });

        return fallback;
    }

private:
    std::map<std::string, PolicyValue> values_;
};

// ============================================================
// Context + Payloads + Envelope
// ============================================================

struct PipelineContext {
    PolicyBag policies;
    std::vector<Diagnostic> diagnostics;
};

struct TextPayload {
    std::string text;
};

struct CollatzPayload {
    std::uint64_t n = 0;
    std::uint64_t steps = 0;
    std::vector<std::uint64_t> sequence;
};

struct OutputPayload {
    std::string text;
};

using Payload = std::variant<TextPayload, CollatzPayload, OutputPayload>;

struct Envelope {
    Payload payload;
    PipelineContext* ctx = nullptr;
};

template <typename T>
T& requirePayload(Envelope& env, const std::string& stage) {
    if (auto* value = std::get_if<T>(&env.payload)) {
        return *value;
    }

    throw DiagnosticError{
        stage,
        "payload contract mismatch"
    };
}

template <typename T>
const T& requirePayloadConst(const Envelope& env, const std::string& stage) {
    if (const auto* value = std::get_if<T>(&env.payload)) {
        return *value;
    }

    throw DiagnosticError{
        stage,
        "payload contract mismatch"
    };
}

// ============================================================
// Routing
// ============================================================

struct Route {
    std::string port;
    Envelope envelope;
};

class Node {
public:
    virtual ~Node() = default;

    [[nodiscard]] virtual std::vector<Route> run(Envelope env) = 0;
};

class StdinProducer {
public:
    explicit StdinProducer(PipelineContext& ctx)
        : ctx_(&ctx) {}

    [[nodiscard]] Envelope produce() const {
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();

        return Envelope{
            TextPayload{buffer.str()},
            ctx_
        };
    }

private:
    PipelineContext* ctx_;
};

// ============================================================
// Nodes
// ============================================================

class ParseUIntNode final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        auto& text = requirePayload<TextPayload>(env, "parse");

        const std::string trimmed = trim(text.text);

        if (trimmed.empty()) {
            throw DiagnosticError{"parse", "empty input"};
        }

        std::size_t pos = 0;
        unsigned long long value = 0;

        try {
            value = std::stoull(trimmed, &pos);
        } catch (const std::exception&) {
            throw DiagnosticError{"parse", "expected unsigned integer input"};
        }

        if (pos != trimmed.size()) {
            throw DiagnosticError{"parse", "input contains non-integer trailing data"};
        }

        if (value == 0) {
            throw DiagnosticError{"parse", "Collatz input must be greater than zero"};
        }

        CollatzPayload payload;
        payload.n = static_cast<std::uint64_t>(value);
        payload.steps = 0;
        payload.sequence.push_back(payload.n);

        return {
            Route{
                "ok",
                Envelope{std::move(payload), env.ctx}
            }
        };
    }

private:
    [[nodiscard]] static std::string trim(const std::string& input) {
        std::size_t first = 0;

        while (first < input.size() &&
               std::isspace(static_cast<unsigned char>(input[first]))) {
            ++first;
        }

        std::size_t last = input.size();

        while (last > first &&
               std::isspace(static_cast<unsigned char>(input[last - 1]))) {
            --last;
        }

        return input.substr(first, last - first);
    }
};

class CheckDoneNode final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        if (env.ctx == nullptr) {
            throw DiagnosticError{"check", "missing pipeline context"};
        }

        auto& payload = requirePayload<CollatzPayload>(env, "check");

        const int maxSteps =
            env.ctx->policies.getIntOrDefault(
                "collatz.max_steps",
                10000,
                env.ctx->diagnostics,
                "check"
            );

        if (payload.n == 1) {
            return {
                Route{"done", std::move(env)}
            };
        }

        if (maxSteps >= 0 &&
            payload.steps >= static_cast<std::uint64_t>(maxSteps)) {
            throw DiagnosticError{
                "check",
                "maximum Collatz steps reached"
            };
        }

        return {
            Route{"continue", std::move(env)}
        };
    }
};

class ParityBranchNode final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        const auto& payload = requirePayloadConst<CollatzPayload>(env, "parity");

        if ((payload.n % 2U) == 0U) {
            return {
                Route{"even", std::move(env)}
            };
        }

        return {
            Route{"odd", std::move(env)}
        };
    }
};

class EvenStepNode final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        auto& payload = requirePayload<CollatzPayload>(env, "even");

        payload.n /= 2U;
        ++payload.steps;
        payload.sequence.push_back(payload.n);

        return {
            Route{"out", std::move(env)}
        };
    }
};

class OddStepNode final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        if (env.ctx == nullptr) {
            throw DiagnosticError{"odd", "missing pipeline context"};
        }

        auto& payload = requirePayload<CollatzPayload>(env, "odd");

        const bool stopOnOverflow =
            env.ctx->policies.getBoolOrDefault(
                "collatz.stop_on_overflow",
                true,
                env.ctx->diagnostics,
                "odd"
            );

        const std::uint64_t max = std::numeric_limits<std::uint64_t>::max();

        if (payload.n > (max - 1U) / 3U) {
            if (stopOnOverflow) {
                throw DiagnosticError{
                    "odd",
                    "3n + 1 would overflow uint64_t"
                };
            }

            env.ctx->diagnostics.push_back({
                Severity::Warning,
                "odd",
                "overflow would occur; stopping sequence at current value"
            });

            payload.n = 1;
            payload.sequence.push_back(payload.n);

            return {
                Route{"out", std::move(env)}
            };
        }

        payload.n = (payload.n * 3U) + 1U;
        ++payload.steps;
        payload.sequence.push_back(payload.n);

        return {
            Route{"out", std::move(env)}
        };
    }
};

class FormatOutputNode final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        if (env.ctx == nullptr) {
            throw DiagnosticError{"format", "missing pipeline context"};
        }

        const auto& payload = requirePayloadConst<CollatzPayload>(env, "format");

        const bool includeSteps =
            env.ctx->policies.getBoolOrDefault(
                "format.include_steps",
                true,
                env.ctx->diagnostics,
                "format"
            );

        std::ostringstream out;

        if (includeSteps) {
            out << "steps: " << payload.steps << '\n';
        }

        out << "sequence:\n";

        for (std::size_t i = 0; i < payload.sequence.size(); ++i) {
            if (i > 0) {
                out << " -> ";
            }

            out << payload.sequence[i];
        }

        out << '\n';

        return {
            Route{
                "out",
                Envelope{OutputPayload{out.str()}, env.ctx}
            }
        };
    }
};

class StdoutSink final : public Node {
public:
    [[nodiscard]] std::vector<Route> run(Envelope env) override {
        const auto& payload = requirePayloadConst<OutputPayload>(env, "stdout");

        std::cout << payload.text;

        return {};
    }
};

// ============================================================
// Runtime graph
// ============================================================

class RuntimeGraph {
public:
    void addNode(std::string id, std::unique_ptr<Node> node) {
        nodes_[std::move(id)] = std::move(node);
    }

    void connect(std::string fromNode, std::string fromPort, std::string toNode) {
        wires_[wireKey(fromNode, fromPort)] = std::move(toNode);
    }

    void startFromProducer(
        const std::string& producerId,
        const std::string& producerPort,
        Envelope env
    ) {
        deliver(producerId, producerPort, std::move(env));

        while (!queue_.empty()) {
            Pending pending = std::move(queue_.front());
            queue_.pop();

            auto it = nodes_.find(pending.nodeId);

            if (it == nodes_.end()) {
                throw DiagnosticError{
                    "runtime",
                    "unknown node: " + pending.nodeId
                };
            }

            trace("runtime", "enter node '" + pending.nodeId + "'", pending.env);

            std::vector<Route> routes =
                it->second->run(std::move(pending.env));

            for (auto& route : routes) {
                deliver(pending.nodeId, route.port, std::move(route.envelope));
            }
        }
    }

private:
    struct Pending {
        std::string nodeId;
        Envelope env;
    };

    [[nodiscard]] static std::string wireKey(
        const std::string& node,
        const std::string& port
    ) {
        return node + "." + port;
    }

    void deliver(
        const std::string& fromNode,
        const std::string& fromPort,
        Envelope env
    ) {
        if (env.ctx == nullptr) {
            throw DiagnosticError{"runtime", "missing pipeline context"};
        }

        const auto key = wireKey(fromNode, fromPort);
        const auto it = wires_.find(key);

        if (it == wires_.end()) {
            env.ctx->diagnostics.push_back({
                Severity::Warning,
                "runtime",
                "no wire connected from " + key + "; dropping envelope"
            });

            return;
        }

        trace(
            "runtime",
            "route " + key + " -> " + it->second + ".in",
            env
        );

        queue_.push(Pending{it->second, std::move(env)});
    }

    static void trace(
        const std::string& stage,
        const std::string& message,
        Envelope& env
    ) {
        if (env.ctx == nullptr) {
            return;
        }

        const bool enabled =
            env.ctx->policies.getBoolOrDefault(
                "runtime.trace",
                false,
                env.ctx->diagnostics,
                stage
            );

        if (!enabled) {
            return;
        }

        env.ctx->diagnostics.push_back({
            Severity::Info,
            stage,
            message
        });
    }

    std::map<std::string, std::unique_ptr<Node>> nodes_;
    std::map<std::string, std::string> wires_;
    std::queue<Pending> queue_;
};

// ============================================================
// Logging
// ============================================================

class LogSink {
public:
    explicit LogSink(std::ostream& err)
        : err_(&err) {}

    void write(const PipelineContext& ctx) const {
        for (const auto& diagnostic : ctx.diagnostics) {
            *err_
                << severityName(diagnostic.severity)
                << " in "
                << diagnostic.stage
                << ": "
                << diagnostic.message
                << '\n';
        }
    }

    void writeFatal(const DiagnosticError& err) const {
        *err_
            << "fatal in "
            << err.stage()
            << ": "
            << err.what()
            << '\n';
    }

private:
    [[nodiscard]] static const char* severityName(Severity severity) {
        switch (severity) {
            case Severity::Info:    return "info";
            case Severity::Warning: return "warning";
            case Severity::Error:   return "error";
            case Severity::Fatal:   return "fatal";
        }

        return "unknown";
    }

    std::ostream* err_;
};

// ============================================================
// CLI policy loading
// ============================================================

bool parseBool(const std::string& value) {
    if (value == "true" || value == "1" || value == "yes" || value == "on") {
        return true;
    }

    if (value == "false" || value == "0" || value == "no" || value == "off") {
        return false;
    }

    throw DiagnosticError{"cli", "expected bool, got: " + value};
}

int parseInt(const std::string& value) {
    try {
        std::size_t pos = 0;
        const int result = std::stoi(value, &pos);

        if (pos != value.size()) {
            throw DiagnosticError{"cli", "expected integer, got: " + value};
        }

        return result;
    } catch (const std::invalid_argument&) {
        throw DiagnosticError{"cli", "expected integer, got: " + value};
    } catch (const std::out_of_range&) {
        throw DiagnosticError{"cli", "integer out of range: " + value};
    }
}

void loadPoliciesFromArgs(int argc, char** argv, PolicyBag& policies) {
    policies.set("collatz.max_steps", 10000);
    policies.set("collatz.stop_on_overflow", true);
    policies.set("format.include_steps", true);
    policies.set("runtime.trace", false);

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto requireValue = [&](const std::string& option) -> std::string {
            if (i + 1 >= argc) {
                throw DiagnosticError{"cli", "missing value after " + option};
            }

            ++i;
            return argv[i];
        };

        if (arg == "--max-steps") {
            policies.set("collatz.max_steps", parseInt(requireValue(arg)));
        } else if (arg == "--stop-on-overflow") {
            policies.set("collatz.stop_on_overflow", parseBool(requireValue(arg)));
        } else if (arg == "--include-steps") {
            policies.set("format.include_steps", parseBool(requireValue(arg)));
        } else if (arg == "--trace") {
            policies.set("runtime.trace", parseBool(requireValue(arg)));
        } else if (arg == "--help" || arg == "-h") {
            std::cout
                << "Usage:\n"
                << "  flowexec [options] < input\n\n"
                << "Options:\n"
                << "  --max-steps N\n"
                << "  --stop-on-overflow true|false\n"
                << "  --include-steps true|false\n"
                << "  --trace true|false\n\n"
                << "Examples:\n"
                << "  echo 27 | ./flowexec\n"
                << "  echo 27 | ./flowexec --trace true\n";
            std::exit(EXIT_SUCCESS);
        } else {
            throw DiagnosticError{"cli", "unknown argument: " + arg};
        }
    }
}

// ============================================================
// Orchestration
// ============================================================

int main(int argc, char** argv) {
    PipelineContext ctx;
    LogSink log(std::cerr);

    try {
        loadPoliciesFromArgs(argc, argv, ctx.policies);

        StdinProducer stdin(ctx);

        RuntimeGraph graph;

        graph.addNode("parse", std::make_unique<ParseUIntNode>());
        graph.addNode("check", std::make_unique<CheckDoneNode>());
        graph.addNode("parity", std::make_unique<ParityBranchNode>());
        graph.addNode("even", std::make_unique<EvenStepNode>());
        graph.addNode("odd", std::make_unique<OddStepNode>());
        graph.addNode("format", std::make_unique<FormatOutputNode>());
        graph.addNode("stdout", std::make_unique<StdoutSink>());

        // Static wires / roads.
        graph.connect("stdin", "out", "parse");
        graph.connect("parse", "ok", "check");
        graph.connect("check", "continue", "parity");
        graph.connect("check", "done", "format");
        graph.connect("parity", "even", "even");
        graph.connect("parity", "odd", "odd");

        // Explicit loop edges.
        graph.connect("even", "out", "check");
        graph.connect("odd", "out", "check");

        graph.connect("format", "out", "stdout");

        graph.startFromProducer("stdin", "out", stdin.produce());

        log.write(ctx);
    } catch (const DiagnosticError& err) {
        log.writeFatal(err);
        log.write(ctx);
        return EXIT_FAILURE;
    } catch (const std::exception& err) {
        std::cerr << "fatal internal error: " << err.what() << '\n';
        log.write(ctx);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
};

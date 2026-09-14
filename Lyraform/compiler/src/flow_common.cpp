#include "flow_common.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace flow {

DiagnosticError::DiagnosticError(std::string stage, std::string message)
    : stage_(std::move(stage)),
      message_(std::move(message)) {}

const char* DiagnosticError::what() const noexcept {
    return message_.c_str();
}

const std::string& DiagnosticError::stage() const noexcept {
    return stage_;
}

const char* DiagnosticError::code() const noexcept {
    return "FLOW_DIAGNOSTIC_ERROR";
}

void PolicyBag::set(std::string key, PolicyValue value) {
    values_[std::move(key)] = std::move(value);
}

bool PolicyBag::getBoolOrDefault(
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

int PolicyBag::getIntOrDefault(
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

std::string PolicyBag::getStringOrDefault(
    const std::string& key,
    std::string fallback,
    std::vector<Diagnostic>& diagnostics,
    const std::string& stage
) const {
    const auto it = values_.find(key);

    if (it == values_.end()) {
        return fallback;
    }

    if (const auto* value = std::get_if<std::string>(&it->second)) {
        return *value;
    }

    diagnostics.push_back({
        Severity::Warning,
        stage,
        "policy '" + key + "' expected string; using fallback"
    });

    return fallback;
}

StdinProducer::StdinProducer(PipelineContext& ctx)
    : ctx_(&ctx) {}

Envelope<ByteBuffer> StdinProducer::produce() const {
    std::ostringstream buffer;
    buffer << std::cin.rdbuf();

    return Envelope<ByteBuffer>{
        ByteBuffer{buffer.str()},
        ctx_,
        {},
        {},
        {}
    };
}

StdoutSink::StdoutSink(std::ostream& out)
    : out_(&out) {}

void StdoutSink::run(const Envelope<ByteBuffer> &env) const {
    if (out_ == nullptr) {
        throw DiagnosticError{"stdout-sink", "missing output stream"};
    }

    out_->write(
        env.payload.bytes.data(),
        static_cast<std::streamsize>(env.payload.bytes.size())
    );
}

LogSink::LogSink(std::ostream& err)
    : err_(&err) {}

void LogSink::write(const PipelineContext& ctx) const {
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

void LogSink::writeFatal(const DiagnosticError& err) const {
    *err_
        << "fatal in "
        << err.stage()
        << ": "
        << err.what()
        << '\n';
}

const char* LogSink::severityName(Severity severity) {
    switch (severity) {
        case Severity::Info:    return "info";
        case Severity::Warning: return "warning";
        case Severity::Error:   return "error";
        case Severity::Fatal:   return "fatal";
    }

    return "unknown";
}

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

std::string requireArgValue(int argc, char** argv, int& index, const std::string& option) {
    if (index + 1 >= argc) {
        throw DiagnosticError{"cli", "missing value after " + option};
    }

    ++index;
    return argv[index];
}

} // namespace flow

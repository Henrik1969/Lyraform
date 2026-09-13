#ifndef FLOWMINI_SCHEMA_H
#define FLOWMINI_SCHEMA_H

#include "flow_common.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace flowmini {

struct Endpoint {
    std::string node;
    std::string port;
};

struct NodeDecl {
    std::string role; // producer, node, sink
    std::string id;
    std::string kind;
    bool source_function = false;
    bool persistent = false;
};

struct WireDecl {
    Endpoint from;
    Endpoint to;
};

struct PolicyDecl {
    std::string node;
    std::string key;
    flow::PolicyValue value;
};

struct ReceiverFrame;

struct ModuleSpec {
    std::string name;
    std::vector<NodeDecl> nodes;
    std::vector<WireDecl> wires;
    std::vector<PolicyDecl> policies;
    std::map<std::string, std::shared_ptr<const ReceiverFrame>> receivers;
};

// Compatibility interpreter projection of a source function. Each delivery
// instantiates this body with a fresh record; it is never an atom factory name.
struct ReceiverFrame {
    ModuleSpec body;
    Endpoint entry;
    Endpoint result;
    std::string input_type;
    std::string output_type;
    std::string input_path;
    std::string result_path;
};

struct AtomContract {
    std::string kind;
    std::map<std::string, std::string> inputs;
    std::map<std::string, std::string> outputs;
    std::vector<std::string> effects;
    std::set<std::string> optional_inputs;
    bool terminal = false;

    AtomContract() = default;
    AtomContract(std::string atom_kind,
                 std::map<std::string, std::string> atom_inputs,
                 std::map<std::string, std::string> atom_outputs,
                 std::vector<std::string> atom_effects,
                 std::set<std::string> atom_optional_inputs = {},
                 bool atom_terminal = false)
        : kind(std::move(atom_kind)), inputs(std::move(atom_inputs)), outputs(std::move(atom_outputs)),
          effects(std::move(atom_effects)), optional_inputs(std::move(atom_optional_inputs)), terminal(atom_terminal) {}
};

using NodePolicyMap = std::map<std::string, flow::PolicyValue>;

struct NodeConfig {
    std::string id;
    std::string kind;
    NodePolicyMap policies;
};

} // namespace flowmini

#endif // FLOWMINI_SCHEMA_H

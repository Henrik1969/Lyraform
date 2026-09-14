#ifndef FLOWMINI_RUNTIME_H
#define FLOWMINI_RUNTIME_H

#include "flowmini_payload.h"
#include "flowmini_schema.h"

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

namespace flowmini {

struct Route {
    std::string port;
    MiniEnvelope envelope;
};

class INode {
public:
    virtual ~INode() = default;
    [[nodiscard]] virtual std::vector<Route> run(MiniEnvelope env) = 0;
};

class AtomRegistry {
public:
    using Factory = std::function<std::unique_ptr<INode>(NodeConfig)>;

    void registerAtom(AtomContract contract, Factory factory);

    [[nodiscard]] const AtomContract& contractFor(const std::string& kind) const;
    [[nodiscard]] std::unique_ptr<INode> create(NodeConfig config) const;
    [[nodiscard]] bool contains(const std::string& kind) const;

private:
    std::map<std::string, AtomContract> contracts_;
    std::map<std::string, Factory> factories_;
};

[[nodiscard]] AtomRegistry makeCoreAtomRegistry();

class RuntimeGraph {
public:
    void addNode(std::string id, std::unique_ptr<INode> node);
    void connect(std::string fromNode, std::string fromPort, std::string toNode, std::string toPort, std::string wireId);

    void startAt(const std::string& nodeId, MiniEnvelope env);
    void setIdentityScope(std::string scope) { identity_scope_ = std::move(scope); }

private:
    struct Pending {
        std::string nodeId;
        MiniEnvelope envelope;
    };

    [[nodiscard]] static std::string wireKey(const std::string& node, const std::string& port);

    struct Connection {
        std::string node;
        std::string port;
        std::string wire_id;
    };

    void deliver(const std::string& fromNode, const std::string& fromPort, MiniEnvelope env);
    void trace(const std::string& message, MiniEnvelope& env) const;

    std::map<std::string, std::unique_ptr<INode>> nodes_;
    std::map<std::string, std::vector<Connection>> wires_;
    std::queue<Pending> queue_;
    std::size_t next_signal_id_ = 0;
    std::size_t next_delivery_id_ = 0;
    std::string identity_scope_;
};

struct BuildResult {
    RuntimeGraph graph;
    std::vector<std::string> producerIds;
};

struct RuntimeResult {
    bool completed = false;
    std::string code;
    std::string stage;
    std::string message;
};

[[nodiscard]] BuildResult buildCheckedGraph(const ModuleSpec& module, const AtomRegistry& registry);

void runModule(const ModuleSpec& module, flow::PipelineContext& ctx, const AtomRegistry& registry);
[[nodiscard]] RuntimeResult runModuleChecked(
    const ModuleSpec& module,
    flow::PipelineContext& ctx,
    const AtomRegistry& registry
);

} // namespace flowmini

#endif // FLOWMINI_RUNTIME_H

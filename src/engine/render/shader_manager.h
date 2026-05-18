#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

namespace mmm {

struct ShaderSource {
    std::string vert_path;
    std::string frag_path;
};

// Manages shader program lifecycles and bindings.
// In headless/test builds the map still loads and lookups work; actual
// GPU compilation is no-op when no backend is active.
class ShaderManager {
public:
    // Register a named shader; actual GPU upload deferred to bind()
    void load(const std::string& name, ShaderSource src) {
        sources_[name] = std::move(src);
    }

    // Returns internal ID (or 0 for null renderer)
    uint32_t bind(const std::string& name) {
        auto it = sources_.find(name);
        if (it == sources_.end()) return 0;
        auto id_it = ids_.find(name);
        if (id_it != ids_.end()) return id_it->second;
        uint32_t id = ++next_id_;
        ids_[name]  = id;
        return id;
    }

    bool has(const std::string& name) const {
        return sources_.count(name) > 0;
    }

    void clear() { sources_.clear(); ids_.clear(); next_id_ = 0; }

private:
    std::unordered_map<std::string, ShaderSource> sources_;
    std::unordered_map<std::string, uint32_t>     ids_;
    uint32_t next_id_{0};
};

} // namespace mmm

#pragma once

#include <map>
#include <string>
#include <optional>
#include <vector>

namespace engine {

template<typename T>
class Registry {
public:
    Registry() = default;

    // Register an item with an ID
    void register_item(const std::string& id, const T& item) {
        items_[id] = item;
    }

    // Get item by ID
    [[nodiscard]] std::optional<T> get(const std::string& id) const {
        auto it = items_.find(id);
        if (it != items_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Check if item exists
    [[nodiscard]] bool has(const std::string& id) const {
        return items_.find(id) != items_.end();
    }

    // Clear all items
    void clear() {
        items_.clear();
    }

    // Get count of registered items
    [[nodiscard]] size_t size() const {
        return items_.size();
    }

    // Get all registered IDs
    [[nodiscard]] std::vector<std::string> get_all_ids() const {
        std::vector<std::string> ids;
        ids.reserve(items_.size());
        for (const auto& [id, _] : items_) {
            ids.push_back(id);
        }
        return ids;
    }

private:
    std::map<std::string, T> items_;
};

} // namespace engine

#pragma once

#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

class AnalysisResults final {
public:
    AnalysisResults() = default;

    template <typename T>
    void set(std::string key, T value) {
        values_.insert_or_assign(std::move(key), std::any(std::move(value)));
    }

    bool has(const std::string& key) const { return values_.contains(key); }

    template <typename T>
    const T& get(const std::string& key) const {
        const auto it = values_.find(key);
        if (it == values_.end()) {
            throw std::out_of_range("AnalysisResults missing key: " + key);
        }
        return std::any_cast<const T&>(it->second);
    }

private:
    std::unordered_map<std::string, std::any> values_;
};


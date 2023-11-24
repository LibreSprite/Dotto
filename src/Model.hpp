#pragma once

#include <memory>
#include <variant>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <vector>

class Model;
enum class Undefined {};
using Value = std::variant<Undefined, float, std::string, std::shared_ptr<Model>>;

class Model {
    std::shared_mutex mutex;
public:
    std::unordered_map<std::string, Value> values;

    void print(std::vector<std::string>* section = nullptr);

    void set(const std::string& key, const Value& value);
    const Value& get(const std::string& key);
    void parse(const std::string& iniFile);

    template <typename T>
    T get(const std::string& key, T def) {
        auto& v = get(key);
        if constexpr (std::is_same_v<T, const char*>) {
            if (auto t = std::get_if<std::string>(&v))
                return t->c_str();
        } else {
            if (auto t = std::get_if<T>(&v))
                return *t;
        }
        return def;
    }

    static Model root;
};

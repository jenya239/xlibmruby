#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <typeindex>
#include <any>
#include <stdexcept>
#include <string>

class Container {
public:
    template<typename T>
    void register_singleton(std::function<std::shared_ptr<T>()> factory) {
        services[std::type_index(typeid(T))] = factory;
    }

    template<typename T>
    std::shared_ptr<T> resolve() {
        auto it = services.find(std::type_index(typeid(T)));
        if (it != services.end()) {
            return std::any_cast<std::shared_ptr<T>>(it->second());
        }
        throw std::runtime_error(std::string("Service not registered: ") + typeid(T).name());
    }

private:
    std::unordered_map<std::type_index, std::function<std::any()>> services;
};
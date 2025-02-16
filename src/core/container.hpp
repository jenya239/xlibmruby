#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <stdexcept>

class Container {
public:
    // Регистрация singleton сервиса
    template<typename T>
    void register_singleton(std::function<std::shared_ptr<T>()> factory) {
        std::type_index index(typeid(T));
        factories_[index] = [factory]() -> std::shared_ptr<void> {
            return std::static_pointer_cast<void>(factory());
        };
    }

    // Разрешение сервиса
    template<typename T>
    std::shared_ptr<T> resolve() const {
        std::type_index index(typeid(T));
        auto it = factories_.find(index);
        if (it != factories_.end()) {
            return std::static_pointer_cast<T>(it->second());
        }
        throw std::runtime_error("Service not found");
    }

private:
    mutable std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
};
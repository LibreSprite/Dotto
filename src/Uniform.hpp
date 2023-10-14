#pragma once

#include <cstddef>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

class UniformRef {
protected:
    const void* valuePtr;

public:
    const std::type_info& type;

    template<typename Type>
    UniformRef(const Type& value) : type{typeid(Type)} {
        valuePtr = &value;
    }

    template<typename Type>
    Type* get() {
        return type == typeid(Type) ? reinterpret_cast<Type*>(valuePtr) : nullptr;
    }

    const void* raw() {return valuePtr;}
};

template <typename Type>
class Uniform : public UniformRef {
public:
    Type value;

    Uniform(const Type& value) : UniformRef{this->value}, value{value} {}

    Uniform() : UniformRef{this->value}, value{} {}

    Uniform(const Uniform<Type>& o) : Uniform{o.value} {}

    Type& operator * () {return value;}

    Type& operator = (const Type& v) {
        value = v;
        return *this;
    }
};

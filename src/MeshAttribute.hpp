#pragma once

#include <cstdint>
#include <vector>

#include "Matrix.hpp"
#include "Log.hpp"

class MeshAttribute {
public:
    enum class ElementType {
        Float,
        Int32,
        Int16,
        Int8
    };

    class Cursor {
        std::vector<uint8_t>& backing;
        mutable std::size_t offset;
        std::size_t stride;

    public:
        Cursor(std::vector<uint8_t>& backing, std::size_t offset, std::size_t stride) :
            backing{backing},
            offset{offset},
            stride{stride}
            {}

        template<typename T>
        const Cursor& operator << (const T& v) const {
            for (std::size_t i = 0; i < sizeof(T); ++i) {
                backing[offset + i] = reinterpret_cast<const uint8_t*>(&v)[i];
            }
            offset += stride;
            return *this;
        }
    };

    virtual ~MeshAttribute() = default;
    virtual ElementType type() = 0;
    virtual std::size_t elementSize() = 0;
    virtual std::size_t elementCount() = 0;
    virtual std::size_t length() = 0;
    virtual void write(const Cursor&) = 0;
    virtual void read(const std::vector<uint8_t>&) = 0;
    bool dirty = true;
};

template<typename Type>
class Attribute : public MeshAttribute {
public:
    std::vector<Type> data;

    ElementType type() override {
        if constexpr (std::is_same_v<Type, int32_t>)
            return ElementType::Int32;
        if constexpr (std::is_same_v<Type, int16_t>)
            return ElementType::Int16;
        if constexpr (std::is_same_v<Type, int8_t>)
            return ElementType::Int8;
        return ElementType::Float;
    }

    std::size_t elementCount() override {
        if constexpr (std::is_same_v<Type, Matrix>)
            return 16;
        if constexpr (std::is_same_v<Type, RGBA>)
            return 4;
        if constexpr (std::is_same_v<Type, Vector>)
            return 3;
        if constexpr (std::is_same_v<Type, UV>)
            return 2;
        return 1;
    }

    std::size_t elementSize() override {
        return sizeof(Type);
    }

    std::size_t length() override {
        return data.size();
    }

    void write(const Cursor& out) override {
        for (auto& el : data)
            out << el;
    }

    void read(const std::vector<uint8_t>& in) override {
	auto elementCount = in.size() / sizeof(Type);
	if (elementCount)
	    dirty = true;
	// LOG("Adding ", elementCount);
	data.reserve(data.size() + elementCount);
	for (int i = 0; i < elementCount; ++i) {
	    auto& v = reinterpret_cast<const Type*>(in.data())[i];
	    // LOG(v);
	    data.push_back(v);
	}
    }
};

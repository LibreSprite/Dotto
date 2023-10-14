#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
#include <functional>

#include "Shared.hpp"
#include "MainThread.hpp"

class VMState;

class VM {
    std::shared_ptr<VMState> state;
public:

    class Args {
        VMState* state;
    public:
        VM* vm;

        Args(VMState* state, VM* vm) : state{state}, vm{vm} {}

        template<typename Type>
        Type get (uint32_t i) const {
            if constexpr (std::is_pointer_v<Type>) {
                return reinterpret_cast<Type>(getPtr(i));
	    } else if constexpr (std::is_same_v<Type, std::string>) {
		auto begin = reinterpret_cast<char*>(getPtr(i));
		auto end = begin;
		auto max = reinterpret_cast<char*>(vm->RAM.data() + vm->RAM.size());
		if (end) {
		    while (end < max && *end)
			++end;
		    if (end > max)
			end = begin;
		}
		if (begin == end)
		    return {};
		return {begin, end};
            } else {
		auto v = get(i);
                return *reinterpret_cast<Type*>(&v);
            }
        }

        enum class Void{};
        using Result = std::variant<Void, int32_t, uint32_t, float, bool, std::string>;
        mutable Result result;

        uint32_t get(uint32_t i) const;

        void* getPtr(uint32_t i) const;
    };

    using APIFunc = std::function<void(const Args&)>;
    using APIFuncMap = std::unordered_map<std::string, APIFunc>;

    static APIFuncMap& globalMap() {
        static APIFuncMap map;
        return map;
    }

    class API {
    public:
        API(const APIFuncMap& src) {
            auto& dest = globalMap();
            for (auto& entry : src)
                dest[entry.first] = entry.second;
        }
    };

    std::size_t speed = (32*1024*1024)/30;

    VM() = default;
    VM(const VM&) = delete;
    virtual ~VM() = default;

    void addAPI(const APIFuncMap& src) {
        for (auto& entry : src)
            api[entry.first] = entry.second;
    }

    void boot(const std::vector<std::byte>& image);
    void run();
    std::vector<std::byte> suspend();
    void thaw(const std::vector<std::byte>&);
    void yield();
    bool crashed() const;

    template<typename Type>
    Type* toHost(Type* ptr) {
        return reinterpret_cast<Type*>(toHost(reinterpret_cast<uintptr_t>(ptr), sizeof(Type)));
    }

    void* toHost(uint32_t ptr, std::size_t size) const;

    uint32_t toGuest(const void* data, std::size_t size);
    uint32_t toGuest(const std::string& str) {return toGuest(str.data(), str.size() + 1);}

protected:
    void link(uint32_t);
    friend class VMState;
    std::mutex ramSizeMutex;
    std::vector<APIFunc> apiIndex;
    std::vector<uint8_t> RAM;
    APIFuncMap api;
};

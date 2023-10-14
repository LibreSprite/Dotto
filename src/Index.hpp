#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "Shared.hpp"

template <typename Type>
class Index {
public:
    const std::size_t offset;

    using Optional = std::optional<Type>;

    Index(std::size_t offset) : offset{offset} {_ptr = this;}

    ~Index() {_ptr = nullptr;}

    uint32_t count() {
        return _data.read([&](auto& _data){ return _data.size(); });
    }

    Optional operator [] (uint32_t key) {
	if (key < offset)
	    return Optional{};
        auto vkey = key - offset;
        return _data.read([&](auto& _data){
	    return (vkey >= _data.size()) ? Optional{} : _data[vkey];
	});
    }

    static Optional find(uint32_t key) {
	return _ptr ? (*_ptr)[key] : Optional{};
    }

    uint32_t add(const Type& v) {
	std::size_t max{};
	_data.write([&](auto& _data) {
	    max = _data.size();
	    for (std::size_t i = 0; i < max; ++i) {
		auto& o = _data[i];
		if (!o.has_value()) {
		    o = v;
		    max = i;
		    return;
		}
	    }
	    _data.push_back(v);
	});
        return (max + offset);
    }

    void remove(uint32_t key) {
        auto vkey = key - offset;
	_data.write([&](auto& _data){
	    if (vkey < _data.size())
		_data[vkey].reset();
	});
    }

private:
    friend class AutoIndex;
    Shared<std::vector<Optional>> _data;
    static inline Index<Type>* _ptr;
};

class AutoIndex {
    void (*remove)(uint32_t);
    uint32_t _key;
public:
    template<typename Type>
    AutoIndex(Type* ref) {
        _key = Index<Type*>::_ptr->add(ref);
        remove = +[](uint32_t key){
            Index<Type*>::_ptr->remove(key);
        };
    }

    ~AutoIndex() {
        remove(_key);
    }

    uint32_t operator* () {
        return _key;
    }
};

inline Shared<std::vector<std::shared_ptr<void>>> heldResources;

inline void gc() {
    heldResources.write([](auto& hr) {
	hr.clear();
    });
}

template<typename Type, typename ... Args>
inline uint32_t create(Args&& ... args) {
    auto ptr = std::make_shared<Type>(std::forward<Args>(args)...);
    auto key = *ptr->key;
    heldResources.write([&](auto& heldResources){
	heldResources.push_back(ptr);
    });
    return key;
}

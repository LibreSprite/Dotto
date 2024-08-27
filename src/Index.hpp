#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "Shared.hpp"
#include "Log.hpp"

template <typename Type>
class Index {
public:
    const std::size_t offset;

    using Optional = std::shared_ptr<Type>;

    Index(std::size_t offset) : offset{offset} {_ptr = this;}

    ~Index() {_ptr = nullptr;}

    uint32_t count() {
        return _data.read().size();
    }

    Optional operator [] (uint32_t key) {
	if (key < offset)
	    return Optional{};
        auto vkey = key - offset;
        return _data.read([&](auto& _data){
	    return (vkey >= _data.size()) ? Optional{} : _data[vkey].lock();
	});
    }

    static Optional find(uint32_t key) {
	return _ptr ? (*_ptr)[key] : Optional{};
    }

    uint32_t add(const std::shared_ptr<Type>& v) {
	std::size_t max{};
	_data.write([&](auto& _data) {
	    max = _data.size();
	    for (std::size_t i = 0; i < max; ++i) {
		auto& o = _data[i];
		if (o.expired()) {
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
    template <typename Derived> friend class AutoIndex;

    Shared<std::vector<std::weak_ptr<Type>>> _data;
    static inline Index<Type>* _ptr;
};

inline Shared<std::vector<std::shared_ptr<void>>> heldResources;

template<typename T>
class Y : public T {
public:
    template <typename ... PArgs>
    Y(PArgs&& ... args) : T{std::forward<PArgs>(args)...} {}
};

template <typename Derived>
class AutoIndex : public std::enable_shared_from_this<Derived> {
    uint32_t _key{};

public:
    template<typename T=Derived, typename ... Args>
    static std::shared_ptr<T> create(Args&& ... args) {
        LOG("Creating ", typeid(Derived).name());
        auto ptr = std::make_shared<Y<T>>(std::forward<Args>(args)...);
        ptr->_key = Index<Derived>::_ptr->add(ptr);
        heldResources.write([&](auto& heldResources){heldResources.push_back(ptr);});
        return ptr;
    }

    ~AutoIndex() {
        LOG("Destroying ", typeid(Derived).name());
	if (_key)
            Index<Derived>::_ptr->remove(_key);
    }

    uint32_t key() const {
        return _key;
    }
};

inline void gc() {
    heldResources.write([](auto& hr) {
	hr.clear();
    });
}

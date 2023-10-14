#pragma once

#include "API.hpp"

template <uint32_t _size = 1>
struct MessagePump {
    static constexpr uint32_t hash(std::string_view str) {
	uint32_t acc = 1991;
	for (auto ch : str) {
	    acc *= 31;
	    acc += ch;
	}
	return acc;
    }

    using Handler = void(*)(const std::vector<std::string>&);

    class PumpEntry {
    public:
	uint32_t key{};
	Handler handler{};
	constexpr PumpEntry() = default;
	constexpr PumpEntry(const char* key, Handler handler) : key{hash(key)}, handler{handler} {}

	template<typename O>
	constexpr void operator = (const O& o) {
	    key = o.key;
	    handler = o.handler;
	}

    };

    using Table = std::array<PumpEntry, _size>;
    Table table{};

    constexpr MessagePump() = default;

    constexpr MessagePump<_size + 1> add(const char* key, Handler handler) {
	MessagePump<_size + 1> p;
	for (int i = 0; i < _size; ++i)
	    p.table[i] = table[i];
	p.table[_size] = {key, handler};
	return p;
    }

    void operator () () const {
	std::vector<std::string> args;
	for (uint32_t argc ; (argc = popMessage());) {
	    if (argc < 2)
		continue;
	    auto key = hash(getMessageArg(1));
	    for (auto& entry : table) {
		if (entry.key == key) {
		    args.clear();
		    args.reserve(argc - 2);
		    for (int i = 2; i < argc; ++i)
			args.push_back(getMessageArg(i));
		    entry.handler(args);
		    break;
		}
	    }
	}
    }
};

#pragma once

#include "API.hpp"
#include <cstdint>

class FMT {
public:
    static inline std::string mode;
    std::variant<std::string, std::pair<std::any, std::string(*)(std::any&)>> data;

    FMT(const std::string& sv) : data{sv} {};
    FMT(const char* sv) : data{sv} {};
    FMT(char* sv) : data{sv} {};
    FMT(std::string_view sv) : data{std::string{sv}} {};

    template<typename T>
    FMT(const T& t) :
	data{
	    std::pair{
		std::any{t},
		+[](std::any& a)->std::string{return (*static_cast<FMT*>(nullptr)) << std::any_cast<T>(a);}
	    }
	} {}

    const std::string& operator () () {
	if (data.index() != 0) {
	    auto& pair = std::get<1>(data);
	    data = pair.second(pair.first);
	}
	return std::get<0>(data);
    }

    static std::string to_string(uint32_t ui, bool s) {
	uint32_t base = 10;
	std::string acc;
	bool force = true;
	if (s && int32_t(ui) < 0) {
	    acc.push_back('-');
	    ui = -ui;
	}
	if (mode == "#x") {
	    base = 16;
	    acc += "0x";
	}
	auto it = acc.end();
	while (ui || force) {
	    force = false;
	    auto d = ui % base;
	    ui /= base;
	    acc.insert(it, ((d < 10) ? '0' : 'A' - 10) + d);
	}
	return acc;
    }

    static std::string to_string(float ui) {
	std::string acc;
	if (ui < 0) {
	    acc.push_back('-');
	    ui = -ui;
	}

	float m = 1, f = ui - int(ui);
	int off = 0;
	for (;off < 5 && float(int(f*m)) < f*m; ++off, m *= 10);

	auto uim = int(ui * m);
	auto it = acc.end();
	while (true) {
	    auto d = uim % 10;
	    m *= 0.1f;
	    acc.insert(it, '0' + d);
	    off--;
	    uim = int(ui * m);
	    if (off == 0) {
		acc.insert(it, '.');
		continue;
	    }
	    if (!uim && off < 0)
		break;
	}
	return acc;

    }
};

inline std::string operator << (FMT&, int i) {return FMT::to_string(i, true);}
inline std::string operator << (FMT&, unsigned int i) {return FMT::to_string(i, false);}
inline std::string operator << (FMT&, long int i) {return FMT::to_string(i, true);}
inline std::string operator << (FMT&, long unsigned int i) {return FMT::to_string(i, false);}
inline std::string operator << (FMT&, float i) {return FMT::to_string(i);}
inline std::string operator << (FMT&, double i) {return FMT::to_string(i);}

inline std::string fmt(std::string_view f, std::vector<FMT>&& args) {
    std::string out;
    out.reserve(f.size());
    enum class Mode {
	def,
	id,
	id2,
	mod
    } mode = Mode::def;
    uint32_t id = 0;
    for (auto ch : f) {
	switch (mode) {
	case Mode::def:
	    if (ch == '{') {
		FMT::mode.clear();
		mode = Mode::id;
		break;
	    }
	    out.push_back(ch);
	    break;
	case Mode::id:
	    if (ch == '{') {
		out.push_back(ch);
		mode = Mode::def;
		break;
	    }
	    if (ch == ':') {
		mode = Mode::mod;
		break;
	    }
	    if (ch >= '0' && ch <= '9') {
		id = ch - '0';
		mode = Mode::id2;
		break;
	    }
	    if (ch == '}') {
		if (id < args.size())
		    out += args[id]();
		id++;
	    } else {
		out.push_back('{');
		out.push_back(ch);
	    }
	    mode = Mode::def;
	    break;
	case Mode::id2:
	    if (ch >= '0' && ch <= '9') {
		id = id * 10 + (ch - '0');
		break;
	    }
	    if (ch == ':') {
		mode = Mode::mod;
		break;
	    }
	    if (ch == '}') {
		if (id < args.size())
		    out += args[id]();
		id++;
	    } else {
		out.push_back(ch);
	    }
	    mode = Mode::def;
	    break;
	case Mode::mod:
	    if (ch == '}') {
		if (id < args.size())
		    out += args[id]();
		id++;
		mode = Mode::def;
	    } else {
		FMT::mode.push_back(ch);
	    }
	    break;
	}
    }
    return out;
}

template <typename ... Args>
inline std::string fmt(std::string_view f, Args&& ... arg) {
    return fmt(f, std::vector<FMT>{std::forward<Args>(arg)...});
}

template <typename ... Args>
inline uint32_t message(std::string_view fmt, Args&& ... args) {
    // return vmSystem(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)).c_str());
    return vmSystem(::fmt(fmt, args...).c_str());
}

template <typename ... Args>
inline void log(std::string_view fmt, Args&& ... args) {
    // puts(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)).c_str());
    puts(::fmt(fmt, args...).c_str());
}

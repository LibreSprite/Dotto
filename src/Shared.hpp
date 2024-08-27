#pragma once
#include <mutex>
#include <shared_mutex>
#include <type_traits>

template <typename Data>
struct Shared {
    using data_t = Data;

    template <typename Func>
    auto read(Func&& f) {
        std::shared_lock lock{mutex};
	if constexpr (std::is_void_v<decltype(f(const_cast<const data_t&>(data)))>) {
	    f(const_cast<const data_t&>(data));
	} else {
	    return f(const_cast<const data_t&>(data));
	}
    }

    template <typename Func>
    void write(Func&& f) {
        std::unique_lock lock{mutex};
        f(data);
    }

    class Writer {
        std::unique_lock<std::shared_mutex> lock;
        data_t& data;
    public:
        Writer(std::shared_mutex& mutex, data_t& data) : lock{mutex}, data{data} {}

        data_t& operator * () {return data;}
        data_t* operator -> () {return &data;}
    };

    Writer write() {
        return {mutex, data};
    }

    class Reader {
        std::shared_lock<std::shared_mutex> lock;
        const data_t& data;
    public:
        Reader(std::shared_mutex& mutex, data_t& data) : lock{mutex}, data{data} {}
        const data_t& operator * () {return data;}
        const data_t* operator -> () {return &data;}
    };

    Reader read() {
        return {mutex, data};
    }

private:
    data_t data;
    std::shared_mutex mutex;
};

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <thread>

#include "Events.hpp"

template <typename Log, typename VM>
class VMPool {
public:
    virtual ~VMPool() {
        running = false;
        for (auto& vm : vms)
            vm->yield();
        for (auto& thread : threads)
            thread.join();
    }

    bool empty() {
	return vms.empty();
    }

    bool busy() {
	return _busy || !queue.empty();
    }

    bool wait() {
	#ifdef EMSCRIPTEN
	return busy();
	#else
        using namespace std::chrono_literals;
	while (busy()) {
	    std::this_thread::sleep_for(1ms);
	}
	return false;
	#endif
    }

    VM& add(std::shared_ptr<VM> vm) {
        vms.push_back(vm);
        return *vm;
    }

    void release(const VM& vm) {
        for (auto it = vms.begin(); it != vms.end(); ++it) {
            if (it->get() == &vm) {
                (*it)->yield();
                *it = vms.back();
                vms.pop_back();
                return;
            }
        }
        log("VM not found");
    }

protected:
    void init() {
        auto maxThreads = std::max<std::size_t>(1, std::thread::hardware_concurrency());
        log("Initializing pool with ", maxThreads, " threads");
        for (std::size_t i = 0; i < maxThreads; ++i) {
            threads.push_back(std::thread([&]{run();}));
        }
    }

    ON(Update) {
        if (busy())
            return;
        if (threads.empty())
            init();
        std::lock_guard lg{queueMutex};
        queue = vms;
    }
    
    void run() {
        using namespace std::chrono_literals;
        while (running) {
            if (queue.empty()) {
                std::this_thread::sleep_for(1ms);
                continue;
            }

            std::shared_ptr<VM> vm;

            {
                std::lock_guard lg{queueMutex};
                if (queue.empty())
                    continue;
		_busy++;
                vm = queue.back();
                queue.pop_back();
            }

            vm->run();
	    _busy--;
        }
    }

private:
    std::atomic<bool> running{true};
    std::atomic<uint32_t> _busy{};
    std::mutex queueMutex;
    std::vector<std::shared_ptr<VM>> queue;
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<VM>> vms;
    std::vector<typename VM::APIFunc> api;
    Log log;
};

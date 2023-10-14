#pragma once

#include <vector>
#include <functional>
#include "Shared.hpp"

inline Shared<std::vector<std::function<void()>>> scheduledCallbacks;

template<typename Callback>
inline void mainThread(Callback&& callback) {
    scheduledCallbacks.write([&] (auto& scheduledCallbacks) {
	scheduledCallbacks.emplace_back(std::forward<Callback>(callback));
    });
}

inline void runMainThreadCallbacks() {
    scheduledCallbacks.write([] (auto& scheduledCallbacks) {
	for (auto& callback : scheduledCallbacks)
	    callback();
	scheduledCallbacks.clear();
    });
}

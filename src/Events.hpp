#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "Shared.hpp"

enum class EventId : uint32_t {
    Boot = 0,
    PreUpdate = 1,
    Update = 2,
    Draw = 3,
    PostUpdate = 4,
    Resize = 5,
    MouseLeftDown = 6,
    MouseMiddleDown = 7,
    MouseRightDown = 8,
    MouseLeftUp = 9,
    MouseMiddleUp = 10,
    MouseRightUp = 11,
    MouseMove = 12,
    MaxEvent
};

struct EventListener {
    static inline void emptyHandler(void*, EventId){}
    void* endpoint = nullptr;
    void (*handler)(void*, EventId) = emptyHandler;
};


inline Shared<std::array<std::vector<EventListener>, static_cast<std::size_t>(EventId::MaxEvent)>> eventListeners;

inline void createListenerSlot(EventId id, const EventListener& el) {
    EventListener* listener{};

    eventListeners.read([&](auto& eventListeners) {
        auto& listeners = eventListeners[static_cast<uint32_t>(id)];
        for (auto& candidate : listeners) {
            if (candidate.handler == EventListener::emptyHandler) {
                listener = const_cast<EventListener*>(&candidate);
                break;
            }
        }
    });

    eventListeners.write([&](auto& eventListeners){
        if (!listener) {
            auto& listeners = eventListeners[static_cast<uint32_t>(id)];
            listeners.push_back({});
            listener = &listeners.back();
        }
        *listener = el;
    });
}

inline void emit(EventId id) {
    eventListeners.read([&](auto& eventListeners) {
        for (auto& listener : eventListeners[static_cast<std::size_t>(id)])
            listener.handler(listener.endpoint, id);
    });
}

template <EventId id>
struct on {
    std::shared_ptr<void> data;

    template <typename ClassName>
    on(ClassName* parent, void (ClassName::*func)()) {
        struct Data {
            ClassName* parent;
            void (ClassName::*func)();
        };
        auto data = std::make_shared<Data>();
        data->parent = parent;
        data->func = func;
        this->data = data;
        createListenerSlot(id, {
                .endpoint = data.get(),
                .handler = [](void* ptr, EventId){
                    auto data = reinterpret_cast<Data*>(ptr);
                    auto parent = data->parent;
                    auto func = data->func;
                    (parent->*func)();
                }
            });
    }

    ~on() {
        eventListeners.write([&](auto& eventListeners) {
            auto& listeners = eventListeners[static_cast<std::size_t>(id)];
            EventListener* listener{};
            for (auto& candidate : listeners) {
                if (candidate.endpoint == data.get()) {
                    candidate = EventListener{};
                    break;
                }
            }
        });
    }
};

#define ON(EVENT) on<EventId::EVENT> EVENT ## _listener {this, &std::remove_reference_t<decltype(*this)>::EVENT}; void EVENT()

// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>

namespace internal {
    struct Listener {
        void* object;
        void* key;
        void (*call)(void* object, void* message);
    };

    template<typename Message>
    Vector<Listener>* channel() {
        static Vector<Listener>* ptr = ([]{
           atexit(+[]{
                      delete ptr;
                      ptr = nullptr;
                  });
           return new Vector<Listener>();
        })();
        return ptr;
    }

    template <typename Message, typename Object>
    void sub(Object* obj, void* key) {
        auto ch = channel<Message>();
        if (!ch)
            return;

        Listener listener{
            obj,
            key,
            +[](void* obj, void* msg) {
                static_cast<Object*>(obj)->on(*static_cast<Message*>(msg));
            }
        };

        for (auto& slot : *ch) {
            if (!slot.key) {
                slot = listener;
                return;
            }
        }

        ch->push_back(listener);
    }

    template <typename Message>
    void unsub(void* key) {
        auto ch = channel<Message>();
        if (!ch)
            return;
        for (auto it = ch->begin(); it != ch->end(); ++it) {
            if (it->key == key) {
                it->key = nullptr;
                it->call = nullptr;
                break;
            }
        }
    }

    inline void pub(Vector<Listener>* listeners, void* msg) {
        if (!listeners)
            return;
        for (std::size_t i = 0; i < listeners->size(); ++i) {
            auto& listener = listeners->at(i);
            if (listener.call) {
                listener.call(listener.object, msg);
            }
        }
    }
}

template<typename ... Messages>
class PubSub {
public:
    template<typename Object>
    PubSub(Object* obj) {
        (internal::sub<Messages>(obj, this),...);
    }

    template<typename Message>
    static Message& pub(Message&& msg) {
        internal::pub(internal::channel<Message>(), &msg);
        return msg;
    }

    template<typename Message>
    static Message& pub(Message& msg) {
        internal::pub(internal::channel<Message>(), &msg);
        return msg;
    }

    ~PubSub() {
        (internal::unsub<Messages>(this),...);
    }

    template<typename Message>
    Message& operator () (Message&& msg) {
        internal::pub(internal::channel<Message>(), &msg);
        return msg;
    }
};

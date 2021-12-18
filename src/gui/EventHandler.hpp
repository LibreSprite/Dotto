// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <typeinfo>
#include <typeindex>
#include <common/types.hpp>
#include <gui/Events.hpp>

namespace ui {

    class EventHandler {
        struct Listener {
            void* target;
            void (*call)(void* target, const Event& event);
        };

        HashMap<std::type_index, Vector<Listener>> handlers;

        template<typename Type, typename Target>
        void addSingleEventListener(Target* target) {
            auto& listeners = handlers[std::type_index(typeid(Type))];
            Listener obj {
                target,
                +[](void* target, const Event& event){
                    static_cast<Target*>(target)->eventHandler(*static_cast<const Type*>(&event));
                }
            };

            for (auto& listener : listeners) {
                if (listener.target == nullptr) {
                    listener = obj;
                    return;
                }
            }

            listeners.push_back(obj);
        }

    public:
        template<typename ... Type, typename Target>
        void addEventListener(Target* target) {
            (addSingleEventListener<Type>(target),...);
        }

        void removeEventListeners(void* target) {
            for (auto& entry : handlers) {
                for (auto& listener : entry.second) {
                    if (listener.target == target) {
                        listener.target = nullptr;
                        listener.call = nullptr;
                    }
                }
            }
        }

        virtual void processEvent(const Event& event) {
            auto it = handlers.find(std::type_index(typeid(event)));
            if (it != handlers.end()) {
                for (auto& listener : it->second) {
                    if (event.cancel) break;
                    listener.call(listener.target, event);
                }
            }
        }
    };
}

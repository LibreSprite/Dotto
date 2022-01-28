// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <functional>
#include <typeinfo>
#include <typeindex>

#include <common/types.hpp>
#include <gui/Events.hpp>
#include <log/Log.hpp>

namespace ui {

    class EventHandler {
        struct Listener {
            void* target;
            void (*call)(void* target, const Event& event);
        };

        struct StringListener {
            void* target;
            std::function<void(const void* arg)> call;
        };

        HashMap<std::type_index, Vector<Listener>> handlers;
        HashMap<String, Vector<StringListener>> stringHandlers;

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

        template<typename Arg, typename Target>
        void addEventListener(Target* target, std::function<void(const Arg&)>&& func) {
            auto wrapper = [=, func=std::move(func)](const void* arg){func(*static_cast<const Arg*>(arg));};
            auto& list = stringHandlers[typeid(Arg).name()];
            for (auto& entry : list) {
                if (entry.target == nullptr) {
                    entry = {target, std::move(wrapper)};
                    return;
                }
            }
            list.push_back({target, std::move(wrapper)});
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
            for (auto& entry : stringHandlers) {
                for (auto& listener : entry.second) {
                    if (listener.target == target) {
                        listener.target = nullptr;
                        listener.call = nullptr;
                    }
                }
            }
        }

        virtual void processEvent(const Event& event) {
            {
                auto it = handlers.find(std::type_index(typeid(event)));
                if (it != handlers.end()) {
                    for (auto& listener : it->second) {
                        if (event.cancel) break;
                        listener.call(listener.target, event);
                    }
                }
            }
            {
                auto name = typeid(event).name();
                auto it = stringHandlers.find(name);
                if (it != stringHandlers.end()) {
                    for (auto& listener : it->second) {
                        if (event.cancel) break;
                        listener.call(&event);
                    }
                }
            }
        }
    };
}

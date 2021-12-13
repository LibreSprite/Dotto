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
            void* const target;
            void (* const call)(void* target, const Event& event);
        };

        HashMap<std::type_index, Vector<Listener>> handlers;

        template<typename Type, typename Target>
        void addSingleEventListener(Target* target) {
            Listener obj {
                target,
                +[](void* target, const Event& event){
                    static_cast<Target*>(target)->eventHandler(*static_cast<const Type*>(&event));
                }
            };
            handlers[std::type_index(typeid(Type))].push_back(obj);
        }

    protected:

        template<typename ... Type, typename Target>
        void addEventListener(Target* target) {
            (addSingleEventListener<Type>(target),...);
        }

    public:
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

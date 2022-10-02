// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PropertySet.hpp>
#include <common/String.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Shortcut : public ui::Controller {
    inject<Config> config;

    class KeyEntry;
    using KeyMap = HashMap<String, std::shared_ptr<KeyEntry>>;

    class KeyEntry {
        std::unique_ptr<KeyMap> childMap;
        std::function<void()> action;

        bool isModifier(const String& key) {
            return key == "LCTRL" ||
                   key == "RCTRL" ||
                   key == "LSHIFT" ||
                   key == "RSHIFT" ||
                   key == "LALT" ||
                   key == "RALT";
        }

    public:
        KeyMap* getChildMap(Shortcut* shortcut) {
            if (!childMap) {
                childMap = std::make_unique<KeyMap>();
                action = [this, shortcut]{
                    String keyCode;
                    Vector<String> keys;
                    Vector<String> altKeys;
                    for (auto& key : shortcut->lastKeyDown->pressedKeys) {
                        keys.push_back(key);
                        altKeys.push_back(key.c_str() + isModifier(key));

                    }
                    std::sort(keys.begin(), keys.end());
                    auto it = childMap->find(join(keys, "+"));
                    if (it == childMap->end()) {
                        std::sort(altKeys.begin(), altKeys.end());
                        it = childMap->find(join(altKeys, "+"));
                    }
                    if (it == childMap->end())
                        it = childMap->find(std::to_string(shortcut->lastKeyDown->keycode));
                    if (it == childMap->end())
                        it = childMap->find(tolower(shortcut->lastKeyDown->keyname));
                    if (it == childMap->end() && isModifier(shortcut->lastKeyDown->keyname))
                        it = childMap->find(tolower(shortcut->lastKeyDown->keyname + 1));
                    if (it == childMap->end()) {
                        static String lastPrint;
                        if (lastPrint != shortcut->lastKeyDown->keyname) {
                            lastPrint = shortcut->lastKeyDown->keyname;
                            logV("Key name: ", lastPrint);
                        }
                    }
                    shortcut->currentMap = (it != childMap->end()) ? it->second.get() : &shortcut->defaultMap;
                    if (!shortcut->currentMap->childMap || shortcut->currentMap->childMap->empty()) {
                        shortcut->currentMap->activate();
                    }
                };
            }
            return childMap.get();
        }

        void setAction(std::function<void()>&& action) {
            this->action = std::move(action);
        }

        void activate() {
            if (action) {
                action();
            }
        }
    };

    std::shared_ptr<PropertySet> releaseCommand;
    KeyEntry defaultMap;
    KeyEntry* currentMap = &defaultMap;
    const ui::KeyDown *lastKeyDown = nullptr;

    void attach(ui::Node* node) {
        if (!node)
            return;
        // attach(node->getParent());
        auto& id = *node->id;
        if (id.empty())
            return;
        auto containerProperties = config->properties->get<std::shared_ptr<PropertySet>>(id);
        if (!containerProperties)
            return;
        auto bindings = containerProperties->get<std::shared_ptr<PropertySet>>("keybindings");
        if (!bindings)
            return;
        for (auto& entry : bindings->getMap()) {
            std::shared_ptr<PropertySet> commandList = *entry.second;
            if (!commandList)
                continue;
            auto chords = split(entry.first, " ");
            currentMap = &defaultMap;
            for (auto& chord : chords) {
                auto& map = *currentMap->getChildMap(this);
                auto sorted = split(chord, "+");
                std::sort(sorted.begin(), sorted.end());
                auto& ptr = map[join(sorted, "+")];
                if (!ptr)
                    ptr = std::make_shared<KeyEntry>();
                currentMap = ptr.get();
            }
            currentMap->setAction([=]{
                currentMap = &defaultMap;
                releaseCommand = commandList->get<std::shared_ptr<PropertySet>>("release");
                std::size_t size = commandList->getMap().size();
                for (std::size_t i = 0; i < size; ++i) {
                    call(commandList->get<std::shared_ptr<PropertySet>>(std::to_string(i)));
                }
            });
        }
        currentMap = &defaultMap;
    }

    void call(std::shared_ptr<PropertySet> properties) {
        if (!properties)
            return;
        auto command = tolower(properties->get<String>("name"));
        if (command.empty())
            return;
        if (auto instance = inject<Command>{command}) {
            instance->load(*properties);
            instance->run();
        }
    }

public:
    void attach() override {
        node()->addEventListener<ui::KeyUp, ui::KeyDown>(this);
        attach(node());
    }

    void eventHandler(const ui::KeyDown& event) {
        lastKeyDown = &event;
        currentMap->activate();
    }

    void eventHandler(const ui::KeyUp& event) {
        call(releaseCommand);
        releaseCommand = nullptr;
    }
};

static ui::Controller::Shared<Shortcut> shortcut{"shortcut"};

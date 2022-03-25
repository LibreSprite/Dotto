// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/FunctionRef.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/String.hpp>
#include <doc/Cell.hpp>
#include <log/Log.hpp>
#include <filters/Filter.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class ActivateFilter : public Command {
    Property<String> filter{this, "filter"};
    Property<bool> interactive{this, "interactive"};
    PubSub<> pub{this};
    std::weak_ptr<ui::Node> menu;

    Vector<Surface::PixelType> undoData;
    std::shared_ptr<Cell> cell;

public:
    void undo() override {
        cell->getComposite()->setPixels(this->undoData);
    }

    void redo() override {
        auto it = Filter::instances.find(tolower(filter));
        if (it == Filter::instances.end()) {
            logE("Invalid filter \"", *filter, "\"");
            return;
        }
        auto& filter = it->second;
        auto surface = cell->getComposite();
        filter->run(surface->shared_from_this());
    }

    void run() override {
        auto it = Filter::instances.find(tolower(filter));
        if (it == Filter::instances.end()) {
            logE("Invalid filter \"", *filter, "\"");
            return;
        }

        auto& filter = it->second;
        if (Filter::active.lock() == filter) {
            logV("Filter already active");
            return;
        }

        inject<Cell> cell{"activecell"};
        if (!cell) {
            logE("No active cell");
            return;
        }

        std::shared_ptr<PropertySet> meta;
        if (interactive) {
            meta = filter->getMetaProperties();
        }

        if (!interactive || !meta) {
            auto surface = cell->getComposite();
            this->cell = cell;
            this->undoData = surface->getPixels();
            filter->load(getPropertySet());
            filter->run(surface->shared_from_this());
            commit();
            return;
        }

        auto metamenu = ui::Node::fromXML("metamenu");
        if (!metamenu) {
            logE("Could not create metamenu");
            return;
        }

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "row"},
                    {"id", "okcancel"}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "textbutton"},
                    {"parent", "okcancel"},
                    {"label", "cancel"},
                    {"click", FunctionRef<void()>([=]{
                        metamenu->remove();
                    })}
                }));

        auto shared = shared_from_this();
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "textbutton"},
                    {"parent", "okcancel"},
                    {"label", "ok"},
                    {"click", FunctionRef<void()>([=]{
                        auto ps = std::make_shared<PropertySet>();
                        metamenu->set("result", ps);
                        metamenu->remove();
                        shared->load(*ps);
                        interactive.value = false;
                        shared->run();
                    })}
                }));

        metamenu->set("meta", meta);
    }

};

static Command::Shared<ActivateFilter> cmd{"activatefilter"};

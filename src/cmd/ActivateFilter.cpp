// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/FunctionRef.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/String.hpp>
#include <doc/Cell.hpp>
#include <doc/Timeline.hpp>
#include <log/Log.hpp>
#include <filters/Filter.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class ActivateFilter : public Command {
    Property<String> filter{this, "filter"};
    Property<bool> interactive{this, "interactive"};
    PubSub<> pub{this};
    std::weak_ptr<ui::Node> menu;

    U32 frame, layer;
    bool allFrames, allLayers;

    U32 startFrame() {return allFrames ? 0 : frame;}
    U32 endFrame() {return allFrames ? frame : frame + 1;}
    U32 frameCount() {return endFrame() - startFrame();}

    U32 startLayer() {return allLayers ? 0 : layer;}
    U32 endLayer() {return allLayers ? layer : layer + 1;}
    U32 layerCount() {return endLayer() - startLayer();}

    U32 undoSize() {return layerCount() * frameCount();}

    Vector<std::shared_ptr<Surface>> undoData;

public:
    U32 commitSize() override {return undoSize();}

    void undo() override {
        auto doc = this->doc();
        auto timeline = doc->currentTimeline();
        U32 stride = allFrames ? (allLayers ? timeline->layerCount() : 1) : 0;
        for (U32 frame = startFrame(); frame != endFrame(); ++frame) {
            for (U32 layer = startLayer(); layer != endLayer(); ++layer) {
                auto cell = timeline->getCell(frame, layer);
                if (!cell)
                    continue;
                auto surface = cell->getComposite();
                if (!surface)
                    continue;
                if (auto data = undoData[(frame - startFrame()) * stride + (layer - startLayer())]) {
                    *surface = *data;
                }
            }
        }
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

        auto doc = this->doc();
        if (!doc) {
            logE("No active document");
            return;
        }

        auto timeline = doc->currentTimeline();
        if (!timeline) {
            logE("No active timeline");
            return;
        }

        if (interactive) {
            if (auto meta = filter->getMetaProperties()) {
                showMenu(meta);
                return;
            }
        }

        filter->load(getPropertySet());
        allFrames = filter->allFrames;
        allLayers = filter->allLayers;
        frame = allFrames ? timeline->frameCount() : timeline->frame();
        layer = allLayers ? timeline->layerCount() : timeline->layer();
        U32 stride = allFrames ? (allLayers ? timeline->layerCount() : 1) : 0;

        bool hasUndoData = undoData.size() == undoSize();
        if (!hasUndoData) {
            undoData.resize(undoSize());
        }

        logV("Running ", *this->filter, " on ",
             (allFrames ? "all frames " : "frame " + std::to_string(frame)),
             " and ",
             (allLayers ? "all layers " : "layer " + std::to_string(layer)));

        for (U32 frame = startFrame(); frame != endFrame(); ++frame) {
            for (U32 layer = startLayer(); layer != endLayer(); ++layer) {
                auto cell = timeline->getCell(frame, layer);
                if (!cell)
                    continue;
                auto surface = cell->getComposite();
                if (!surface)
                    continue;
                if (!hasUndoData) {
                    undoData[(frame - startFrame()) * stride + (layer - startLayer())] = surface->clone();
                }
                filter->run(surface->shared_from_this());
            }
        }

        commit();
    }

    void showMenu(std::shared_ptr<PropertySet> meta) {
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

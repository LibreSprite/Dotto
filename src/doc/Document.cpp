// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <chrono>

#include <cmd/Command.hpp>
#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/PropertySet.hpp>
#include <common/String.hpp>
#include <common/Surface.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <fs/FileSystem.hpp>


class DocumentImpl : public Document {
    friend class HistoryLock;

    PubSub<> pub{this};
    HashMap<String, std::shared_ptr<Timeline>> guidToTimeline;
    String GUID = getGUID();
    String currentTimelineName;
    U32 docWidth = 0;
    U32 docHeight = 0;
    String filepath;
    inject<Palette> globalPalette{"new"};

    Vector<std::shared_ptr<Command>> history;
    U32 historyCursor = 0;
    U32 lockHistory = 0;

public:
    String getGUID() {
        U32 now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        static U32 counter = now * U32(rand());
        counter++;
        return tostring(counter, 36) + "-" + tostring(now, 36);
    }

    U32 width() override {
        return docWidth;
    }

    U32 height() override {
        return docHeight;
    }

    void setDocumentSize(U32 width, U32 height) override {
        if (docWidth == width && docHeight == height) {
            return;
        }
        docWidth = width;
        docHeight = height;
        pub(msg::ResizeDocument{shared_from_this()});
    }

    bool load(const Value& resource) override {
        if (resource.has<std::shared_ptr<Surface>>()) {
            return loadFromSurface(resource);
        }
        if (resource.has<std::shared_ptr<PropertySet>>()) {
            return loadFromPropertySet(resource);
        }
        return false;
    }

    bool loadFromPropertySet(std::shared_ptr<PropertySet> properties) {
        if (!properties)
            return false;
        auto timeline = createTimeline();
        inject<Cell> cell{"bitmap"};
        auto surface = cell->getComposite();
        surface->resize(properties->get<U32>("width") ?: 16,
                        properties->get<U32>("height") ?: 16);
        Value palette;
        auto palettePath = properties->get<String>("palette");
        if (!palettePath.empty()) {
            palette = inject<FileSystem>{}->parse(palettePath);
        } else {
            palette = properties->get<std::shared_ptr<Palette>>("palette");
            if (!palette) {
                palette = properties->get<std::shared_ptr<Surface>>("palette");
            }
        }

        if (palette.has<std::shared_ptr<Surface>>()) {
            if (auto surface = palette.get<std::shared_ptr<Surface>>()) {
                globalPalette->loadFromSurface(*surface, 255);
            }
        } else if (palette.has<std::shared_ptr<Palette>>()) {
            if (auto ptr = palette.get<std::shared_ptr<Palette>>()) {
                *globalPalette = *ptr;
            }
        }

        timeline->setCell(0, 0, cell);
        docWidth = surface->width();
        docHeight = surface->height();
        return true;
    }

    bool loadFromSurface(std::shared_ptr<Surface> surface) {
        if (!surface)
            return false;
        auto timeline = createTimeline();
        inject<Cell> cell{"bitmap"};
        *cell->getComposite() = *surface;
        globalPalette->loadFromSurface(*surface, 255);
        timeline->setCell(0, 0, cell);
        docWidth = surface->width();
        docHeight = surface->height();
        return true;
    }

    bool selectTimeline(const String& timeline) override {
        auto it = guidToTimeline.find(timeline);
        if (it == guidToTimeline.end())
            return false;
        currentTimelineName = timeline;
        return true;
    }

    std::shared_ptr<Timeline> currentTimeline() override {
        return guidToTimeline[currentTimelineName];
    }

    std::shared_ptr<Timeline> createTimeline() override {
        Provides doc{this};
        auto timeline = inject<Timeline>{"new"}.shared();
        timeline->GUID = getGUID();
        guidToTimeline[timeline->GUID] = timeline;
        currentTimelineName = timeline->GUID;
        return timeline;
    }

    const HashMap<String, std::shared_ptr<Timeline>>& getTimelines() override {
        return guidToTimeline;
    }

    std::shared_ptr<Command> getLastCommand() override {
        return historyCursor == 0 ? nullptr : history[historyCursor - 1];
    }

    void writeHistory(std::shared_ptr<Command> command) override {
        if (lockHistory)
            return;
        if (historyCursor < history.size())
            history.resize(historyCursor);
        logV("Commit: ", command->getName());
        history.push_back(command);
        U32 maxUndoSize = inject<Config>{}->properties->get<U32>("max-undo-size");
        if (history.size() > maxUndoSize)
            history.erase(history.begin());
        historyCursor = history.size();
    }

    void undo() override {
        if (historyCursor == 0)
            return;
        lockHistory++;
        historyCursor--;
        history[historyCursor]->undo();
        lockHistory--;
    }

    void redo() override {
        if (historyCursor == history.size())
            return;
        lockHistory++;
        history[historyCursor]->redo();
        historyCursor++;
        lockHistory--;
    }

    HistoryLock getHistoryLock() override {
        return {shared_from_this()};
    }

    String path() override {
        return filepath;
    }

    void setPath(const String& path) override {
        filepath = path;
    }

    std::shared_ptr<Palette> palette() override {
        return globalPalette;
    }
};

HistoryLock::HistoryLock(std::shared_ptr<Document> doc) : doc{doc} {
    std::static_pointer_cast<DocumentImpl>(doc)->lockHistory++;
}

HistoryLock::~HistoryLock() {
    std::static_pointer_cast<DocumentImpl>(doc)->lockHistory--;
}

static Document::Shared<DocumentImpl> reg{"new"};

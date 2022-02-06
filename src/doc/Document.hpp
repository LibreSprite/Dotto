// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

class Timeline;
class Command;

class Document : public Injectable<Document>, public Serializable, public std::enable_shared_from_this<Document> {
public:
    virtual bool load(const Value& resource) = 0;
    virtual U32 width() = 0;
    virtual U32 height() = 0;
    virtual bool selectTimeline(const String& timeline) = 0;
    virtual std::shared_ptr<Timeline> currentTimeline() = 0;
    virtual std::shared_ptr<Timeline> createTimeline() = 0;
    virtual const HashMap<String, std::shared_ptr<Timeline>>& getTimelines() = 0;
    virtual std::shared_ptr<Command> getLastCommand() = 0;
    virtual void writeHistory(std::shared_ptr<Command> command) = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual String path() = 0;
    virtual void setPath(const String&) = 0;
};

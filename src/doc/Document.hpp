// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

class Timeline;

class Document : public Injectable<Document>, public Serializable, public std::enable_shared_from_this<Document> {
public:
    virtual std::shared_ptr<Timeline> createTimeline() = 0;
    virtual const HashMap<String, std::shared_ptr<Timeline>>& getTimelines() = 0;
};

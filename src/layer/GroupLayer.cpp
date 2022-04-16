// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/System.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <layer/Layer.hpp>
#include <tools/Tool.hpp>

class GroupLayer : public Layer {
public:
    virtual void clearOverlays() {}
    virtual void update() {}
};

static Layer::Shared<GroupLayer> reg{"group"};

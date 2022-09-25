// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <script/ScriptObject.hpp>

namespace ui {
    class Event;
}

class AppScriptObject : public script::ScriptObject {
public:
    virtual Value setTarget(const Value& target) = 0;
    virtual Value setEventTarget(const Value& eventTarget) = 0;
    virtual const ui::Event* setEvent(const ui::Event* event) = 0;
};

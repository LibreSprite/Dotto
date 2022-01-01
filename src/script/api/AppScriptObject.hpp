// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <script/ScriptObject.hpp>

class AppScriptObject : public script::ScriptObject {
public:
    virtual void setTarget(const Value& target) = 0;
    virtual void setEventTarget(const Value& eventTarget) = 0;
};

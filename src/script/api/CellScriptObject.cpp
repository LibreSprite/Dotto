// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class CellScriptObject : public script::ScriptObject {
    std::weak_ptr<Cell> weak;
    PubSub<> pub{this};

public:
    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vcell) override {
        std::shared_ptr<Cell> cell = vcell;
        weak = cell;

        auto weak = this->weak;
        addFunction("activate", [=]{
            if (auto cell = weak.lock()) {
                pub(msg::ActivateCell{cell});
                return true;
            }
            return false;
        });

        addProperty("composite", [=]() -> script::Value {
            if (auto cell = weak.lock()) {
                return getEngine().toValue(cell->getComposite()->shared_from_this());
            }
            return nullptr;
        });
    }
};

static script::ScriptObject::Shared<CellScriptObject> reg{typeid(std::shared_ptr<Cell>).name()};

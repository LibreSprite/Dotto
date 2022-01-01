// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <script/ScriptObject.hpp>
#include <common/Value.hpp>
#include <script/Value.hpp>

namespace script {

    String Value::str() const {
        if (type == Type::UNDEFINED) return "undefined";
        if (type == Type::INT) return std::to_string(data.int_v);
        if (type == Type::DOUBLE) return std::to_string(data.double_v);
        if (type == Type::BUFFER) return String(data.buffer_v->data(), data.buffer_v->end());
        if (type == Type::STRING) return *data.string_v;
        if (type == Type::OBJECT) return data.object_v ? data.object_v->getWrapped().toString() : "[object null]";
        logV("Could not convert ", (int) type, " into string");
        return String{};
    }

}

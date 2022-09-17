// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/inject.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>

void Cell::postInject() {
    if (name.empty()) {
        setName(inject<Config>{}->translate(getType()), true);
    }
}

void Cell::setDocument(Document* document) {
    if (_document)
        _document->removeCell(this);

    _document = document;

    if (document)
        _document->addCell(this);
}

void Cell::modify(bool silent) {
    if (!silent) {
        PubSub<>::pub(msg::ModifyCell{shared_from_this()});
    }
}

void Cell::setName(const String& newName, bool silent) {
    if (name == newName)
        return;
    name = newName;
    modify(silent);
}

void Cell::setAlpha(F32 v, bool silent) {
    auto old = alpha;
    alpha = std::clamp(v, 0.0f, 1.0f);
    modify(old == alpha || silent);
}

void Cell::setBlendMode(const String& newMode, bool silent) {
    if (blendMode == newMode)
        return;
    blendMode = newMode;
    modify(silent);
}


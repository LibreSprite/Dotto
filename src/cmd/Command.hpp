// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <doc/Document.hpp>

class Command : public Injectable<Command>,
                public Model,
                public std::enable_shared_from_this<Command> {
    std::weak_ptr<Document> weakDoc;
    bool wasCommitted = false;

protected:
    Command() {
        weakDoc = inject<Document>{InjectSilent::Yes, "activedocument"}.shared();
    }

    void commit() {
        if (wasCommitted)
            return;
        wasCommitted = true;
        if (auto doc = weakDoc.lock())
            doc->writeHistory(shared_from_this());
    }

public:
    std::shared_ptr<Document> doc() {return weakDoc.lock();}
    bool committed() {return wasCommitted;}
    virtual void run() = 0;
    virtual void undo() {};
    virtual void redo() {run();}
};

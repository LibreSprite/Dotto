// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Parser.hpp>
#include <common/String.hpp>
#include <fs/FileDialog.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>

class CloseFile : public Command {
public:
    void run() override {
        inject<ui::Node> editor{"activeeditor"};
        // TODO: check if the file needs to be saved first
        if (editor) {
            editor->remove();
        }
    }
};

static Command::Shared<CloseFile> closefile{"closefile"};

// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Parser.hpp>
#include <common/String.hpp>
#include <fs/FileDialog.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>

class OpenFile : public Command {
public:
    Property<String> fileName{this, "filename"};

    void showOpenDialog() {
        auto& parsers = Parser::getRegistry();
        Vector<String> filters;
        for (auto& entry : parsers) {
            if (entry.first.empty() || !entry.second.hasFlag("image"))
                continue;
            filters.push_back("*." + entry.first);
        }
        inject<FileDialog> dialog;
        dialog->filterDescription = "All Formats";
        dialog->title = "Open Image...";
        dialog->filters = std::move(filters);
        dialog->open();
        if (!dialog->result.empty())
            set("filename", join(dialog->result, "|"));
    }

    void run() override {
        if (fileName->empty())
            showOpenDialog();
        if (!fileName->empty()) {
            auto files = split(fileName, "|");
            if (auto editor = inject<ui::Node>{"root"}->findChildById("editor")) { // TODO: create new editor instead
                editor->set("file", files[0]);
            }
        }
    }
};

static Command::Shared<OpenFile> openfile{"openfile"};

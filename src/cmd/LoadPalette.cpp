// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/Parser.hpp>
#include <common/String.hpp>
#include <doc/Document.hpp>
#include <doc/Palette.hpp>
#include <fs/FileDialog.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>

class LoadPalette : public Command {
public:
    Property<String> fileName{this, "filename"};
    std::shared_ptr<Palette> undo;

    void showOpenDialog() {
        auto& parsers = Parser::getRegistry();
        Vector<String> filters;
        for (auto& entry : parsers) {
            if (entry.first.empty() || (!entry.second.hasFlag("palette") && !entry.second.hasFlag("image")))
                continue;
            filters.push_back("*." + entry.first);
        }
        inject<FileDialog> dialog;
        if (dialog) {
            auto that = shared_from_this();
            dialog->filterDescription = "All Formats";
            dialog->title = "Open Palette...";
            dialog->filters = std::move(filters);
            dialog->open([=](const Vector<String>& name){
                that->set("filename", join(name, "|"));
                if (!fileName->empty()) {
                    that->run();
                }
            });
        } else {
            logE("No File dialog available");
        }
    }

    void run() override {
        inject<Document> doc{"activedocument"};
        if (!doc) {
            return;
        }

        if (fileName->empty()) {
            showOpenDialog();
            return;
        }
        if (!fileName->empty()) {
            auto file = inject<FileSystem>{}->parse(split(fileName, "|")[0]);
            std::shared_ptr<Palette> pal;
            if (!undo) {
                undo = inject<Palette>{"new"};
                *undo = *doc->palette();
            }
            if (file.has<std::shared_ptr<Surface>>()) {
                auto surface = file.get<std::shared_ptr<Surface>>();
                if (!surface)
                    return;
                doc->palette()->loadFromSurface(*surface, 256);
            } else if (file.has<std::shared_ptr<Palette>>()) {
                auto pal = file.get<std::shared_ptr<Palette>>();
                if (!pal)
                    return;
                *doc->palette() = *pal;
            }
            PubSub<>::pub(msg::ChangePalette{doc->palette()});
            commit();
        }
    }
};

static Command::Shared<LoadPalette> reg{"loadpalette"};

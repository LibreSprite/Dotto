// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(EMSCRIPTEN)

#include <common/String.hpp>
#include <fs/FileDialog.hpp>
#include <log/Log.hpp>
#include <task/TaskManager.hpp>
#include <gui/Node.hpp>

class WasmFileDialog : public FileDialog {
public:
    TaskHandle handle;
    inject<TaskManager> taskManager;

    void open(Callback&& callback) override {
        handle = taskManager->add(
            [
                filters = this->filters,
                defaultPath = this->defaultPath,
                filterDescription = this->filterDescription,
                title = this->title,
                allowMultiple = this->allowMultiple
             ]()->Value{
                return Vector<String>{};
            },
            [=](Value&& result){
                if (result.has<Vector<String>>()) {
                    callback(result.get<Vector<String>>());
                } else {
                    callback({});
                }
            });
    }

    void save(Callback&& callback) override {
        if (auto node = ui::Node::fromXML("websave")) {
            node->set("default-path", this->defaultPath);
        }
        // handle = taskManager->add(
        //     [
        //         filters = this->filters,
        //         defaultPath = this->defaultPath,
        //         filterDescription = this->filterDescription,
        //         title = this->title,
        //         allowMultiple = this->allowMultiple
        //      ]()->Value{
        //         return String{};
        //     },
        //     [=](Value&& result){
        //         if (result.has<String>()) {
        //             callback(Vector<String>{result.get<String>()});
        //         } else {
        //             callback({});
        //         }
        //     });
    }
};

static FileDialog::Singleton<WasmFileDialog> reg{""};

#endif

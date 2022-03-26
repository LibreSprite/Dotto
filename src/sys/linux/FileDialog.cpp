// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(__linux__) || defined(__WINDOWS__) || defined(__APPLE__)

#include <tinyfiledialogs/tinyfiledialogs.h>

#include <common/String.hpp>
#include <fs/FileDialog.hpp>
#include <log/Log.hpp>
#include <task/TaskManager.hpp>

class LinuxFileDialog : public FileDialog {
public:
    TaskHandle handle;
    inject<TaskManager> taskManager;

    static Vector<const char*> init(const Vector<String>& filters) {
        Vector<const char*> cfilters;
        cfilters.reserve(filters.size());
        for (auto& filter : filters)
            cfilters.push_back(filter.c_str());
        return cfilters;
    }

    void open(Callback&& callback) override {
        handle = taskManager->add(
            [
                filters = this->filters,
                defaultPath = this->defaultPath,
                filterDescription = this->filterDescription,
                title = this->title,
                allowMultiple = this->allowMultiple
             ]()->Value{
                auto cfilters = init(filters);
                auto chosen = tinyfd_openFileDialog(title.c_str(),
                                                    defaultPath.c_str(),
                                                    cfilters.size(),
                                                    cfilters.data(),
                                                    filterDescription.c_str(),
                                                    allowMultiple);
                return chosen ? split(chosen, "|") : Vector<String>{};
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
        handle = taskManager->add(
            [
                filters = this->filters,
                defaultPath = this->defaultPath,
                filterDescription = this->filterDescription,
                title = this->title,
                allowMultiple = this->allowMultiple
             ]()->Value{
                auto cfilters = init(filters);
                auto chosen = tinyfd_saveFileDialog(title.c_str(),
                                                defaultPath.c_str(),
                                                cfilters.size(),
                                                cfilters.data(),
                                                filterDescription.c_str());
                return chosen ? String{chosen} : String{};
            },
            [=](Value&& result){
                if (result.has<String>()) {
                    callback(Vector<String>{result.get<String>()});
                } else {
                    callback({});
                }
            });
    }
};

static FileDialog::Singleton<LinuxFileDialog> reg{""};

#endif

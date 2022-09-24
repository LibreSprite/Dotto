// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/Parser.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <gui/Node.hpp>

using namespace fs;

class ConfigImpl : public Config {
public:
    PubSub<msg::Flush, msg::Tock> pub{this};

    std::shared_ptr<PropertySet> style;

    bool _dirty = false;

    bool boot() override {
        inject<FileSystem> fs;

        if (!properties)
            properties = fs->parse("%appdata/settings.ini");

        try {
            if (std::shared_ptr<PropertySet> userprops = fs->parse("%userdata/settings.ini")) {
                properties->append(*userprops);
            }
        }catch(const std::exception& ex){}

        if (!properties) {
            Log::write(Log::Level::Error, "Could not open settings file. Reinstalling may fix this problem.");
            return false;
        }

        initLanguage(fs);
        initStyle(fs);
        return true;
    }

    void on(msg::Flush& flush) {
        flush.hold(style);
    }

    void dirty() override {
        _dirty = true;
    }

    void on(msg::Tock&) {
        if (!_dirty) {
            return;
        }
        _dirty = false;
        if (!inject<FileSystem>{}->write("%userdata/settings.ini", properties)) {
            logE("Could not save user settings.");
        }
    }

    void initLanguage(inject<FileSystem>& fs) {
        auto languageName = properties->get<String>("language");
        if (languageName.empty())
            languageName = "en_US";

        language = std::make_shared<PropertySet>();

        String fullName;
        bool first = true;
        for (auto& part : split("all_" + languageName, "_")) {
            if (first) languageName = "all";
            else {
                if (!fullName.empty())
                    fullName += "_";
                fullName += part;
                languageName = fullName;
            }
            first = false;
            if (std::shared_ptr<PropertySet> ps = fs->parse("%appdata/i18n/" + languageName + ".ini")) {
                language->append(ps);
                for (auto& entry : ps->getMap()) {
                    auto lower = tolower(entry.first);
                    if (entry.first != lower && language->getMap().find(lower) == language->getMap().end())
                        language->set(lower, entry.second->get<String>());
                }
            } else if (languageName != "en_US") {
                logI("Could not load language [", languageName, "]");
            }
        }
    }

    void initStyle(inject<FileSystem>& fs) {
        auto skin = properties->get<String>("skin");
        if (auto root = fs->getRoot()->get<Folder>())
            root->mount("%skin", "dir", fs->find(skin, "dir")->getUID());
        style = fs->parse("%skin/gui/style.ini");
    }

    String eval(const String& str, const ui::Node* context) {
        String ret;
        String out = trim(str);
        while (context) {
            if (context->getPropertySet().get(out, ret)) {
                return ret;
            }
            context = context->getParent();
        }
        if (properties->get(out, ret))
            return ret;
        return str;
    }

    String translate(const String& str, const ui::Node* context) override {
        String format = language->get<String>(str);
        if (format.empty())
            format = str;

        String out;
        out.reserve(format.size());

        for (std::size_t i = 0, size = format.size(); i < size; ++i) {
            auto c = format[i];
            if (c == '$' && i < size - 3 && format[i+1] == '{') {
                auto end = i + 2;
                while (end < size && format[end] != '}') {
                    end++;
                }
                if (end != size) {
                    out += eval(format.substr(i + 2, end - (i + 2)), context);
                    i = end;
                    continue;
                }
            }
            out.append(&c, 1);
        }

        return out;
    }
};

static Config::Shared<ConfigImpl> cfg{"new"};

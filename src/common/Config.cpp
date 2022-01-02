// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Config.hpp>
#include <common/Parser.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>

class ConfigImpl : public Config {
public:

    bool boot() override {
        inject<FileSystem> fs;
        properties = fs->parse("%userdata/settings.ini");
        if (!properties)
            properties = fs->parse("%appdata/settings.ini");
        if (!properties) {
            Log::write(Log::Level::ERROR, "Could not open settings file. Reinstalling may fix this problem.");
            return false;
        }

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
            auto add = fs->parse("%appdata/i18n/" + languageName + ".ini");
            if (add.has<std::shared_ptr<PropertySet>>()) {
                language->append(*add.get<std::shared_ptr<PropertySet>>());
            }
        }

        return true;
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

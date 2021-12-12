// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <common/PropertySet.hpp>
#include <common/String.hpp>

class IniParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        auto set = std::make_shared<PropertySet>();
        if (!file || !file->isOpen()) {
            logV("Could not read ini file");
            return nullptr;
        }

        logV("Parsing ini file");

        String domain = "global";
        auto subset = set;
        for (auto& rawline : split(file->readTextFile(), std::regex("[\n\r]+"))) {
            String line = trim(rawline);

            auto comment = line.find("#");
            if (comment != String::npos) {
                line = trim(line.substr(0, comment));
            }

            if (line.empty())
                continue;

            if (line.front() == '[' && line.back() == ']') {
                domain = trim(line.substr(1, line.size() - 2));
                auto parent = set;
                logV("Section [", domain, "]");
                for (auto& part : split(domain, ":")) {
                    if (!parent->get(part, subset)) {
                        subset = std::make_shared<PropertySet>();
                        parent->set(part, subset);
                    }
                    parent = subset;
                }
                continue;
            }

            auto sep = line.find("=");
            if (sep == String::npos)
                continue;

            auto key = trim(line.substr(0, sep));
            auto value = trim(line.substr(sep + 1));
            logV(domain, "[", key, "] = [", value, "]");

            subset->set(key, value);
        }

        return set;
    }
};

static Parser::Shared<IniParser> ini{"ini"};

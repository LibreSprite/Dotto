// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "../libs/tinyxml2/tinyxml2.h"

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <common/XML.hpp>
#include <log/Log.hpp>

class XmlParser : public Parser {
public:
    std::shared_ptr<XMLNode> recurse(tinyxml2::XMLNode* in) {
        if (auto element = in->ToElement()) {
            auto out = std::make_shared<XMLElement>();
            out->tag = element->Name();
            out->text = element->GetText() ?: "";
            for (auto attr = element->FirstAttribute(); attr; attr = attr->Next()) {
                out->attributes[attr->Name()] = attr->Value();
            }
            for (auto child = in->FirstChild(); child; child = child->NextSibling()) {
                if (auto node = recurse(child)) {
                    out->children.push_back(node);
                }
            }
            return out;
        } else if (auto text = in->ToText()) {
            auto out = std::make_shared<XMLNode>();
            out->text = text->Value();
            return out;
        }
        return nullptr;
    }

    Value parseFile(std::shared_ptr<File> file) override {
        tinyxml2::XMLDocument doc;
        if (doc.Parse(file->readTextFile().c_str()) != tinyxml2::XMLError::XML_SUCCESS) {
            logE("Error parsing XML file ", file->getUID());
            return nullptr;
        }
        return recurse(doc.FirstChild());
    }
};

static Parser::Shared<XmlParser> xml{"xml"};

// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>

class XMLNode {
public:
    virtual bool isElement() {return false;}
    virtual ~XMLNode(){}
    String text;
};

class XMLElement : public XMLNode {
public:
    String tag;
    HashMap<String, String> attributes;
    Vector<std::shared_ptr<XMLNode>> children;
    virtual bool isElement() {return true;}
};

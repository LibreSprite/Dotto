// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "Log.hpp"

class ConsoleLog : public Log {
public:
    void internalWrite(const char* string) override {
        if (string == std::string_view{"\n"}) {
            std::cout << std::endl;
        } else {
            std::cout << string;
        }
    }
};

static Log::Shared<ConsoleLog> logger{"stdout"};

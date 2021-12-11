// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <log/Log.hpp>

int main(int argc, const char* argv[]) {
    inject<Log> log{"stdout"};
    log->setGlobal();
    log->setLevel(Log::Level::VERBOSE); // TODO: Configure using args
    Log::write(Log::Level::VERBOSE, "Logger initialized");

    return 0;
}

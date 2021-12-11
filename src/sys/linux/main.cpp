// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <app/App.hpp>
#include <common/System.hpp>
#include <log/Log.hpp>

int main(int argc, const char* argv[]) {
    Log::setDefault("stdout");
    System::setDefault("sdl2");

    inject<App> app{"dotto"};
    app->boot(argc, argv);

    Log::write(Log::Level::VERBOSE, "Running Dotto!");
    while(app->run());

    Log::write(Log::Level::INFO, "Shutting down");
    return 0;
}

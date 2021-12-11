// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <log/Log.hpp>
#include <app/App.hpp>

int main(int argc, const char* argv[]) {
    Log::setDefault("stdout");
    inject<App> app{"dotto"};
    app->boot(argc, argv);
    while(app->run());
    Log::write(Log::Level::INFO, "Shutting down");
    return 0;
}

// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __WINDOWS__

#include <app/App.hpp>
#include <common/System.hpp>
#include <log/Log.hpp>

extern "C" int main(int argc, const char* argv[]) {
    Log::setDefault("stdout");
    inject<App> app{"dotto"};
    app->boot(argc, argv);

    logI("Running Dotto!");
    while(app->run());

    return 0;
}

#endif

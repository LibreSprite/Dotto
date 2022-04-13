// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(ANDROID)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <app/App.hpp>
#include <common/System.hpp>
#include <log/Log.hpp>

extern "C" __attribute__((visibility("default"))) int SDL_main(int argc, const char* argv[]) {
    Log::setDefault("logcat");
    inject<App> app{"dotto"};
    app->boot(argc, argv);
    logI("Running Dotto!");
    while(app->run());
    return 0;
}

#endif

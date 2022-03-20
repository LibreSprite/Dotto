// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef EMSCRIPTEN

#include <emscripten.h>

#include <app/App.hpp>
#include <common/System.hpp>
#include <log/Log.hpp>

int main(int argc, const char* argv[]) {
    Log::setDefault("stdout");
    static inject<App> app{"dotto"};
    app->boot(argc, argv);

    Log::write(Log::Level::VERBOSE, "Running Dotto!");
    emscripten_set_main_loop(+[]{app->run();}, 0, true);

    return 0;
}

#endif

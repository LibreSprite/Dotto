// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __linux__

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <app/App.hpp>
#include <common/System.hpp>
#include <log/Log.hpp>

String crashInfo = "¯\\_(ツ)_/¯";

void setCrashInfo(const String& info) {
    crashInfo = info;
}

void crashHandler(int sig) {
  void* frames[20];
  size_t size = backtrace(frames, sizeof(frames)/sizeof(frames[0]));
  fprintf(stderr, "Crashinfo: %s\n", crashInfo.c_str());
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(frames, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, const char* argv[]) {
    signal(SIGSEGV, crashHandler);

    Log::setDefault("stdout");
    inject<App> app{"dotto"};
    app->boot(argc, argv);

    logI("Running Dotto!");
    while(app->run());

    return 0;
}

#endif

// Copyright (c) 2022 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(USE_PROFILER)

#include "types.hpp"
#include "Profiler.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

static Profiler* current;
static U32 started;
static std::ofstream file{"profile.log"};
static std::stringstream infoheader;

struct Entry {
    std::chrono::duration<F64> selfTime = {};
    U32 hitCount = 0;
};

HashMap<const char*, Entry> entries;

Profiler::Profiler(const char* func) : parent{current},
                                       startTime{clock::now()},
                                       func{func},
                                       childTime{} {
    current = this;
}

Profiler::~Profiler() {
    if (started) {
        auto delta = (clock::now() - startTime);
        auto& entry = entries[func];
        entry.hitCount++;
        entry.selfTime += delta - childTime;
        if (parent) {
            parent->childTime += delta;
        }
        current = parent;
    }
}

void Profiler::info(std::string_view str) {
    infoheader << String{str} << std::endl;
}

void Profiler::start() {
    started = 1;
}

void Profiler::end() {
    if (!started || !file)
        return;
    started = 0;

    Vector<std::pair<const char*, Entry*>> entryVec;
    entryVec.reserve(entries.size());

    for (auto& [key, entry] : entries) {
        if (entry.hitCount) {
            entryVec.push_back(std::make_pair(key, &entry));
        }
    }

    std::sort(entryVec.begin(), entryVec.end(), [](auto& left, auto& right) {
        return right.second->selfTime < left.second->selfTime;
    });

    file << infoheader.str() << std::endl;
    infoheader.str("");

    for (auto& [key, entry] : entryVec) {
        auto selfTime = entry->selfTime.count();
        auto normalized = selfTime / entry->hitCount;
        file << normalized << ")\t" << key << ": " << entry->hitCount << " hits, " << selfTime << std::endl;
        entry->selfTime = std::chrono::duration<double>{};
        entry->hitCount = 0;
    }
    file << std::endl;
}

#endif

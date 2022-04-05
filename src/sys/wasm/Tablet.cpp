// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(EMSCRIPTEN)

#include <emscripten.h>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <log/Log.hpp>
#include <sys/NativeWindowPlugin.hpp>

class WASMTabletDriver : public NativeWindowPlugin {
public:
    void init() override {
        EM_ASM(
            const onPointerEvent = Module.cwrap("onPointerEvent", "", ["number", "number"]);
            Module.canvas.addEventListener("pointerdown", event => onPointerEvent(1, event.pressure));
            Module.canvas.addEventListener("pointermove", event => onPointerEvent(2, event.pressure));
            Module.canvas.addEventListener("pointerup", event => onPointerEvent(3, event.pressure));
            );
    }
};

extern "C" {
    void onPointerEvent(int event, F32 pressure) {
        msg::MouseMove::pressure = pressure;
    }
}

static NativeWindowPlugin::Shared<WASMTabletDriver> reg{"wasm-tablet-driver"};

#endif

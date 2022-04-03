// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(__linux__)

#include <X11/Xlib.h>

#define EASYTAB_IMPLEMENTATION
#include <EasyTab/easytab.h>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <log/Log.hpp>
#include <sys/NativeWindowPlugin.hpp>


class LinuxTabletDriver : public NativeWindowPlugin {
public:
    PubSub<XEvent> pub{this};
    EasyTabInfo* easyTab = nullptr;

    void init() {
        if (!display) {
            logE("No X11 display, disabling tablet support");
            return;
        }

        S32 count;
        auto devices = XListInputDevices(display, &count);
        if (!devices) {
            logI("No input devices");
            return;
        } else {
            for (int32_t i = 0; i < count; i++) {
                logV("Device ", i, ": ", devices[i].name);
            }
            XFreeDeviceList(devices);
        }

        if (EasyTab_Load(display, window) == EASYTAB_OK)
            easyTab = EasyTab;
    }

    void on(XEvent& event) {
        if (event.xany.window != window || !easyTab)
            return;
        EasyTab = easyTab;
        if (EasyTab_HandleEvent(&event) != EASYTAB_OK)
            return;
        msg::MouseMove::pressure = easyTab->Pressure;
    }

    ~LinuxTabletDriver() {
        if (easyTab) {
            EasyTab = easyTab;
            EasyTab_Unload(display);
        }
    }
};

static NativeWindowPlugin::Shared<LinuxTabletDriver> reg{"linux-tablet-driver"};

#endif

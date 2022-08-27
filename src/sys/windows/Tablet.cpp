// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(__WINDOWS__)

#define EASYTAB_IMPLEMENTATION
#include <EasyTab/easytab.h>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <log/Log.hpp>
#include <sys/NativeWindowPlugin.hpp>

class WindowsTabletDriver : public NativeWindowPlugin {
public:
    PubSub<msg::WinEvent> pub{this};
    EasyTabInfo* easyTab = nullptr;

    void init() {
        if (EasyTab_Load(window) == EASYTAB_OK) {
            easyTab = EasyTab;
            logI("Max pressure: ", EasyTab->MaxPressure, " cxs:", GetSystemMetrics(SM_CXSCREEN), " cxy:", GetSystemMetrics(SM_CYSCREEN));
        }
    }

    void on(msg::WinEvent& event) {
        if (event.hwnd != window || !easyTab)
            return;
        EasyTab = easyTab;
        if (EasyTab_HandleEvent(event.hwnd, event.msg, event.lParam, event.wParam) != EASYTAB_OK)
            return;
        msg::MouseMove::pressure = easyTab->Pressure;
    }

    ~WindowsTabletDriver() {
        if (easyTab) {
            EasyTab = easyTab;
            EasyTab_Unload();
        }
    }
};

static NativeWindowPlugin::Shared<WindowsTabletDriver> reg{"windows-tablet-driver"};

#endif

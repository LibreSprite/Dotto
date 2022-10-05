// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef USE_SDL2

#include <SDL.h>

#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/System.hpp>
#include <gui/Events.hpp>
#include <gui/Graphics.hpp>
#include <log/Log.hpp>

#include <SDL_events.h>
#include <SDL_syswm.h>

static const char* getKeyName(S32 code);

class SDL2System : public System {
public:
    Provides sys{this};
    ui::Node::Provides win{"sdl2Window", "window"};
    PubSub<msg::Copy, msg::PollPaste> pub{this};

    bool running = true;
    std::shared_ptr<ui::Node> root;
    ui::Node::Provides _root{root, "root"};
    std::unordered_set<String> pressedKeys;
    bool mapJoyhatToMouseWheel = false;
    bool mapJoyaxisToMouseWheel = false;
    bool invertMouseWheelX = false;
    bool invertMouseWheelY = false;
    U32 mouseState = 0;

    bool boot() override {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) != 0) {
            logE(SDL_GetError());
            return false;
        }

        SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
        inject<Config> config;
        mapJoyhatToMouseWheel = config->properties->get<bool>("map-joyhat-to-mousewheel");
        mapJoyaxisToMouseWheel = config->properties->get<bool>("map-joyaxis-to-mousewheel");
        invertMouseWheelX = config->properties->get<bool>("mouse-wheel-invert-x");
        invertMouseWheelY = config->properties->get<bool>("mouse-wheel-invert-y");

        if (mapJoyhatToMouseWheel || mapJoyaxisToMouseWheel) {
            SDL_JoystickEventState(SDL_ENABLE);
            U32 numJoysticks = SDL_NumJoysticks();
            for (U32 i=0; i<numJoysticks; ++i)
                SDL_JoystickOpen(i);
        }

        SDL_StartTextInput();

        root = inject<ui::Node>{"node"};
        root->processEvent(ui::AddToScene{root.get()});

        return true;
    }

    bool run() override {
        if (!running) return false;
        if (!root || root->getChildren().empty()) return false;
        pumpEvents();
        if (!running) return false;
        root->update();
        Graphics gfx;
        root->draw(0, gfx);
        return running;
    }

    const std::unordered_set<String>& getPressedKeys() override {
        return pressedKeys;
    }

    void setMouseCursorVisible(bool visible) override {
        SDL_ShowCursor(visible);
    }

    void on(msg::Copy& event) {
        auto str = event.value.toString();
        SDL_SetClipboardText(str.c_str());
    }

    void on(msg::PollPaste& event) {
        if (SDL_HasClipboardText() && event.value.empty()) {
            std::unique_ptr<char, decltype(&SDL_free)> text{SDL_GetClipboardText(), SDL_free};
            event.value = String{text.get()};
        }
    }

    void pumpEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_SYSWMEVENT:

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
                if (event.syswm.msg->subsystem == SDL_SYSWM_WINDOWS) {
                    auto& win = event.syswm.msg->msg.win;
                    pub(msg::WinEvent{
                            .hwnd = win.hwnd,
                            .msg = win.msg,
                            .wParam = win.wParam,
                            .lParam = win.lParam
                        });
                }
#endif

#if defined(SDL_VIDEO_DRIVER_X11)
                if (event.syswm.msg->subsystem == SDL_SYSWM_X11) {
                    pub(std::move(event.syswm.msg->msg.x11.event));
                }
#endif

                break;

            case SDL_MOUSEWHEEL:
                pub(msg::MouseWheel{
                        event.wheel.windowID,
                        (invertMouseWheelX ? -event.wheel.x : event.wheel.x),
                        (invertMouseWheelY ? -event.wheel.y : event.wheel.y)
                    });
                break;

            case SDL_MOUSEMOTION:{
                pub(msg::MouseMove{
                        event.motion.windowID,
                        event.motion.x,
                        event.motion.y,
                        mouseState
                    });
            }
                break;
            case SDL_MOUSEBUTTONUP:
                mouseState &= ~(1U << (event.button.button - 1));
                pub(msg::MouseUp{
                        event.button.windowID,
                        event.button.x,
                        event.button.y,
                        1U << (event.button.button - 1)
                    });
                msg::MouseMove::pressure = 0.0f;
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseState |= 1U << (event.button.button - 1);
                if (msg::MouseMove::pressure == 0.0f)
                    msg::MouseMove::pressure = 1.0f;
                pub(msg::MouseDown{
                        event.button.windowID,
                        event.button.x,
                        event.button.y,
                        1U << (event.button.button - 1)
                    });
                break;

            case SDL_JOYHATMOTION:
            {
                if (mapJoyhatToMouseWheel) {
                    int y = (event.jhat.value & 1) - !!(event.jhat.value & 4);
                    int x = (event.jhat.value & 2) - !!(event.jhat.value & 8);
                    pub(msg::MouseWheel{0, x, y});
                } else {
                    static const char* buttons[] = {
                        "UP",
                        "RIGHT",
                        "DOWN",
                        "LEFT"
                    };
                    static HashMap<U32, U32> hatstates;

                    U32 state = event.jhat.value;
                    U32 old = hatstates[event.jhat.hat];
                    hatstates[event.jhat.hat] = state;

                    for (U32 i = 0; i < 4; ++i) {
                        U32 flag = 1 << i;
                        if ((state & flag) == (old & flag))
                            continue;
                        bool pressed = state & flag;
                        String name = "HAT" + std::to_string(event.jhat.hat) + "_" + buttons[i];
                        if (pressed) {
                            pressedKeys.insert(name);
                            pub(msg::KeyDown{0, 0, name.c_str(), 0, pressedKeys});
                        } else {
                            pressedKeys.erase(name);
                            pub(msg::KeyUp{0, 0, name.c_str(), 0, pressedKeys});
                        }
                    }
                }
                break;
            }

            case SDL_JOYAXISMOTION:
            {
                if (mapJoyaxisToMouseWheel) {
                    pub(msg::MouseWheel{
                            0,
                            (event.jaxis.axis == 0 ? event.jaxis.value / 30000 : 0),
                            (event.jaxis.axis != 0 ? event.jaxis.value / 30000 : 0)
                        });
                }
                break;
            }

            case SDL_JOYBUTTONUP:
            {
                String name = "JOY" + std::to_string(event.jbutton.button);
                pressedKeys.erase(name);
                pub(msg::KeyUp{
                        0,
                        0,
                        name.c_str(),
                        0,
                        pressedKeys
                    });
                break;
            }
            case SDL_JOYBUTTONDOWN:
            {
                String name = "JOY" + std::to_string(event.jbutton.button);
                pressedKeys.insert(name);
                pub(msg::KeyDown{
                        0,
                        0,
                        name.c_str(),
                        0,
                        pressedKeys
                    });
                break;
            }


            case SDL_KEYUP: {
                auto name = getKeyName(event.key.keysym.sym);
                pressedKeys.erase(name);
                pub(msg::KeyUp{
                        event.key.windowID,
                        event.key.keysym.scancode,
                        name,
                        static_cast<U32>(event.key.keysym.sym),
                        pressedKeys
                    });
                break;
            }

            case SDL_KEYDOWN: {
                auto name = getKeyName(event.key.keysym.sym);
                pressedKeys.insert(name);
                pub(msg::KeyDown{
                        event.key.windowID,
                        event.key.keysym.scancode,
                        name,
                        static_cast<U32>(event.key.keysym.sym),
                        pressedKeys
                    });
                break;
            }

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                #if SDL_MAJOR_VERSION >= 2 && SDL_MINOR_VERSION >= 0 && SDL_PATCHLEVEL >= 18
                case SDL_WINDOWEVENT_ICCPROF_CHANGED:  // TODO: check ICC profile
                    break;
                #endif

                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    pressedKeys.clear();
                    break;

                case SDL_WINDOWEVENT_LEAVE:
                    pub(msg::MouseMove{event.window.windowID, -0xFFFFF, -0xFFFFF, 0});
                    break;

                case SDL_WINDOWEVENT_ENTER:
                case SDL_WINDOWEVENT_MOVED:
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_EXPOSED:
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    pub(msg::WindowMaximized{event.window.windowID});
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    pub(msg::WindowMinimized{event.window.windowID});
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    pub(msg::WindowMinimized{event.window.windowID});
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    pub(msg::WindowClosed{event.window.windowID});
                    break;
                default:
                    logV("Unknown SDL2 Window event: ", event.window.event);
                    break;
                }
                break;

            case SDL_TEXTINPUT:
                pub(msg::TextEvent{
                        event.text.windowID,
                        event.text.text,
                        pressedKeys
                    });
                break;

            case SDL_TEXTEDITING:
            case SDL_KEYMAPCHANGED:
                break;

            default:
                logV("Unknown SDL2 event: ", event.type);
                break;
            }
        }
    }

    ~SDL2System() {
        root.reset();
        SDL_Quit();
    }
};

System::Shared<SDL2System> sys{"new"};

static const char* getKeyName(S32 code) {
    switch (code) {
    case 0: return "UNKNOWN0";
    case '\r':  return "RETURN";
    case '\x1B':  return "ESCAPE";
    case '\b':  return "BACKSPACE";
    case '\t':  return "TAB";
    case ' ':  return "SPACE";
    case '!':  return "EXCLAIM";
    case '"':  return "QUOTEDBL";
    case '#':  return "HASH";
    case '%':  return "PERCENT";
    case '$':  return "DOLLAR";
    case '&':  return "AMPERSAND";
    case '\'':  return "QUOTE";
    case '(':  return "LEFTPAREN";
    case ')':  return "RIGHTPAREN";
    case '*':  return "ASTERISK";
    case '+':  return "PLUS";
    case ',':  return "COMMA";
    case '-':  return "MINUS";
    case '.':  return "PERIOD";
    case '/':  return "SLASH";
    case '0':  return "0";
    case '1':  return "1";
    case '2':  return "2";
    case '3':  return "3";
    case '4':  return "4";
    case '5':  return "5";
    case '6':  return "6";
    case '7':  return "7";
    case '8':  return "8";
    case '9':  return "9";
    case ':':  return "COLON";
    case ';':  return "SEMICOLON";
    case '<':  return "LESS";
    case '=':  return "EQUALS";
    case '>':  return "GREATER";
    case '?':  return "QUESTION";
    case '@':  return "AT";
    case '[':  return "LEFTBRACKET";
    case '\\':  return "BACKSLASH";
    case ']':  return "RIGHTBRACKET";
    case '^':  return "CARET";
    case '_':  return "UNDERSCORE";
    case '`':  return "BACKQUOTE";
    case 'a':  return "a";
    case 'b':  return "b";
    case 'c':  return "c";
    case 'd':  return "d";
    case 'e':  return "e";
    case 'f':  return "f";
    case 'g':  return "g";
    case 'h':  return "h";
    case 'i':  return "i";
    case 'j':  return "j";
    case 'k':  return "k";
    case 'l':  return "l";
    case 'm':  return "m";
    case 'n':  return "n";
    case 'o':  return "o";
    case 'p':  return "p";
    case 'q':  return "q";
    case 'r':  return "r";
    case 's':  return "s";
    case 't':  return "t";
    case 'u':  return "u";
    case 'v':  return "v";
    case 'w':  return "w";
    case 'x':  return "x";
    case 'y':  return "y";
    case 'z':  return "z";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK):  return "CAPSLOCK";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1):  return "F1";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2):  return "F2";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3):  return "F3";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4):  return "F4";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5):  return "F5";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6):  return "F6";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7):  return "F7";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8):  return "F8";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9):  return "F9";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10):  return "F10";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11):  return "F11";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12):  return "F12";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN):  return "PRINTSCREEN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK):  return "SCROLLLOCK";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE):  return "PAUSE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT):  return "INSERT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME):  return "HOME";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP):  return "PAGEUP";
    case '\x7F':  return "DELETE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END):  return "END";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN):  return "PAGEDOWN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT):  return "RIGHT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT):  return "LEFT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN):  return "DOWN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP):  return "UP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR):  return "NUMLOCKCLEAR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE):  return "KP_DIVIDE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY):  return "KP_MULTIPLY";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS):  return "KP_MINUS";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS):  return "KP_PLUS";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER):  return "KP_ENTER";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1):  return "KP_1";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2):  return "KP_2";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3):  return "KP_3";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4):  return "KP_4";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5):  return "KP_5";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6):  return "KP_6";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7):  return "KP_7";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8):  return "KP_8";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9):  return "KP_9";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0):  return "KP_0";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD):  return "KP_PERIOD";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION):  return "APPLICATION";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER):  return "POWER";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS):  return "KP_EQUALS";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13):  return "F13";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14):  return "F14";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15):  return "F15";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16):  return "F16";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17):  return "F17";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18):  return "F18";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19):  return "F19";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20):  return "F20";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21):  return "F21";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22):  return "F22";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23):  return "F23";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24):  return "F24";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE):  return "EXECUTE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP):  return "HELP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU):  return "MENU";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT):  return "SELECT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP):  return "STOP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN):  return "AGAIN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO):  return "UNDO";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT):  return "CUT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY):  return "COPY";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE):  return "PASTE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND):  return "FIND";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE):  return "MUTE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP):  return "VOLUMEUP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN):  return "VOLUMEDOWN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA):  return "KP_COMMA";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400):  return "KP_EQUALSAS400";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE):  return "ALTERASE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ):  return "SYSREQ";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL):  return "CANCEL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR):  return "CLEAR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR):  return "PRIOR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2):  return "RETURN2";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR):  return "SEPARATOR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT):  return "OUT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER):  return "OPER";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN):  return "CLEARAGAIN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL):  return "CRSEL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL):  return "EXSEL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00):  return "KP_00";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000):  return "KP_000";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR):  return "THOUSANDSSEPARATOR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR):  return "DECIMALSEPARATOR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT):  return "CURRENCYUNIT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT):  return "CURRENCYSUBUNIT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN):  return "KP_LEFTPAREN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN):  return "KP_RIGHTPAREN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE):  return "KP_LEFTBRACE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE):  return "KP_RIGHTBRACE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB):  return "KP_TAB";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE):  return "KP_BACKSPACE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A):  return "KP_A";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B):  return "KP_B";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C):  return "KP_C";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D):  return "KP_D";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E):  return "KP_E";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F):  return "KP_F";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR):  return "KP_XOR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER):  return "KP_POWER";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT):  return "KP_PERCENT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS):  return "KP_LESS";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER):  return "KP_GREATER";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND):  return "KP_AMPERSAND";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND):  return "KP_DBLAMPERSAND";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR):  return "KP_VERTICALBAR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR):  return "KP_DBLVERTICALBAR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON):  return "KP_COLON";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH):  return "KP_HASH";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE):  return "KP_SPACE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT):  return "KP_AT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM):  return "KP_EXCLAM";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE):  return "KP_MEMSTORE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL):  return "KP_MEMRECALL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR):  return "KP_MEMCLEAR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD):  return "KP_MEMADD";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT):  return "KP_MEMSUBTRACT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY):  return "KP_MEMMULTIPLY";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE):  return "KP_MEMDIVIDE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS):  return "KP_PLUSMINUS";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR):  return "KP_CLEAR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY):  return "KP_CLEARENTRY";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY):  return "KP_BINARY";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL):  return "KP_OCTAL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL):  return "KP_DECIMAL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL):  return "KP_HEXADECIMAL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL):  return "LCTRL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT):  return "LSHIFT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT):  return "LALT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI):  return "LGUI";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL):  return "RCTRL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT):  return "RSHIFT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT):  return "RALT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI):  return "RGUI";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE):  return "MODE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIONEXT):  return "AUDIONEXT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPREV):  return "AUDIOPREV";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOSTOP):  return "AUDIOSTOP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOPLAY):  return "AUDIOPLAY";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOMUTE):  return "AUDIOMUTE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIASELECT):  return "MEDIASELECT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WWW):  return "WWW";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MAIL):  return "MAIL";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALCULATOR):  return "CALCULATOR";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COMPUTER):  return "COMPUTER";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH):  return "AC_SEARCH";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME):  return "AC_HOME";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK):  return "AC_BACK";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD):  return "AC_FORWARD";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP):  return "AC_STOP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH):  return "AC_REFRESH";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS):  return "AC_BOOKMARKS";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSDOWN):  return "BRIGHTNESSDOWN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_BRIGHTNESSUP):  return "BRIGHTNESSUP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DISPLAYSWITCH):  return "DISPLAYSWITCH";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMTOGGLE):  return "KBDILLUMTOGGLE";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMDOWN):  return "KBDILLUMDOWN";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KBDILLUMUP):  return "KBDILLUMUP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EJECT):  return "EJECT";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP):  return "SLEEP";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP1):  return "APP1";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APP2):  return "APP2";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOREWIND):  return "AUDIOREWIND";
    case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AUDIOFASTFORWARD):  return "AUDIOFASTFORWARD";
    default: return "UNKNOWNX";
    }
}

#endif

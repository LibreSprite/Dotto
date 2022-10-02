// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef USE_SDL1

#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/System.hpp>
#include <gui/Events.hpp>
#include <gui/Graphics.hpp>
#include <log/Log.hpp>

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <sdl1/SDLGraphics.hpp>

static const char* getKeyName(S32 code);

class SDL1System : public System {
public:
    Provides sys{this};
    ui::Node::Provides win{"sdl1Window", "window"};
    PubSub<> pub{this};

    bool running = true;
    std::shared_ptr<ui::Node> root;
    ui::Node::Provides _root{root, "root"};
    std::unordered_set<String> pressedKeys;

    SDL_Surface *screen = nullptr;
    bool mapJoyhatToMouseWheel = false;
    bool mapJoyaxisToMouseWheel = false;
    bool invertMouseWheelX = false;
    bool invertMouseWheelY = false;

    bool boot() override {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
            logE(SDL_GetError());
            return false;
        }

        U32 screenWidth=320;
        U32 screenHeight=240;

#ifdef SDL_TOPSCR
        // screen = SDL_SetVideoMode(screenWidth, screenHeight, 0, SDL_SWSURFACE|/* * /SDL_TOPSCR | SDL_CONSOLEBOTTOM/*/SDL_DUALSCR/**/); // SDL_HWSURFACE);
        screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_SWSURFACE | SDL_BOTTOMSCR | SDL_CONSOLETOP | SDL_FITHEIGHT);
#else
        screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_HWSURFACE); // SDL_HWSURFACE);
#endif

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

        root = inject<ui::Node>{"node"};
        root->processEvent(ui::AddToScene{root.get()});
        root->load({
                {"width", std::to_string(screenWidth) + "px"},
                {"height", std::to_string(screenHeight) + "px"}
            });

        return true;
    }

    bool run() override {
        if (!running) return false;
        if (!root || root->getChildren().empty()) return false;
        pumpEvents();
        if (!running) return false;
        root->update();

        SDLGraphics gfx(screen);
        root->draw(0, gfx);
        SDL_Flip(screen);

        return running;
    }

    const std::unordered_set<String>& getPressedKeys() override {
        return pressedKeys;
    }

    void setMouseCursorVisible(bool visible) override {
        SDL_ShowCursor(visible);
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
                    pub(event.syswm.msg->msg.win);
                }
#endif

#if defined(SDL_VIDEO_DRIVER_X11)
                if (event.syswm.msg->subsystem == SDL_SYSWM_X11) {
                    pub(std::move(event.syswm.msg->event.xevent));
                }
#endif

                break;

            case SDL_MOUSEMOTION:
                pub(msg::MouseMove{0, event.motion.x, event.motion.y, event.motion.state});
                break;
            case SDL_MOUSEBUTTONUP:
                pub(msg::MouseUp{0, event.button.x, event.button.y, 1U << (event.button.button - 1)});
                break;
            case SDL_MOUSEBUTTONDOWN:
                pub(msg::MouseDown{0, event.button.x, event.button.y, 1U << (event.button.button - 1)});
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
                        0,
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
                        0,
                        event.key.keysym.scancode,
                        name,
                        static_cast<U32>(event.key.keysym.sym),
                        pressedKeys
                    });
                break;
            }

            default:
                logV("Unknown SDL1 event: ", event.type);
                break;
            }
        }
    }

    ~SDL1System() {
        root.reset();
        SDL_Quit();
    }
};

System::Shared<SDL1System> sys{"new"};

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
    case SDLK_CAPSLOCK:  return "CAPSLOCK";
    case SDLK_F1:  return "F1";
    case SDLK_F2:  return "F2";
    case SDLK_F3:  return "F3";
    case SDLK_F4:  return "F4";
    case SDLK_F5:  return "F5";
    case SDLK_F6:  return "F6";
    case SDLK_F7:  return "F7";
    case SDLK_F8:  return "F8";
    case SDLK_F9:  return "F9";
    case SDLK_F10:  return "F10";
    case SDLK_F11:  return "F11";
    case SDLK_F12:  return "F12";
    case SDLK_PRINT:  return "PRINTSCREEN";
    case SDLK_SCROLLOCK:  return "SCROLLLOCK";
    case SDLK_PAUSE:  return "PAUSE";
    case SDLK_INSERT:  return "INSERT";
    case SDLK_HOME:  return "HOME";
    case SDLK_PAGEUP:  return "PAGEUP";
    case '\x7F':  return "DELETE";
    case SDLK_END:  return "END";
    case SDLK_PAGEDOWN:  return "PAGEDOWN";
    case SDLK_RIGHT:  return "RIGHT";
    case SDLK_LEFT:  return "LEFT";
    case SDLK_DOWN:  return "DOWN";
    case SDLK_UP:  return "UP";
    case SDLK_NUMLOCK:  return "NUMLOCKCLEAR";
    case SDLK_KP_DIVIDE:  return "KP_DIVIDE";
    case SDLK_KP_MULTIPLY:  return "KP_MULTIPLY";
    case SDLK_KP_MINUS:  return "KP_MINUS";
    case SDLK_KP_PLUS:  return "KP_PLUS";
    case SDLK_KP_ENTER:  return "KP_ENTER";
    case SDLK_KP1:  return "KP_1";
    case SDLK_KP2:  return "KP_2";
    case SDLK_KP3:  return "KP_3";
    case SDLK_KP4:  return "KP_4";
    case SDLK_KP5:  return "KP_5";
    case SDLK_KP6:  return "KP_6";
    case SDLK_KP7:  return "KP_7";
    case SDLK_KP8:  return "KP_8";
    case SDLK_KP9:  return "KP_9";
    case SDLK_KP0:  return "KP_0";
    case SDLK_KP_PERIOD:  return "KP_PERIOD";
    case SDLK_POWER:  return "POWER";
    case SDLK_KP_EQUALS:  return "KP_EQUALS";
    case SDLK_F13:  return "F13";
    case SDLK_F14:  return "F14";
    case SDLK_F15:  return "F15";
    case SDLK_HELP:  return "HELP";
    case SDLK_MENU:  return "MENU";
    case SDLK_UNDO:  return "UNDO";
    case SDLK_SYSREQ:  return "SYSREQ";
    case SDLK_CLEAR:  return "CLEAR";
    case SDLK_LCTRL:  return "LCTRL";
    case SDLK_LSHIFT:  return "LSHIFT";
    case SDLK_LALT:  return "LALT";
    case SDLK_RCTRL:  return "RCTRL";
    case SDLK_RSHIFT:  return "RSHIFT";
    case SDLK_RALT:  return "RALT";
    case SDLK_MODE:  return "MODE";
    default: return "UNKNOWNX";
    }
}

#endif

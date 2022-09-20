// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <unordered_set>

#if defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX   /* don't define min() and max(). */
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <common/Value.hpp>
#include <common/Color.hpp>

namespace ui {
    class Node;
}

class Document;
class Cell;
class Palette;
class PropertySet;
class Selection;

namespace msg {

    #ifdef __WINDOWS__
    struct WinEvent {
        HWND hwnd;                  /**< The window for the message */
        UINT msg;                   /**< The type of message */
        WPARAM wParam;              /**< WORD message parameter */
        LPARAM lParam;              /**< LONG message parameter */
    };
    #endif

    class Message {
    public:
        virtual ~Message() = default;
        virtual Vector<String> toStrings(const String& name) {
            return {name};
        }
    };

    class BootComplete{};
    class Shutdown{};
    class RequestShutdown{};
    class Tick{};
    class PostTick{};

    class Flush{
        const Value& resource;
        bool held = false;
    public:
        Flush(const Value& resource) : resource{resource} {};

        void hold(const Value& other) {
            if (!held)
                held = other == resource;
        }

        bool isHeld() {
            return held;
        }
    };

    struct WindowMaximized {const U32 windowId;};
    struct WindowMinimized {const U32 windowId;};
    struct WindowRestored {const U32 windowId;};
    struct WindowClosed {const U32 windowId;};

    struct MouseMove {
        const U32 windowId;
        const S32 x, y;
        U32 buttons;
        static inline F32 pressure = 1.0f;
    };

    struct MouseWheel {
        const U32 windowId;
        S32 wheelX, wheelY;
    };

    struct MouseUp {
        const U32 windowId;
        S32 x, y;
        U32 buttons;
    };

    struct MouseDown {
        const U32 windowId;
        S32 x, y;
        U32 buttons;
    };

    struct Copy {
        Value value;
    };

    struct PollPaste {
        Value value;
    };

    struct TextEvent {
        const U32 windowId;
        const char* text;
        std::unordered_set<String>& pressedKeys;
    };

    struct KeyUp {
        const U32 windowId;
        U32 scancode;
        const char* keyName;
        U32 keycode;
        std::unordered_set<String>& pressedKeys;
    };

    struct KeyDown {
        const U32 windowId;
        U32 scancode;
        const char* keyName;
        U32 keycode;
        std::unordered_set<String>& pressedKeys;
    };

    struct BeginDrag {
        std::shared_ptr<ui::Node> target;
        S32 anchorX, anchorY;
    };

    struct EndDrag {
        std::shared_ptr<ui::Node> target;
    };

    struct InvalidateMetaMenu {
        std::shared_ptr<PropertySet> oldMeta;
        std::shared_ptr<PropertySet> newMeta;
    };

    struct PollActiveWindow {
        ui::Node* node = nullptr;
    };

    struct ActivateEditor : public Message {
        ui::Node* editor = nullptr;
        ActivateEditor(ui::Node* editor) : editor{editor} {}
    };

    struct PollActiveEditor : public Message {
        ui::Node* editor = nullptr;
    };

    struct ActivateTool : public Message {
        const String& tool;
        ActivateTool(const String& tool) : tool{tool} {}
        Vector<String> toStrings(const String& name) {return {name, tool};}
    };

    struct ActivateColor : public Message {
        const Color& color;
        ActivateColor(const Color& color) : color{color} {}
        Vector<String> toStrings(const String& name) {return {name, color};}
    };

    struct ActivateFrame : public Message {
        std::shared_ptr<Document> doc;
        U32 frame;
        ActivateFrame(std::shared_ptr<Document> doc, U32 frame) : doc{doc}, frame{frame} {}
    };

    struct ActivateLayer : public Message {
        std::shared_ptr<Document> doc;
        U32 layer;
        ActivateLayer(std::shared_ptr<Document> doc, U32 layer) : doc{doc}, layer{layer} {}
    };

    struct CloseDocument : public Message {
        Document* doc;
        CloseDocument(Document* doc) : doc{doc} {}
    };

    struct RenameDocument : public Message {
        std::shared_ptr<Document> doc;
        RenameDocument(std::shared_ptr<Document> doc) : doc{doc} {}
    };

    struct OpenDocument : public Message {
        std::shared_ptr<Document> doc;
        OpenDocument(std::shared_ptr<Document> doc) : doc{doc} {}
    };

    struct RequestActivateDocument : public Message {
        std::shared_ptr<Document> doc;
        RequestActivateDocument(std::shared_ptr<Document> doc) : doc{doc} {}
    };

    struct ActivateDocument : public Message {
        std::shared_ptr<Document> doc;
        ActivateDocument(std::shared_ptr<Document> doc) : doc{doc} {}
    };

    struct ResizeDocument : public Message {
        std::shared_ptr<Document> doc;
        ResizeDocument(std::shared_ptr<Document> doc) : doc{doc} {}
    };

    struct ActivateCell : public Message {
        std::shared_ptr<Cell> cell;
        ActivateCell(std::shared_ptr<Cell> cell) : cell{cell} {}
    };

    struct ChangePalette : public Message {
        std::shared_ptr<Palette> palette;
        ChangePalette(std::shared_ptr<Palette> palette) : palette{palette} {}
    };

    struct ModifyCell : public Message {
        std::shared_ptr<Cell> cell;
        ModifyCell(std::shared_ptr<Cell> cell) : cell{cell} {}
    };

    struct ModifyGroup : public Message {};

    struct PreModifySelection : public Message {
        Selection* selection;
        PreModifySelection(Selection* selection) : selection{selection} {}
    };
}

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <any>
#include <variant>
#include <vector>

enum class EventId : uint32_t {
    Boot = 0,
    PreUpdate = 1,
    Update = 2,
    Draw = 3,
    PostUpdate = 4,
    Resize = 5,
    MouseLeftDown = 6,
    MouseMiddleDown = 7,
    MouseRightDown = 8,
    MouseLeftUp = 9,
    MouseMiddleUp = 10,
    MouseRightUp = 11,
    MouseMove = 12,
    MaxEvent
};

#define DECL_IMPORT(name, type) inline __attribute__ ((section(".imports"))) volatile auto name = (type)(#name)

enum class MaterialId : uint32_t;
enum class NodeId : uint32_t;
enum class MeshId : uint32_t;
enum class SurfaceId : uint32_t;

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct Vector2 {
    float x, y;
};
struct Vector3 : public Vector2 {
    float z;
};
struct Vector4 : public Vector3 {
    float w;
};

// Model
DECL_IMPORT(getFloat, float (*)(const char* key, float defval));
DECL_IMPORT(getString, const char* (*)(const char* key, const char* defval));

// Graphics
DECL_IMPORT(createNode, NodeId (*)());
DECL_IMPORT(Node_setVisible, void (*)(NodeId, bool));
DECL_IMPORT(Node_getVisible, bool (*)(NodeId));
DECL_IMPORT(Node_getComponentCount, uint32_t (*)(NodeId));
DECL_IMPORT(Node_getMesh, MeshId (*)(NodeId, uint32_t));
DECL_IMPORT(Node_getMaterial, MaterialId (*)(NodeId, uint32_t));
DECL_IMPORT(Node_addComponent, void (*)(NodeId, MeshId, MaterialId));
DECL_IMPORT(Node_setPosition, void (*)(NodeId, float, float, float));
DECL_IMPORT(Node_setScale, void (*)(NodeId, float, float, float));
DECL_IMPORT(Node_rotate, void (*)(NodeId, float, float, float, float));
DECL_IMPORT(Node_setRotation, void (*)(NodeId, float, float, float, float));

DECL_IMPORT(createMesh, MeshId (*)());
DECL_IMPORT(Mesh_addAttributeFloat, void (*)(MeshId, const char*));
DECL_IMPORT(Mesh_addAttributeVector2, void (*)(MeshId, const char*));
DECL_IMPORT(Mesh_addAttributeVector3, void (*)(MeshId, const char*));
DECL_IMPORT(Mesh_addAttributeVector4, void (*)(MeshId, const char*));
DECL_IMPORT(Mesh_pushAttribute, void (*)(MeshId, const char*, const void*, const void*));
DECL_IMPORT(Mesh_clearElements, void (*)(MeshId));
DECL_IMPORT(Mesh_pushElements, void (*)(MeshId, const void*, const void*));

DECL_IMPORT(createMaterial, MaterialId (*)(const char*));
DECL_IMPORT(Material_addTag, void(*)(MaterialId, const char*));
DECL_IMPORT(Material_setTexture, void(*)(MaterialId, const char*, SurfaceId));
DECL_IMPORT(Material_setUniformFloat, void(*)(MaterialId, const char*, const float&));
DECL_IMPORT(Material_setUniformVector2, void(*)(MaterialId, const char*, const Vector2&));
DECL_IMPORT(Material_setUniformVector3, void(*)(MaterialId, const char*, const Vector3&));
DECL_IMPORT(Material_setUniformVector4, void(*)(MaterialId, const char*, const Vector4&));

DECL_IMPORT(createSurface, SurfaceId (*)(uint32_t, uint32_t));
DECL_IMPORT(Surface_resize, void (*)(SurfaceId, uint32_t, uint32_t));
DECL_IMPORT(Surface_fill, void (*)(SurfaceId, uint32_t, uint32_t, uint32_t, uint32_t));
DECL_IMPORT(Surface_write, void (*)(SurfaceId, int32_t, int32_t, uint32_t, uint32_t, Color*));
DECL_IMPORT(Surface_width, uint32_t (*)(SurfaceId));
DECL_IMPORT(Surface_height, uint32_t (*)(SurfaceId));

// Events
DECL_IMPORT(pollEvents, EventId (*)());
DECL_IMPORT(enableEvent, void (*)(EventId));

// Messaging
DECL_IMPORT(popMessage, uint32_t (*)());
DECL_IMPORT(getMessageArg, const char* (*)(uint32_t));

inline long parseInt(std::string_view str) {
    return std::strtol(str.data(), nullptr, 0);
}

class MessageListener {
    struct Handler {
        std::function<void(std::vector<std::string>&)> callback;
        MessageListener* listener;
        bool autoRemove;
    };
    static inline std::unordered_map<std::size_t, std::vector<std::unique_ptr<Handler>>> messageHandlers;

public:
    template <typename Callback>
    void attach(std::size_t messageId, Callback&& callback, bool autoRemove = true) {
        auto handler = std::make_unique<Handler>();
        handler->callback = std::forward<Callback>(callback);
        handler->autoRemove = autoRemove;
        handler->listener = this;
        messageHandlers[messageId].push_back(std::move(handler));
    }

    void detach(std::size_t messageId) {
        auto mapIt = messageHandlers.find(messageId);
        if (mapIt == messageHandlers.end())
            return;
        for (auto vecIt = mapIt->second.begin(); vecIt != mapIt->second.end();) {
            if ((*vecIt)->listener == this) {
                vecIt = mapIt->second.erase(vecIt);
            } else {
                ++vecIt;
            }
        }
        if (mapIt->second.empty())
            messageHandlers.erase(mapIt);
    }

    ~MessageListener() {
        for (auto mapIt = messageHandlers.begin(); mapIt != messageHandlers.end();) {
            for (auto vecIt = mapIt->second.begin(); vecIt != mapIt->second.end();) {
                if ((*vecIt)->listener == this) {
                    vecIt = mapIt->second.erase(vecIt);
                } else {
                    ++vecIt;
                }
            }
            if (mapIt->second.empty()) {
                mapIt = messageHandlers.erase(mapIt);
            } else {
                ++mapIt;
            }
        }
    }

    std::vector<std::string> poll() {
        std::vector<std::string> msg;
        while (true) {
            uint32_t argc = popMessage();
            msg.resize(argc);
            if (!argc)
                break;
            for (uint32_t i = 0; i < argc; ++i)
                msg[i] = getMessageArg(i + 1);
            auto handlerId = parseInt(msg[0]);
            auto mapIt = messageHandlers.find(handlerId);
            if (mapIt == messageHandlers.end())
                break;
            for (auto vecIt = mapIt->second.begin(); vecIt != mapIt->second.end();) {
                auto& handler = **vecIt;
                handler.callback(msg);
                if (handler.autoRemove) {
                    vecIt = mapIt->second.erase(vecIt);
                } else {
                    ++vecIt;
                }
            }
            if (mapIt->second.empty()) {
                messageHandlers.erase(mapIt);
            }
        }
        return msg;
    }
};

// System
DECL_IMPORT(getId, uint32_t (*)());
DECL_IMPORT(yield, void (*)());
DECL_IMPORT(vmOpen, int (*)(const char *, const char *));
DECL_IMPORT(vmLSeek, uint32_t(*)(int, int, int));
DECL_IMPORT(vmClose, void (*)(uint32_t));
DECL_IMPORT(vmWrite, int (*)(uint32_t, const uint8_t*, uint32_t));
DECL_IMPORT(vmRead, int (*)(uint32_t, const uint8_t*, uint32_t));
DECL_IMPORT(vmExit, void (*)(uint32_t));
DECL_IMPORT(vmSystem, uint32_t (*)(const char*));

inline float get(const char* key, float defval) {return getFloat(key, defval);}
inline const char* get(const char* key, const char* defval) {return getString(key, defval);}

#pragma once

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
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

// Model
DECL_IMPORT(getFloat, float (*)(const char* key, float defval));
DECL_IMPORT(getString, const char* (*)(const char* key, const char* defval));

// Graphics
DECL_IMPORT(createRenderable, NodeId (*)());
DECL_IMPORT(Node_getComponentCount, uint32_t (*)(NodeId));
DECL_IMPORT(Node_getMesh, MeshId (*)(NodeId, uint32_t));
DECL_IMPORT(Node_getMaterial, MaterialId (*)(NodeId, uint32_t));
DECL_IMPORT(Node_addComponent, void (*)(NodeId, MeshId, MaterialId));
DECL_IMPORT(Node_setPosition, void (*)(NodeId, float, float, float));
DECL_IMPORT(Node_rotate, void (*)(NodeId, float, float, float, float));

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

// Events
DECL_IMPORT(pollEvents, EventId (*)());
DECL_IMPORT(enableEvent, void (*)(EventId));

// Messaging
DECL_IMPORT(popMessage, uint32_t (*)());
DECL_IMPORT(getMessageArg, const char* (*)(uint32_t));

inline std::vector<std::string> pollMessage() {
    std::vector<std::string> msg;
    uint32_t argc = popMessage();
    msg.reserve(argc);
    for (uint32_t i = 1; i < argc; ++i)
	msg.push_back(getMessageArg(i));
    return msg;
}

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

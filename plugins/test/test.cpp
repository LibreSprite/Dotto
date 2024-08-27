#include "API.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sys/unistd.h>
#include <math.h>
#include <fmt.hpp>

static constexpr float PI {3.1415926535897932384626433f};

using namespace std::literals;

int main(int argc, const char* argv[]) {
    MessageListener msg;
    // auto tex = createSurface(256, 256);
    // Surface_fill(tex, 32, 128, 255, 255);

    NodeId obj{};
    msg.attach(message("parseobj {} {:#x}", "data/craft_speederA.obj", getpid()), [&](auto& msg){
        obj = (NodeId) parseInt(msg[1]);
        Node_setPosition(obj, 0, 0, 10);
        Node_setVisible(obj, true);
        log("node id = {:#x}", int(obj));
    });

    NodeId rocket{};
    msg.attach(message("parsestl {} {:#x}", "data/rocket.stl", getpid()), [&](auto& msg){
        rocket = (NodeId) parseInt(msg[1]);
        Node_setPosition(rocket, 0, -300, 600);
        log("node id = {:#x}", int(rocket));
    });

    std::vector<float> position {
        -.5f, -.5f,  0.0f,
        0.5f, -.5f,  0.0f,
        -.5f, 0.5f,  0.0f,
        0.5f, 0.5f, 0.0f,
    };

    std::vector<float> color {
        1.0f,  0.5f,  0.0f, 1.0f,
        1.0f,  1.0f,  1.0f, 1.0f,
        1.0f,  0.0f,  0.0f, 1.0f,
        1.0f,  0.0f,  0.0f, 1.0f,
    };

    std::vector<float> uv {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    std::vector<uint32_t> elements {
        0, 1, 2,
        1, 3, 2
    };

    auto node = createNode();
    Node_setVisible(node, false);

    auto mesh = createMesh();
    Mesh_addAttributeVector3(mesh, "position");
    Mesh_addAttributeVector2(mesh, "uv");
    Mesh_addAttributeVector4(mesh, "color");
    Mesh_pushAttribute(mesh, "position", position.data(), position.data() + position.size());
    Mesh_pushAttribute(mesh, "uv", uv.data(), uv.data() + uv.size());
    Mesh_pushAttribute(mesh, "color", color.data(), color.data() + color.size());
    Mesh_pushElements(mesh, elements.data(), elements.data() + elements.size());

    auto mat = createMaterial("3d texture-color");
    // Material_setTexture(mat, "texDiffuse", tex);

    Node_addComponent(node, mesh, mat);
    msg.attach(message("parseqoi {} {}", "data/kodim10.qoi", getpid()), [=](auto& msg){
        auto tex = (SurfaceId) std::strtol(msg[1].c_str(), nullptr, 0);
        Material_setTexture(mat, "texDiffuse", tex);
        float aspect = float(Surface_width(tex)) / Surface_height(tex);
        Node_setScale(node, 10 * aspect, 10, 1);
        Node_setVisible(node, true);
    });

    enableEvent(EventId::MouseLeftDown);
    enableEvent(EventId::MouseRightDown);
    enableEvent(EventId::MouseMove);

    float x{}, y{}, z{20};
    float t{};

    while(true) {
	if (auto unknownMsg = msg.poll(); !unknownMsg.empty()) {
	    std::string acc;
	    for (auto& s : unknownMsg)
		acc += " " + s;
	    log("unknownMsg:{}", acc);
	}

        for (EventId id; (id = pollEvents()) != EventId::MaxEvent;) {
            if (id == EventId::MouseLeftDown) {
                printf("exiting\n");
                exit(0);
            }
            if (id == EventId::MouseRightDown)
                printf("right click\n");
            if (id == EventId::MouseMove && int(obj)) {
                Node_rotate(obj, 0.001f * get("mouseDeltaY", 0.0f), 1, 0, 0);
                Node_rotate(obj, 0.001f * get("mouseDeltaX", 0.0f), 0, 1, 0);
            }
        }

	t += 0.001f;
	Node_setPosition(node, x, sinf(t) * 2, z);

        Node_setRotation(rocket, -PI/2.0f, 1, 0, 0);
        Node_rotate(rocket, t, 0, 0, 1);

        yield();
    }
}

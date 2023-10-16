#include "API.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sys/unistd.h>
#include <math.h>
#include <fmt.hpp>

using namespace std::literals;

int main(int argc, const char* argv[]) {
    auto jpgparsejob = message("parsepng {} {}", "test.png", getpid());

    auto tex = createSurface(256, 256);
    Surface_fill(tex, 32, 128, 255, 255);

    // system(("parseobj file.obj " + std::to_string(getpid())).c_str());
    auto parsejob = message("parseobj {} {:#x}", "craft_speederA.obj", getpid());

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

    auto node = createRenderable();
    auto mesh = createMesh();
    Mesh_addAttributeVector3(mesh, "position");
    Mesh_addAttributeVector2(mesh, "uv");
    Mesh_addAttributeVector4(mesh, "color");
    Mesh_pushAttribute(mesh, "position", position.data(), position.data() + position.size());
    Mesh_pushAttribute(mesh, "uv", uv.data(), uv.data() + uv.size());
    Mesh_pushAttribute(mesh, "color", color.data(), color.data() + color.size());
    Mesh_pushElements(mesh, elements.data(), elements.data() + elements.size());

    auto mat = createMaterial("3d texture-color");

    Node_addComponent(node, mesh, mat);

    enableEvent(EventId::MouseLeftDown);
    enableEvent(EventId::MouseRightDown);
    enableEvent(EventId::MouseMove);

    float x{}, y{}, z{5};
    float t{};

    NodeId obj;

    while(true) {
	if (auto msg = pollMessage(); !msg.empty()) {
	    std::string acc;
	    for (auto& s : msg)
		acc += " " + s;
	    log("msg:{}", acc);
	    auto i = std::strtol(msg[0].c_str(), nullptr, 0);
	    if (i == parsejob) {
		obj = (NodeId) std::strtol(msg[1].c_str(), nullptr, 0);
		Node_setPosition(obj, 0, 0, 10);
		log("node id = {:#x}", int(obj));
	    }
	    if (i == jpgparsejob) {
		auto tex = (SurfaceId) std::strtol(msg[1].c_str(), nullptr, 0);
		Material_setTexture(mat, "texDiffuse", tex);
		float aspect = float(Surface_width(tex)) / Surface_height(tex);
		Node_setScale(node, 2 * aspect, 2, 1);
	    }
	}

        for (EventId id; (id = pollEvents()) != EventId::MaxEvent;) {
            if (id == EventId::MouseLeftDown) {
                printf("exiting\n");
                exit(0);
            }
            if (id == EventId::MouseRightDown)
                printf("right click\n");
            if (id == EventId::MouseMove)
                log("move {}, {}", (int)get("mouseX", 0.0f), (int)get("mouseY", 0.0f));
        }

	t += 0.001f;
	Node_setPosition(node, x, sinf(t) * 2, z);

	if (int(obj)) {
	    Node_rotate(obj, 0.001f, 1, 0, 0);
	    Node_rotate(obj, 0.002f, 0, 1, 0);
	}

        yield();
    }
}

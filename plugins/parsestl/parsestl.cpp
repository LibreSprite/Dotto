#include <fmt.hpp>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <vector>

using namespace std::literals;

std::ifstream input;

std::uint8_t u8() {
    std::uint8_t b;
    input.read(reinterpret_cast<char*>(&b), sizeof(b));
    return b;
}

std::uint32_t u16() {
    std::uint16_t b;
    input.read(reinterpret_cast<char*>(&b), sizeof(b));
    return b;
}

std::uint32_t u32() {
    std::uint32_t b;
    input.read(reinterpret_cast<char*>(&b), sizeof(b));
    return b;
}

float f32() {
    float b;
    input.read(reinterpret_cast<char*>(&b), sizeof(b));
    return b;
}

template<typename T, typename ... Args> void push(T& c, Args&& ... args) {
    ((c.push_back(args)), ...);
}

NodeId loadSTL(const char* name) {
    input = std::ifstream{name, std::fstream::binary};
    if (!input) {
        printf("Could not open file: %s\n", name);
	return NodeId(0);
    }

    auto node = createNode();
    auto defmat = createMaterial("3d normal-color");
    auto mesh = createMesh();
    Node_addComponent(node, mesh, defmat);
    Material_setUniformVector4(defmat, "globalDiffuse", {0.0f, 0.5f, 1.0f, 1.0f});

    for (auto i = 0; i < 80; ++i)
        u8();

    auto triangleCount = u32();
    std::vector<float> vertices, normals;
    for (uint32_t triangle = 0; triangle < triangleCount; ++triangle) {
        if (input.eof())
            break;
        float n1 = f32();
        float n2 = f32();
        float n3 = f32();
        push(normals, n1, n2, n3);
        push(vertices, f32(), f32(), f32());
        push(normals, n1, n2, n3);
        push(vertices, f32(), f32(), f32());
        push(normals, n1, n2, n3);
        push(vertices, f32(), f32(), f32());
        auto attrib = u16();
    }
    Mesh_addAttributeVector3(mesh, "position");
    Mesh_pushAttribute(mesh, "position", vertices.data(), vertices.data() + vertices.size());
    Mesh_addAttributeVector3(mesh, "normal");
    Mesh_pushAttribute(mesh, "normal", normals.data(), normals.data() + normals.size());

    return node;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
	printf("parsestl error: expected 2 arguments, got %d.\n", argc);
	return 1;
    }
    message("{} {:#x} {:#x} node", argv[1], getpid(), loadSTL(argv[0]));
    return 0;
}

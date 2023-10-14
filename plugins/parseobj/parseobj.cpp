#include "../fmt.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <vector>

#include "obj.hpp"

using namespace std::literals;

int main(int argc, const char* argv[]) {
    if (argc < 2) {
	printf("parseobj error: expected 2 arguments, got %d.\n", argc);
	return 1;
    }
    std::string answer = fmt("{} {} ", argv[1], getpid());

    objl::Loader loader;
    if (!loader.LoadFile(argv[0])) {
	system((answer + "0 \"Could not open file\"").c_str());
	return 1;
    }

    auto node = createRenderable();
    auto defmat = createMaterial("3d vertex-color");

    for (auto& src : loader.LoadedMeshes) {
	std::vector<float> buffer;
	auto mesh = createMesh();
	Node_addComponent(node, mesh, defmat);

	Mesh_pushElements(mesh, src.Indices.data(), src.Indices.data() + src.Indices.size());

	Mesh_addAttributeVector3(mesh, "position");
	for (auto& vtx : src.Vertices) {
	    buffer.push_back(vtx.Position.X);
	    buffer.push_back(vtx.Position.Y);
	    buffer.push_back(vtx.Position.Z);
	}
	Mesh_pushAttribute(mesh, "position", buffer.data(), buffer.data() + buffer.size());
	buffer.clear();

	Mesh_addAttributeVector3(mesh, "normal");
	for (auto& vtx : src.Vertices) {
	    buffer.push_back(vtx.Normal.X);
	    buffer.push_back(vtx.Normal.Y);
	    buffer.push_back(vtx.Normal.Z);
	}
	Mesh_pushAttribute(mesh, "normal", buffer.data(), buffer.data() + buffer.size());
	buffer.clear();

	Mesh_addAttributeVector2(mesh, "uv");
	for (auto& vtx : src.Vertices) {
	    buffer.push_back(vtx.TextureCoordinate.X);
	    buffer.push_back(vtx.TextureCoordinate.Y);
	}
	Mesh_pushAttribute(mesh, "uv", buffer.data(), buffer.data() + buffer.size());
	buffer.clear();

        Mesh_addAttributeVector4(mesh, "color");
	for (auto& vtx : src.Vertices) {
	    buffer.push_back(src.MeshMaterial.Kd.X);
	    buffer.push_back( src.MeshMaterial.Kd.Y);
	    buffer.push_back(src.MeshMaterial.Kd.Z);
	    buffer.push_back(1.0f);
	}
	Mesh_pushAttribute(mesh, "color", buffer.data(), buffer.data() + buffer.size());
	buffer.clear();
    }

    system((answer + std::to_string((int)node)).c_str());
    return 0;
}

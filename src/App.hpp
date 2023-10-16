#pragma once

#include "Events.hpp"
#include "Index.hpp"
#include "MainThread.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Node.hpp"
#include "String.hpp"
#include "Surface.hpp"
#include "VMImpl.hpp"
#include "VMPool.hpp"
#include "Vector.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <ios>
#include <memory>
#include <string>

template <typename Log, typename Graphics>
class App {
public:
    using GraphicsType = Graphics;
    using VMType = VMImpl;
    using VMPoolType = VMPool<Log, VMType>;

    Index<VMType*> vms{0x10000000};
    Index<Node*> nodeIndex{0x20000000};
    Index<Mesh*> meshIndex{0x30000000};
    Index<Material*> materialIndex{0x40000000};
    Index<std::shared_ptr<Surface>> textureIndex{0x50000000};

    Log log;
    Model model;
    GraphicsType gfx;
    Scene scene;
    VMPoolType vmpool;

    App() {
	Model::main = &model;
	Scene::main = &scene;
    }

    void boot() {
        model.parse(readTextFile(model.get("main.settings", "settings.ini")));
	Surface::maxWidth = model.get("main.max-image-width", 4*1024.0f);
	Surface::maxHeight = model.get("main.max-image-height", 4*1024.0f);
        emit(EventId::Boot);

	std::vector<std::string> mainArgs;
	mainArgs.push_back(model.get("main.plugin", "boot.bin"));
	for (int i = 0; true; ++i) {
	    auto arg = model.get("main.args." + std::to_string(i), std::string{});
	    if (arg.empty())
		break;
	    mainArgs.push_back(arg);
	}

	try {
	    bootVM(std::move(mainArgs));
	} catch (std::exception& ex) {
	    log("Exception: ", ex.what());
	}
    }

    uint32_t bootVM(std::vector<std::string>&& parts) {
	if (parts.empty()) {
	    return 0;
	}
	auto folder = "./plugins/" + parts[0] + "/";
	auto data = readBinaryFile(folder + parts[0] + ".drt");
	if (data.empty())
	    return 0;
	if (model.get(parts[0] + ".settingsLoaded", 0.0f) == 0.0f) {
	    model.parse(readTextFile(folder + "settings.ini"));
	    model.set(parts[0] + ".settingsLoaded", 1.0f);
	}
	auto ramSize = model.get(parts[0] + ".reserve-ram-max", 0.0f);
        auto vm = createVM();
	vm->speed = std::max(0.1f, model.get(parts[0] + ".speed", 1.0f)) * 1024.0f * 1024.0f;
        vm->boot(data, int(ramSize)*1024*1024);
	parts.erase(parts.begin());
	if (!parts.empty()) {
	    vm->message(std::move(parts));
	}
        vmpool.add(vm);
        return *vm->key;
    }

    std::shared_ptr<VMType> createVM() {
        auto vm = std::make_shared<VMType>(this);
        vm->addAPI({
                {"vmSystem", [&](const VM::Args& arg) {
		    arg.result = 0;
		    try {
			auto parts = parseCommandLine(arg.get<std::string>(0));
			if (parts.empty() || parts[0].empty())
			    return;
			uint32_t vmid = strtoul(parts[0].c_str(), nullptr, 0);
			if (auto vm = vms.find(vmid)) {
			    (*vm)->message(std::move(parts));
			    arg.result = vmid;
			} else {
			    arg.result = bootVM(std::move(parts));
			}
		    } catch (std::exception&) {
		    }
                }},
            });
        return vm;
    }

    bool running() {
        return gfx.running && !vmpool.empty();
    }

    int step = 0;

    void update() {
	while (true) {
	    if (vmpool.wait())
		return;
	    runMainThreadCallbacks();
	    gc();
	    switch (step) {
	    case 0: emit(EventId::PreUpdate); break;
	    case 1: emit(EventId::Update); break;
	    case 2: emit(EventId::Draw); break;
	    case 3: emit(EventId::PostUpdate); break;
	    case 4: step = 0; return;
	    }
	    step++;
	}
    }

    ON(Resize) {
	scene.resize(gfx.width(), gfx.height());
    }
};

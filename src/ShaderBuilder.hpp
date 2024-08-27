#pragma once

#include "Model.hpp"

#include <memory>

class ShaderBuilder {
    std::vector<std::string> searchPaths;

    std::pair<std::string, std::string> findShaderSegment(const std::string& name) {
	using namespace std;
        for (auto& path : searchPaths) {
            auto full = path + "." + name;
            auto segment = getShaderSegment(full);
            if (!segment.empty())
                return {full, segment};
        }
        return {};
    }

    std::string getShaderSegment(const std::string tag) {
	return Model::root.get(tag, "");
    }

public:
    void addSearchPath(const std::string& path) {
        searchPaths.push_back("material." + path);
    }

    std::string processShaderSource(const std::string type, const std::set<std::string>& tags) {
	using Tag = std::pair<std::string, std::string>;
	std::vector<Tag> parts;
	auto activeTag = ~std::size_t{};

	std::vector<std::string> tagQueue {type};
	for (auto& tag : tags)
	    tagQueue.push_back(tag + "." + type);

	for (auto& tag : tagQueue) {
	    auto [fullPath, src] = findShaderSegment(tag);
	    auto tagSrc = split(src, "\n");
	    for (auto& line : tagSrc) {
		auto trimmed = trim(line);
		if (!trimmed.empty() && trimmed.back() == '>') {
		    if (trimmed.find("//section<") == 0) {
			auto sectionName = trimmed.substr(10, trimmed.size() - 11);
			std::size_t targetTag = 0;
			auto max = parts.size();
			for (; targetTag < max; ++targetTag) {
			    if (parts[targetTag].first == sectionName)
				break;
			}
			if (targetTag >= max) {
			    targetTag = activeTag + 1;
			    auto it = parts.begin();
			    std::advance(it, targetTag);
			    parts.insert(it, {sectionName, {}});
			} else {
			    line = "// *** " + fullPath + " ***";
			}
			activeTag = targetTag;
			continue;
		    }
		}
		if (activeTag < parts.size()) {
		    parts[activeTag].second += line + (" // " + fullPath) + "\n";
		} else {
		    LOG("skipping ", activeTag, "[", line, "]");
		}
	    }
	}

	std::string src;
	for (auto& tag : parts) {
	    // src += "// begin " + tag.first + "\n";
	    src += tag.second;
	}
	return src;
    }

};

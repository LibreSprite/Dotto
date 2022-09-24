// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/PropertySet.hpp>
#include <common/Writer.hpp>

class IniWriter : public Writer {
public:
    HashSet<PropertySet*> done;

    void write(fs::File& file, std::shared_ptr<PropertySet> props, Vector<std::string_view>& ns) {
        bool wroteHeader = false;

        struct Entry {
            std::string_view key;
            std::shared_ptr<Value> val;
            bool prop;
        };
        Vector<Entry> entries;

        for (auto& [key, val] : props->getMap()) {
            entries.push_back({
                    key,
                    val,
                    val->has<std::shared_ptr<PropertySet>, true>()
                });
        }

        std::sort(entries.begin(), entries.end(), [](Entry& a, Entry& b){
            if (a.prop != b.prop)
                return a.prop < b.prop;
            return a.key < b.key;
        });

        for (auto& entry : entries) {
            if (auto cps = entry.prop ? entry.val->get<std::shared_ptr<PropertySet>>() : nullptr) {
                if (done.count(cps.get())) {
                    file.write("# circular ref: ");
                    file.write(entry.key);
                    file.write("\n");
                    continue;
                }
                done.insert(cps.get());
                ns.push_back(entry.key);
                write(file, cps, ns);
                ns.pop_back();
            } else if (String str = *entry.val; !str.empty()) {
                if (!wroteHeader) {
                    bool first = true;
                    for (auto& s : ns) {
                        file.write(first ? "\n[" : ":");
                        first = false;
                        file.write(s);
                    }
                    if (!first) {
                        file.write("]\n");
                    }
                    wroteHeader = true;
                }
                file.write(entry.key);
                file.write(" = ");
                file.write(std::regex_replace(str.c_str(), std::regex("\n"), "\u2028"));
                file.write("\n");
            }
        }
    }
    
    bool writeFile(std::shared_ptr<fs::File> file, const Value& data) override {
        if (!data.has<std::shared_ptr<PropertySet>>())
            return false;
        done.clear();
        logV("Writing INI file in:", file->getUID());
        Vector<std::string_view> ns;
        write(*file, data, ns);
        return true;
    }
};

static Writer::Shared<IniWriter> ini{"ini", {"*.ini", typeid(std::shared_ptr<PropertySet>).name()}};

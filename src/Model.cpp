#include "Model.hpp"
#include "String.hpp"

#include <memory>
#include <cstdlib>
#include <mutex>
#include <shared_mutex>
#include <variant>

void Model::print(std::vector<std::string>* section) {
    if (!section) {
	std::vector<std::string> s;
	print(&s);
	return;
    }
    for (auto& entry : values) {
	if (auto child = std::get_if<std::shared_ptr<Model>>(&entry.second)) {
	    section->push_back(entry.first);
	    printf("[%s]\n", join(*section, ".").c_str());
	    (*child)->print(section);
	    section->pop_back();
	    printf("[%s]\n", join(*section, ".").c_str());
	} else if (auto f = std::get_if<float>(&entry.second)) {
	    printf("%s = %f\n", entry.first.c_str(), *f);
	} else if (auto s = std::get_if<std::string>(&entry.second)) {
	    printf("%s =`%s`\n", entry.first.c_str(), s->c_str());
	} else if (auto u = std::get_if<Undefined>(&entry.second)) {
	    printf("%s undefined\n", entry.first.c_str());
	} else {
	    printf("%s panic %d\n", entry.first.c_str(), (int) entry.second.index());
	}
    }
}

void Model::set(const std::string& key, const Value& value) {
    Model* container = this;
    const char* begin = key.data();
    const char* cursor = begin;
    while (*cursor) {
        if (*cursor == '.') {
            std::string chunk{begin, cursor};
            auto it = container->values.find(chunk);
            std::shared_ptr<Model> child;

            {
                std::shared_lock lock{container->mutex};
                if (it == container->values.end()) {
                } else if (auto ptr = std::get_if<std::shared_ptr<Model>>(&it->second)) {
                    child = *ptr;
                }
            }

            if (!child) {
                child = std::make_shared<Model>();
                std::unique_lock lock{container->mutex};
                container->values[chunk] = child;
            }

            container = child.get();
            begin = cursor + 1;
        }
        cursor++;
    }
    container->values[{begin, cursor}] = value;
}

const Value& Model::get(const std::string& key) {
    static Value empty = Undefined{};
    Model* container = this;
    const char* begin = key.data();
    const char* cursor = begin;
    while (*cursor) {
        if (*cursor == '.') {
            std::string chunk{begin, cursor};
            std::shared_lock lock{container->mutex};
            auto it = container->values.find(chunk);
            if (it == container->values.end())
                return empty;
            if (auto ptr = std::get_if<std::shared_ptr<Model>>(&it->second))
                container = ptr->get();
            else
                return empty;
            begin = cursor + 1;
        }
        cursor++;
    }

    {
        std::string chunk{begin, cursor};
        std::shared_lock lock{container->mutex};
        auto it = container->values.find(chunk);
        return it == container->values.end() ? empty : it->second;
    }
}

void Model::parse(const std::string& iniFile) {
    Model* container = this;

    bool multiline = false;
    std::string acc, key;

    enum class Mode {
        Start,
        Section,
        Key,
        Value,
        Comment,
    } mode = Mode::Start;

    uint32_t c;
    for (std::size_t i = 0; (c = popUTF8(iniFile, i));) {
        switch (mode) {
        case Mode::Start:
            if (c <= ' ')
                break;

            acc.clear();
            if (c == '#') {
                mode = Mode::Comment;
                break;
            }

            if (c == '[') {
                container = this;
                mode = Mode::Section;
                break;
            }

            acc += c;
            mode = Mode::Key;
            break;

        case Mode::Section:
            if (c == ']' || c == '.') {
                acc = trim(acc);
                if (c == ']')
                    mode = Mode::Start;
                if (!acc.empty()) {
                    auto v = container->get(acc);
                    if (auto ptr = std::get_if<std::shared_ptr<Model>>(&v)) {
                        container = ptr->get();
                    } else {
                        auto child = std::make_shared<Model>();
                        container->set(acc, child);
                        container = child.get();
                    }
                    acc.clear();
                }
            } else {
                acc += c;
            }
            break;

        case Mode::Key:
            if (c == '=') {
                multiline = false;
                key = trim(acc);
                mode = Mode::Value;
                acc.clear();
                break;
            }
            acc += c;
            break;

        case Mode::Value:
            if (c == '`') {
                if (multiline) {
                    c = 10;
                    multiline = false;
                } else if (acc.empty()) {
                    multiline = true;
                    break;
                }
            }
            if (!multiline && (c == 10 || c == 13 || c == '#')) {
                acc = trim(acc);
                char* end = nullptr;
                float fval = strtof(acc.data(), &end);
                if (end == acc.data() + acc.size()) {
                    container->set(key, fval);
                } else {
                    container->set(key, acc);
                }
                if (c == '#') {
                    mode = Mode::Comment;
                } else {
                    mode = Mode::Start;
                }
                break;
            }
            acc += c;
            break;

        case Mode::Comment:
            if (c == 10 || c == 13)
                mode = Mode::Start;
            break;
        }
    }
    if (mode == Mode::Value) {
	acc = trim(acc);
	char* end = nullptr;
	float fval = strtof(acc.data(), &end);
	if (end == acc.data() + acc.size()) {
	    container->set(key, fval);
	} else {
	    container->set(key, acc);
	}
    }
}

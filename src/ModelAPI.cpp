#include "VM.hpp"
#include "Model.hpp"

inline VM::API modelapi {{
                {"getString", [](const VM::Args& arg) {
                    auto defVal = arg.get(1);
                    auto value = Model::main->get(arg.get<std::string>(0));
                    if (auto str = std::get_if<std::string>(&value)) {
                        arg.result = *str;
                    } else {
                        arg.result = defVal;
                    }
                }},

                {"getFloat", [](const VM::Args& arg) {
                    auto defVal = arg.get(1);
                    auto value = Model::main->get(arg.get<std::string>(0));
                    if (auto val = std::get_if<float>(&value)) {
                        arg.result = *val;
                    } else {
                        arg.result = defVal;
                    }
                }}
    }};

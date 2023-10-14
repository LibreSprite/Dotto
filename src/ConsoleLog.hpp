#pragma once

#include <iostream>

class ConsoleLog {
public:
    template <typename ... Arg>
    void operator () (Arg&& ... arg) {
        ((std::cout << std::forward<Arg>(arg)), ...);
        std::cout << std::endl;
    }
};

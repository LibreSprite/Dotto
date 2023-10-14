#pragma once

class Boot {
public:
    template<typename Function>
    Boot(Function&& func) {func();}
};

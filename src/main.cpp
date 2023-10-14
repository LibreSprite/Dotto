#include "Log.hpp"
#include "Graphics.hpp"
#include "App.hpp"
#include "Matrix.hpp"

int main(int argc, const char* argv[]) {
    App<ConsoleLog, Graphics<ConsoleLog>> app;
    Model::main->set("main.settings", "settings.ini");
    Model::main->set("main.argc", static_cast<float>(argc));
    for (int i = 0; i < argc; ++i)
	Model::main->set("main.args." + std::to_string(i), argv[i]);
    app.boot();
    while (app.running())
        app.update();
    // Matrix m;
    // m *= Matrix::position(1, 2, 3);
    // m *= Matrix::position(1, 2, 3);
    // ConsoleLog log;
    // log(m.v[3], m.v[7], m.v[11], m.v[15]);
    return 0;
}

#include "Log.hpp"
#include "Graphics.hpp"
#include "App.hpp"
#include "Matrix.hpp"

#if defined(main)
extern "C" int main(int argc, char* argv[])
#else
int main(int argc, const char* argv[])
#endif
{
    LOG("Creating app");
    App<Graphics> app;
    LOG("Initializing model");
    Model::root.set("main.settings", "./data/settings.ini");
    Model::root.set("main.argc", static_cast<float>(argc));
    for (int i = 0; i < argc; ++i)
	Model::root.set("main.args." + std::to_string(i), argv[i]);
    LOG("Booting");
    app.boot();
    LOG("Running");
    while (app.running())
        app.update();
    // Matrix m;
    // m *= Matrix::position(1, 2, 3);
    // m *= Matrix::position(1, 2, 3);
    // LOGGER log;
    // log(m.v[3], m.v[7], m.v[11], m.v[15]);
    return 0;
}

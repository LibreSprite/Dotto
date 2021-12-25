const surface = app.target.findChildById("draw").surface;
const width = surface.width;
const height = surface.height;

app.target.addEventListener("mousemove");

function onEvent(event) {
    if (event == "mousemove")
        surface.setPixel(Math.random() * width, Math.random() * height, ((Math.random() * 0xFFFFFF) << 8) | 0xFF);;
}

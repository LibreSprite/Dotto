const draw = app.target.findChildById("draw");
const surface = draw.surface;

app.target.addEventListener("mousemove");

function onEvent(event) {
    if (event == "mousemove" && arguments[4] == 4) {
        const x = arguments[2] / draw.globalWidth * surface.width | 0;
        const y = arguments[3] / draw.globalHeight * surface.height | 0;
        surface.setPixel(x + Math.random() * 10 - 5,
                         y + Math.random() * 10 - 5,
                         Math.random() * 0xFFFFFF | 0xFF000000);
    }
}

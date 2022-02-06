const target = app.target;
const sample = app.target.findChildById("sample");
const surface = sample.surface;

sample.addEventListener("mousedown");

function onEvent(event) {
    if (event == "mousedown") {
        const x = arguments[2] / sample.globalWidth * surface.width | 0;
        const y = arguments[3] / sample.globalHeight * surface.height | 0;
        const color = surface.getPixel(x, y);
        target.set("value", color);
        app.command("activatecolor", "color", color);
    }
}

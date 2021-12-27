const draw = app.target.findChildById("draw");
const surface = draw.surface;

app.addEventListener("tick");

function onEvent(event) {
    surface.setPixel(Math.random() * surface.width,
                     Math.random() * surface.height,
                     Math.random() * 0xFFFFFF | 0xFF000000);
}

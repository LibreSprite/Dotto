const title = app.target.findChildById("title");
const surface = app.target.findChildById("draw").surface;
const width = surface.width;
const height = surface.height;

app.target.removeChild(app.target.findChildById("close"));
app.target.addEventListener("mousemove");
app.target.addEventListener("mousedown");
app.target.addEventListener("mouseup");

function onEvent(event) {
    title.text = Array.prototype.join.call(arguments, ", ");
    surface.setPixel(Math.random() * width, Math.random() * height, ((Math.random() * 0xFFFFFF) << 8) | 0xFF);
}

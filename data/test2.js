const title = app.target.findChildById("title");

app.target.addEventListener("mousedown");
app.target.addEventListener("mouseup");
app.target.addEventListener("mousemove");

function onEvent(event) {
    title.text = Array.prototype.join.call(arguments, ", ");
}

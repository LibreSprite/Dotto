app.addTool("spray");

function onEvent(name) {
    if (name == "toolupdate")
        app.target.surface.setPixel(app.target.lastX + Math.random() * 10 - 5,
                                    app.target.lastY + Math.random() * 10 - 5,
                                    app.target.color);
}

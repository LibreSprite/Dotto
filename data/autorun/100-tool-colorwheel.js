app.addTool("wheel");
var window;

function onEvent(name) {
    if (!window) {
        if (name == "toolactivate") {
            window = app.window.createChild("colorpicker");
            window.findChildById("close").addEventListener("click");
        }
    } else {
        if (name == "toolstart" || name == "tooldeactivate" || name == "click") {
            window.remove();
            window = null;
            if (name != "tooldeactivate")
                app.command("toggletool");
        }
    }
}

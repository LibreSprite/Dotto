app.addTool("wheel");
var window;

function onEvent(name) {
    if (!window) {
        if (name == "toolactivate") {
            window = app.window.createChild("colorpicker");
            window.addEventListener("remove");
        }
    } else {
        if (app.target.which == 0 && (name == "toolstart" || name == "toolupdate" || name == "toolend"))
            return;
        if (name == "toolstart" || name == "tooldeactivate" || name == "remove") {
            var old = window;
            window = null;
            old.remove();
            if (name != "tooldeactivate")
                app.command("toggletool");
        }
    }
}

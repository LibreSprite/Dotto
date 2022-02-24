app.addTool("hand")

function onEvent(name) {
    var target = app.target;
    switch (name) {
    case "toolstart":
        app.activeEditor.set("draggable", true);
        break;
    case "toolend":
    case "tooldeactivate":
        app.activeEditor.set("draggable", false);
        break;
    }
}

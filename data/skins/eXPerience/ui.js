var views = {};
var controllers = {
    filemenu : {
        click:closeStartMenu
    },

    editor : {},

    script : {
        init : function() {
            console.log("Booting Default UI");

            for (var name in controllers) {
                var node = app.target.findChildById(name);
                if (node) {
                    for (var event in controllers[name]) {
                        node.addEventListener(event.trim());
                    }
                    views[name] = node;
                } else {
                    console.log("Could not find " + name);
                }
            }

            for (var name in controllers.script) {
                app.addEventListener(name);
            }
        },

        activatetool : function() {
            var activeTool = app.activeTool;
            if (activeTool) {
                views.toolconfigbutton.src = activeTool.get("icon");
                views.toolconfigbutton.visible = true;
                views.toolconfigmenu.set("meta", activeTool.get("meta"));
            } else {
                views.toolconfigbutton.visible = false;
            }
        }
    },

    toolconfigmenu : {
        change : function() {
            console.log("toolconfigmenu change");
            views.toolconfigmenu.set("result", null); // clean previous result
            app.activeTool.apply(views.toolconfigmenu.get("result"));
        }
    },

    toolconfigbutton : {},

    startbutton : {
        mouseup : function() {
            var pressed = views.startbutton.get("state") != "active";
            views.filemenu.visible = pressed;
            views.startbutton.set("state", pressed ? "active" : "enabled");
            if (pressed)
                views.filemenu.bringToFront();
        }
    },

    quitbutton : {
        click : function() {
            app.quit();
        }
    }
};

function closeStartMenu() {
    views.filemenu.visible = false;
    views.startbutton.set("state", "enabled");
}

function onEvent(name) {
    var controller, target;
    for (var k in views) {
        if (app.target == views[k]) {
            target = k;
            controller = controllers[target];
            break;
        }
    }
    if (!controller || !controller[name]) {
        target = "script";
        controller = controllers[target];
    }
    var func = controller[name];
    if (func) {
        var view = views[target];
        var args = [];
        for (var i = 2; i < arguments.length; ++i)
            args.push(arguments[i]);
        func.apply(view, args);
    } else {
        console.log("No listener \"" + name + "\" in " + target);
    }
}

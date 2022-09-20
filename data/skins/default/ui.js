var views = {};
var controllers = {
    editor : {},

    script : {
        init : function() {
            console.log("Booting Default UI");

            for (var name in controllers) {
                var node = app.target.findChildById(name) || app.target.findChild(name);
                if (node) {
                    for (var event in controllers[name]) {
                        console.log("Listening to " + name + "." + event);
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
                views.toolconfigbutton.apply(
                    "src", activeTool.get("icon"),
                    "visible", true
                );
                var meta = activeTool.get("meta");
                views.toolconfigquick.set("meta", meta);
                views.toolconfigpanel.set("meta", meta);
            } else {
                views.toolconfigbutton.visible = false;
            }
        }
    },

    toolconfigpanel : {
        change : function() {
            views.toolconfigpanel.set("result", null); // clean previous result
            app.activeTool.apply(views.toolconfigpanel.get("result"));
        }
    },

    toolconfigquick : {
        change : function() {
            views.toolconfigquick.set("result", null); // clean previous result
            app.activeTool.apply(views.toolconfigquick.get("result"));
        }
    },

    toolconfigbutton : {},

    quitbutton : {
        click : function() {
            app.quit();
        }
    }
};

function closeStartMenu() {
    var target = app.eventTarget;
    if (!target) {
        console.log("No event target");
        return;
    }
    var isClickout = target.hasTag("*clickout");
    console.log("clazz " + isClickout);
    if (!isClickout)
        return;
    target.visible = false;
    // views.startbutton.set("state", "enabled");
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

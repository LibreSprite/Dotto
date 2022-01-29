var views = {};
var controllers = {
    filemenu : {
        click:closeStartMenu
    },

    editor : {},

    script : {
        init : function() {
            console.log("Booting eXPerience UI");

            for (var name in controllers) {
                var node = app.target.findChildById(name);
                if (node) {
                    for (var event in controllers[name]) {
                        node.addEventListener(event.trim());
                    }
                    views[name] = node;
                }
            }

            for (var name in controllers.script) {
                app.addEventListener(name);
            }
        },

        activatetool : function() {
            views.toolconfigbutton.src = app.activeTool.get("icon");
            views.toolconfigbutton.visible = true;
            views.toolconfigmenu.set("meta", app.activeTool.get("meta"));
        }
    },

    toolconfigmenu : {
        change : function() {
            views.toolconfigmenu.set("result", null); // clean previous result
            app.activeTool.apply(views.toolconfigmenu.get("result"));
        }
    },

    toolconfigbutton : {
        click : function() {
            views.toolconfigmenu.visible = !views.toolconfigmenu.visible;
        }
    },

    startbutton : {
        mouseup : function() {
            var pressed = views.startbutton.get("state") != "active";
            views.filemenu.visible = pressed;
            views.startbutton.set("state", pressed ? "active" : "enabled");
            if (pressed)
                views.filemenu.bringToFront();
        }
    },

    newbutton : {
        click : function() {
            app.command("newfile", "interactive", true);
        }
    },

    openbutton : {
        click : function() {
            var pick = app.open('*.png', 'Open image'); //
            if (pick) {                                 //
                views.editor.set("file", pick);         // TODO: use a command instead
                views.editor.visible = true;            //
            }                                           //
        }
    },

    savebutton : {
        click : function() {
            var pick = app.save('*.png', 'Save image', "Image format"); //
            if (pick) {                                                 // TODO: use a command instead
                app.write(pick, app.activeCell.composite);              //
            }                                                           //
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

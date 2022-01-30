var views = {};
var sidebars = [];

var controllers = {
    filemenu : {click:clickOutsideMenu},
    filtermenu : {click:closeMenu},

    editor : {},

    script : {
        init : function() {
            console.log("Booting Futuretro UI");

            for (var name in controllers) {
                var node = app.target.findChildById(name);
                if (node) {
                    for (var event in controllers[name]) {
                        node.addEventListener(event.trim());
                    }
                    if (node.get("class") == "sidebar")
                        sidebars.push(node);
                    views[name] = node;
                }
            }

            for (var name in controllers.script) {
                app.addEventListener(name);
            }
        },

        activatetool : function() {
            views.toolconfigmenu.set("meta", app.activeTool.get("meta"));
        }
    },

    toolconfigmenu : {
        change : function() {
            views.toolconfigmenu.set("result", null); // clean previous result
            app.activeTool.apply(views.toolconfigmenu.get("result"));
        }
    },

    newbutton : {
        click : function() {
            closeMenu();
            app.command("newfile", "interactive", true);
        }
    },

    openbutton : {
        click : app.command.bind(null, "openfile")
    },

    savebutton : {
        click : app.command.bind(null, "savefile")
    },

    quitbutton : {
        click : function() {
            app.quit();
        }
    }
};

var openMenuButton, openMenuMenu;

function clickOutsideMenu() {
    if (app.eventTarget == app.target)
        closeMenu();
}

function mouseUpMenuButton(button, menu) {
    closeMenu();
    openMenuButton = button;
    openMenuMenu = menu;
    var pressed = views[button].get("state") != "active";
    views[menu].visible = pressed;
    views[button].set("state", pressed ? "active" : "enabled");
    if (pressed)
        views[menu].bringToFront();
}

function closeMenu() {
    for (var i = 0; i < sidebars.length; ++i)
        sidebars[i].visible = false;
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

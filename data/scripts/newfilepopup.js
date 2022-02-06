var views = {script:app.target};

var controllers = {
    widthtextbox :{},
    heighttextbox :{},

    script : {
        init : function() {
            for (var name in controllers) {
                var node = app.target.findChildById(name);
                if (node) {
                    for (var event in controllers[name]) {
                        node.addEventListener(event.trim());
                    }
                    views[name] = node;
                }
            }
        }
    },

    okbutton : {
        click : function() {
            var width = views.widthtextbox.findChildById("value").get("text");
            var height = views.heighttextbox.findChildById("value").get("text");
            app.command("newfile", "width", width, "height", height);
            views.script.remove();
        }
    },

    cancelbutton : {
        click : function() {
            views.script.remove();
        }
    }
};

function onEvent(name) {
    var controller, target;
    for (var k in views) {
        if (app.target == views[k]) {
            target = k;
            controller = controllers[target];
            break;
        }
    }
    if (!controller) {
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

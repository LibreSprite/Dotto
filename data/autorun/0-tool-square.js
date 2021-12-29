app.addTool("square");
var selection;

function onEvent(name) {
    (({
        toolstart:function(){
            app.release(selection);
            selection = app.newSelection();
            app.hold(selection);
        },

        toolupdate:function(){
            selection.add(app.target.lastX, app.target.lastY, 255);
            app.command("paint", "selection", selection, "preview", true)
        },

        toolend:function(){
            app.command("paint", "selection", selection)
            selection = app.release(selection);
        }

    })[name] || (function(){}))();
}

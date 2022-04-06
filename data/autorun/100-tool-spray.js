app.addTool("spray");
var scatter = 50;
var selection, which, cx, cy;

function len(x, y) {
    return x*x + y*y;
}

function random() {
    cx = app.target.lastX + Math.random() * (scatter*2) - scatter;
    cy = app.target.lastY + Math.random() * (scatter*2) - scatter;
}

function onEvent(name) {
    (({
        init:function(){},

        toolstart:function(){
            app.target.hideCursor = true;
            selection = app.newSelection();
            app.target.previewArea = selection;
            which = app.target.which;
            onEvent("toolupdate");
        },

        toolupdate:function(){
            if (!which)
                selection.clear();
            var size = scatter - (len(cx - app.target.lastX, cy - app.target.lastY) / (scatter));
            var normalize = 255 / (size*size);
            for (var y = cy - size; y < cy + size; ++y) {
                for (var x = cx - size; x < cx + size; ++x) {
                    var v = 255 - ((x-cx)*(x-cx)+(y-cy)*(y-cy)) * normalize;
                    if (v > 0) selection.add(x, y, v);
                }
            }
            if (which) {
                app.command("paint",
                            "selection", selection,
                            "preview", true,
                            "surface", app.target.surface);
            }
            random();
        },

        toolend:function(){
            if (which) {
                app.command("paint",
                            "selection", selection,
                            "surface", app.target.surface);
            }
        }

    })[name] || (function(){
        console.log("Unknown event: " + name);
    }))();
}

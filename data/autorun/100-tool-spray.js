app.addTool("spray");
var selection, which;

function len(x, y) {
    return x*x + y*y;
}

function onEvent(name) {
    (({
        init:function(){},

        toolstart:function(){
            app.release(selection);
            selection = app.newSelection();
            app.hold(selection);
            which = app.target.which;
            onEvent("toolupdate");
        },

        toolupdate:function(){
            var scatter = 50;
            var cx = app.target.lastX + Math.random() * (scatter*2) - scatter;
            var cy = app.target.lastY + Math.random() * (scatter*2) - scatter;
            var size = scatter - (len(cx - app.target.lastX, cy - app.target.lastY) / (scatter));
            var normalize = 255 / (size*size);
            for (var y = cy - size; y < cy + size; ++y) {
                for (var x = cx - size; x < cx + size; ++x) {
                    var v = 255 - ((x-cx)*(x-cx)+(y-cy)*(y-cy)) * normalize;
                    if (v > 0) selection.add(x, y, v);
                }
            }
            app.command("paint",
                        "selection", selection,
                        "preview", true,
                        "cursor", which == 0);
        },

        toolend:function(){
            app.command("paint",
                        "selection", selection,
                        "cursor", which == 0);
            selection = app.release(selection);
        }

    })[name] || (function(){
        console.log("Unknown event: " + name);
    }))();
}

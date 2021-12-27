const jumpers = [];

function Jumper() {
    var x = 0, y = 0, vx = Math.random() * 10 - 5, vy = 0;
    const node = app.target.parent.createChild("image");
    node.absolute = true;
    node.width = 128;
    node.height = 128;
    node.src = "%appdata/" + (Math.random()*5+1|0) + ".png";

    this.update = function() {
        vy += 1.0;
        if ((vx < 0 && x < 0) || (vx > 0 && x > node.parent.globalWidth - 128))
            vx = Math.random() * 20 * (vx < 0 ? 1 : -1);
        if (vy > 0 && y > node.parent.globalHeight - 128) {
            vy = -vy * 0.5;
            if (vy < 1) {
                vy = -Math.random() * 100;
                vx = Math.random() * 20 - 10;
            }
            node.src = "%appdata/" + (Math.random()*5+1|0) + ".png";
        }
        x += vx * 0.1;
        y += vy * 0.1;
        node.x = x;
        node.y = y;
    };
}

for (var i = 0; i < 10; ++i)
    jumpers.push(new Jumper());

app.addEventListener("tick");

function onEvent(event) {
    for (var i = 0; i < jumpers.length; ++i)
        jumpers[i].update();
}

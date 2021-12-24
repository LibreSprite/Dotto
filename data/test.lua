local target = app.target()
target.findChildById("title").text("Hello world from Lua!")
target.removeChild(target.findChildById("close"))

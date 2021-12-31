app.addTool("dropper")

function onEvent(name)
   local target = app.target();
   if name == "toolstart" or name == "toolupdate" then
      app.command("activatecolor", "color", target.surface().getPixel(target.lastX(), target.lastY()))
   end
end

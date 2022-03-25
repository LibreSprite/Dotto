app.addTool("dropper")

function onEvent(name)
   local target = app.target();

   if not target then return end

   local x = target.lastX();
   local y = target.lastY();
   local color = target.surface().getPixel(x, y);

   if target.which() == 0 then
      color = color | 0xFF000000
      if name == "toolend" then
         app.command("paint", "cursor", true)
         return
      end

      local selection = app.newSelection();
      for i = -5, 5 do
         local absi = math.abs(i)
         if absi > 1 then
            selection.add(x + i, y, 300 - absi * 30)
            selection.add(x, y + i, 300 - absi * 30)
         end
      end
      app.command("paint",
                  "preview", true,
                  "cursor", true,
                  "color", color,
                  "selection", selection)

   elseif name == "toolstart" or name == "toolupdate" then
      app.command("activatecolor", "color", color)
      app.command("toggletool")
   end
end

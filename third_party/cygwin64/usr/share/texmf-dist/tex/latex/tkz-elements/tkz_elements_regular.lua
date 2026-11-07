-- tkz_elements_regular.lua
-- date 2024/02/04
-- version 2.00c
-- Copyright 2024  Alain Matthes
-- This work may be distributed and/or modified under the
-- conditions of the LaTeX Project Public License, either version 1.3
-- of this license or (at your option) any later version.
-- The latest version of this license is in
--   http://www.latex-project.org/lppl.txt
-- and version 1.3 or later is part of all distributions of LaTeX
-- version 2005/12/01 or later.
-- This work has the LPPL maintenance status “maintained”.
-- The Current Maintainer of this work is Alain Matthes.

regular_polygon = {}
function regular_polygon: new (za, zb ,nb)
   local type              = 'regular_polygon'
   local table              = regular_ (za , zb , nb)
   local center            = za
   local through           = zb
   local angle             = 2 * math.pi/nb
   local exradius          = point.abs (zb-za)
   local circle            = circle : new (za,zb)
   local inradius          = exradius * math.cos(math.pi/nb)
   local side              = exradius * math.sin(math.pi/nb)
   local next              = table[2]
   local first             = table[1]
   local proj              = projection_ (first,next,za)
   local o = { type        = type, 
               center      = center, 
               through     = through,
               exradius    = exradius, 
               inradius    = inradius,
               table       = table, 
               circle      = circle, 
               nb          = nb,
               angle       = angle, 
               side        = side, 
               proj        = proj }
    setmetatable(o, self)
    self.__index = self
    return o
end
-----------------------
-- points --
-----------------------
-------------------
-- Result -> line
-------------------
-----------------------
-- circles --
-----------------------
function regular_polygon : incircle ()
   local next,first
    next = self.table[2]
    first = self.table[1]
   return circle : new ( self.center , projection_ (first,next,self.center) )  
end
-------------------
-- Result -> triangle
-------------------

-------------------
-- Result -> miscellaneous
-------------------
function regular_polygon : name (nm)
     for K,V in ipairs(self.table) do
         z[nm..K] = V
      end
end

return regular_polygon
-- tkz_elements-square.lua
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
---------------------------------------------------------------------------
--                           Squares
---------------------------------------------------------------------------

square = {}
function square: new (za, zb,zc,zd)
   local d
   local zi = midpoint_ (za,zc)
   local zj = midpoint_ (zb,zd)
   if point.abs (zj-zi) < tkz_epsilon  then  else error ("it's not a square (center)")
   end
   if math.abs(point.abs (zc-za) - point.abs (zd-zb)) < tkz_epsilon  then  else error ("it's not a square (diagonal)")
   end
   if math.abs(point.abs (zb-za) - point.abs (zd-za)) < tkz_epsilon  then  else error ("it's not a square (side)")
   end

   local type              = 'square'
   local side              = point.abs ( zb - za )
   local pc                = rotation_ (zb,-math.pi/2,za)
   local pd                = rotation_ (za,math.pi/2,zb)
   local center            = midpoint_ (za,zc)
   local exradius          = point.abs (center-za)
   local inradius          = exradius * math.cos(math.pi/4)
   local diagonal          = math.sqrt(2) * side
   local proj              = projection_ (za,zb,center)
   local ab                = line : new (za,zb)
   local bc                = line : new (zb,zc)
   local cd                = line : new (zc,zd)
   local da                = line : new (zd,za)
   local bd                = line : new (zb,zd)
   local ac                = line : new (za,zc)
   local o  = {   pa       = za, 
                  pb       = zb, 
                  pc       = zc,
                  pd       = zd,
                  side     = side,
                  center   = center,
                  exradius = exradius,
                  inradius = inradius,
                  diagonal = diagonal,
                  proj     = proj,
                  ab       = ab,
                  ac       = ac,
                  bc       = bc,
                  da       = da,
                  cd       = cd,
                  bd       = bd,
                  type     = type  }
    setmetatable(o, self)
    self.__index = self
    return o
end

function square : rotation (zi,za)
   local zb = rotation_ (zi,math.pi/2,za)
   local zc = rotation_ (zi,math.pi/2,zb)
   local zd = rotation_ (zi,math.pi/2,zc)
   return square : new (za,zb,zc,zd)
end

function square : side (za,zb,swap)
   if swap == nil 
    then
      local zc = rotation_ (zb,-math.pi/2,za)
      local zd = rotation_ (za,math.pi/2,zb)
       return square : new (za,zb,zc,zd)
    else
       local zc = rotation_ (zb,math.pi/2,za)
       local zd = rotation_ (za,-math.pi/2,zb)
       return square : new (za,zb,zc,zd) 
   end
end

return square
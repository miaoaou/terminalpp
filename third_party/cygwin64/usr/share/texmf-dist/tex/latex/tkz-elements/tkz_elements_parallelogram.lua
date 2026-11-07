-- tkz_elements_parallelogram.lua
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
--                           Parallelogram
---------------------------------------------------------------------------

parallelogram = {}
function parallelogram: new (za, zb, zc, zd)
   local d
   local zi = midpoint_ (za,zc)
   local zj = midpoint_ (zb,zd)
   if point.abs (zj-zi) < tkz_epsilon  then  else error ("it's not a parallelogram")    
   end
   local type              = 'parallelogram'
   local center                 = midpoint_ (za,zc)
   local ab                 = line : new (za,zb)
   local bc                 = line : new (zb,zc)
   local cd                 = line : new (zc,zd)
   local da                 = line : new (zd,za)
   local ac                 = line : new (za,zc)
   local bd                 = line : new (zb,zd)
   local o  = {   pa       = za, 
                  pb       = zb, 
                  pc       = zc,
                  pd       = zd,
                  ab       = ab,
                  ac       = ac,
                  bc       = bc,
                  da       = da,
                  cd       = cd,
                  bd       = bd,
                  center   = center,
                  type     = type  }
    setmetatable(o, self)
    self.__index = self
    return o
end

function parallelogram : fourth (za,zb,zc)
   local zd = zc + (za-zb)
   return parallelogram : new (za, zb, zc,zd)
end 

return parallelogram
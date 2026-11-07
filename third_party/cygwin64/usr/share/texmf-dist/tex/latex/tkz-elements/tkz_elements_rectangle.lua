-- tkz_elements-rectangle.lua
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
--                           rectangle
---------------------------------------------------------------------------

rectangle = {}
function rectangle: new (za, zb,zc,zd)
   local d
   local zi = midpoint_ (za,zc)
   local zj = midpoint_ (zb,zd)
   if point.abs (zj-zi) < tkz_epsilon  then  else error ("it's not a rectangle")    
   end
   if math.abs(point.abs (zc-za)-point.abs (zd-zb)) < tkz_epsilon  then  else error ("it's not a rectangle")    
   end
   local type               = 'rectangle'
   local center             = midpoint_ (za,zc)
   local exradius           = point.abs (center-za)
   local ab                 = line : new (za,zb)
   local bc                 = line : new (zb,zc)
   local cd                 = line : new (zc,zd)
   local da                 = line : new (zd,za)
   local ac                 = line : new (za,zc)
   local bd                 = line : new (zb,zd)
   local length             = point.mod (zb-za)
   local width              = point.mod (zd-za)
   local diagonal           = point.abs (zc-za)
   local o  = {   pa        = za, 
                  pb        = zb, 
                  pc        = zc,
                  pd        = zd,
                  ab        = ab,
                  ac        = ac,
                  bc        = bc,
                  da        = da,
                  cd        = cd,
                  bd        = bd,
                  diagonal  = diagonal,
                  exradius  = exradius,
                  center    = center,
                  length    = length,
                  width     = width,
                  type      = type  }
    setmetatable(o, self)
    self.__index = self
    return o
end

function rectangle : angle (zi,za,an)
   local zb,zc,zd
   local zc = symmetry_ (zi,za)
   local zb = rotation_(zi,-an,zc)
   local zd = symmetry_ (zi,zb)
   return rectangle : new (za,zb,zc,zd)
end

function rectangle : gold  (za,zb,swap)
 local zc,zd
 local a     = point.arg (zb-za)
 local d     = point.abs(zb-za)/tkzphi
 if swap == nil then swap = false end
 if swap  then 
     zc    = zb : south (d/scale)
     zc    = rotation_ (zb,a,zc)
     zd    = za : south (d/scale)
     zd    = rotation_ (za,a,zd)
    return rectangle : new (za,zb,zc,zd)   
 else
    zc    = zb : north (d/scale)
    zc    = rotation_ (zb,a,zc)
    zd    = za : north (d/scale)
    zd    = rotation_ (za,a,zd)
 return rectangle : new (za,zb,zc,zd)
 end
end

function rectangle : diagonal  (za,zc,swap)
   local zb,zd
   if swap == nil then swap = false end
   if swap  then 
      zd   = point (zc.re,za.im)
      zb   = point (za.re,zc.im)
     return rectangle : new (za,zb,zc,zd) 
   else
    zb   = point (zc.re,za.im)
    zd   = point (za.re,zc.im)
   return rectangle : new (za,zb,zc,zd)
end
end

function rectangle : side  (za,zb,d,swap)
   local a,zc,zd
    a  = point.arg (zb-za)
    if swap == nil then swap = false end
    if swap  then 
       zc = zb : south (d/scale)
       zc = rotation_ (zb,a,zc)
       zd = za : south (d/scale)
       zd = rotation_ (za,a,zd)
      return rectangle : new (za,zb,zc,zd)
     else
    zc = zb : north (d/scale)
    zc = rotation_ (zb,a,zc)
    zd = za : north (d/scale)
   zd = rotation_ (za,a,zd)
   return rectangle : new (za,zb,zc,zd)
end
end

-------------------
function rectangle : get_lengths ()
   local a = self.pa
   local b = self.pb
   local c = self.pc
   local d = self.pd
  return point.mod(b-a),point.mod(c-b),point.mod(d-c),point.mod(a-d)
end

return rectangle

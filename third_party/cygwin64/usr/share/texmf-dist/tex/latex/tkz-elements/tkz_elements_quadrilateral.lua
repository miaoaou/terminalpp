-- tkz_elements_quadrilateral.lua
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

quadrilateral = {}
function quadrilateral: new (za, zb, zc, zd)
   local type        = 'quadrilateral'
   local a           = point.abs ( zb - za )
   local b           = point.abs ( zc - zb )
   local c           = point.abs ( zd - zc )
   local d           = point.abs ( za - zd ) 
   local ab          = line : new (za,zb)
   local bc          = line : new (zb,zc)
   local cd          = line : new (zc,zd)
   local da          = line : new (zd,za)
   local ac          = line : new (za,zc)
   local bd          = line : new (zb,zd)
   local i           = intersection_ll_ (za,zc,zb,zd)
   local g           = barycenter_ ( {za,1} , {zb,1} , {zc,1} ,{ zd ,1 } )
   local o = { pa    = za, 
               pb    = zb, 
               pc    = zc,
               pd    = zd,
               a     = a,
               b     = b,
               c     = c,
               d     = d,
               ab    = ab,
               bc    = bc,
               cd    = cd,
               da    = da,
               ac    = ac,
               bd    = bd,
               i     = i,
               g     = g,
               type  = type }
    setmetatable(o, self)
    self.__index = self
    return o
end
-----------------------
function quadrilateral : iscyclic ()
   local d
   local alpha = point.arg ((self.pd-self.pa) / (self.pb-self.pa))
   local beta  = point.arg ((self.pb-self.pc) / (self.pd-self.pc))
   if math.abs (alpha+beta-math.pi) < tkz_epsilon then return true
   else return false
   end
end 

return quadrilateral
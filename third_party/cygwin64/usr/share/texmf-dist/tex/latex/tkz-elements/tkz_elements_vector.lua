-- tkz_elements_vectors.lua
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

-- ----------------------------------------------------------------------------
vector = {}
function vector: new(za, zb)
    local type          = 'vector'
    local slope         = angle_normalize_(point.arg(zb-za))
    local norm        = point.mod(zb-za)
    local o =  {t      = za, 
                h      = zb,
                norm  = norm,
                slope   = slope,
                type    = type }
    setmetatable(o, self)
    self.__index = self
    return o
end

function vector.__add(v1,v2)
  return v1 : add (v2)
end

function vector.__sub(v1,v2)
   local v = v2 : scale(-1)
  return v1 : add (v)
end

function vector.__unm(v)
 return v : scale(-1)
end

function vector.__mul(r,v)
  return v : scale(r)
end

function vector: normalize  ()
   local z  = self.h-self.t
   local d  = point.abs(z)
   local nz = point(z.re/d,z.im/d)
   return vector : new (self.t,nz + self.t)  
end

  function vector: add (ve)
    return vector :new (self.t,self.h+ve.h-ve.t)
  end
 
function vector: orthogonal (d)
local z
if d == nil then
   return vector : new (self.t, rotation_(self.t,math.pi/2,self.h))
else
   z = self.t+ point (d*math.cos(self.slope),d*math.sin(self.slope))
 return vector : new (self.t, rotation_(self.t,math.pi/2,z))
end
end

function vector: scale (d)
   local l,z
   l = self.norm
   z = self.t+ point (d*l*math.cos(self.slope),d*l*math.sin(self.slope))
   return vector : new (self.t,z )
end
 
function vector: at (zc)
  return vector :new (zc,zc+self.h-self.t)
end

return vector

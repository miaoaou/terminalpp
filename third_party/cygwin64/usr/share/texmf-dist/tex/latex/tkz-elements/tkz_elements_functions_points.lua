-- tkz_elements_functions_points.lua
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

function id ()
for i,k in pairs(z) do
 if _G[i]==k then  else _G[i]=k end
 end
end

function polar_ (radius, phi)
	return point(radius * math.cos(phi), radius * math.sin(phi))
end

function barycenter_ (...)
local cp = table.pack(...)
local i
local sum = 0
local weight=0
for i=1,cp.n do
   sum = sum + cp[i][1]*cp[i][2]
   weight = weight + cp[i][2]
end
return sum/weight
end

function rotation_ (c,a,pt)
  local z = point( math.cos(a) , math.sin(a) )
  return z*(pt-c)+c
end

function set_rotation_ (c,angle,...)
   local tp = table.pack(...)
   local i
   local t = {}
   for i=1,tp.n do
         table.insert( t ,  rotation_(c , angle , tp[i] ))
   end
   return table.unpack ( t )
end

function symmetry_(c,pt)
    return 2 * c - pt
end

function set_symmetry_ (c,...)
   local tp = table.pack(...)
   local i
    local t = {}
    for i=1,tp.n do
        table.insert( t , symmetry_ (c , tp[i])  )
     end
  return table.unpack ( t )
end

function homothety_(c,t,p)
    return c + t * (p - c)
end

function set_homothety_ (c,coeff,...)
   local tp = table.pack(...)
   local i
    local t = {}
    for i=1,tp.n do
        table.insert( t , homothety_ (c, coeff , tp[i]) )
     end
   return table.unpack ( t )
end

function translation_(a , p)
    return a+p
end

function set_translation_ (u,...)
   local tp = table.pack(...)
   local i
   local t = {}
   for i=1,tp.n do
         table.insert( t ,  (u + tp[i])  )
   end
   return table.unpack ( t )
end

function random_point_(lower, upper)
    math.randomseed( tonumber(tostring(os.time()):reverse():sub(1,6)) )
    x = math.random (lower, upper)
    y = math.random (lower, upper)
   return scale * point (x,y)
end 

function midpoints_ (...)
   local arg = table.pack(...)
   local n = arg.n
   local i
   local t = {}
   for i=1, n-1 do
      table.insert(  t , (arg[i]+arg[i+1])/2 )
   end
   table.insert(  t , (arg[n]+arg[1])/2 )
   return table.unpack ( t )
end


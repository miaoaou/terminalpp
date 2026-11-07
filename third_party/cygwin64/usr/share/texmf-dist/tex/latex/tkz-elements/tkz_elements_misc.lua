-- tkz_elements_functions_maths.lua
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
-- ----------------------------------------------------------------
-- 
-- ----------------------------------------------------------------
function get_points (obj)
if obj.type == 'line' then return   obj.pa,obj.pb
 elseif
  obj.type  == 'triangle' then return   obj.pa,obj.pb,obj.pc
  elseif 
  obj.type == 'circle' then   return obj.center,obj.through
  elseif 
  obj.type == 'ellipse' then   return obj.pc,obj.pa,obj.pb
  elseif 
  obj.type == 'square' or obj.type == 'rectangle' or obj.type == 'quadrilateral' or obj.type == 'parallelogram'
  then   return obj.pa,obj.pb,obj.pc,obj.pd
end
end

function set_lua_to_tex (t)
   for k,v in pairs(t) do
    token.set_macro(v,_ENV[v],'global')
   end
end  

function bisector (a,b,c)
   local i
      i = in_center_ (a,b,c)
   return  line : new (a,intersection_ll_ (a,i,b,c))
end

function altitude (a,b,c)
   local o,p 
   o = ortho_center_ (a,b,c)
   p = projection_ (b,c,a)
   return  line : new (a,p) 
end

function  bisector_ext(a,b,c)   -- n =1 swap n=2 2 swap
 local i,p
    i = in_center_ (a,b,c)
    p = rotation_ (a,math.pi/2,i)
   return line : new (a,p)
end

function equilateral (a,b)
   return equilateral_tr_ (a,b)
end

function midpoint (a,b)
    return (a+b)/2
end
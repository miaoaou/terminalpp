-- tkz_elements_functions_regular.lua
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
 --  
--------------------------------------------------------------------------- 
function regular_(c,th,s) -- center through side
    local r,t,dep
    dep = angle_normalize(point.arg (th-c))
     t = {}
    r = point.mod (th-c)
   for i =1,s do
       table.insert(  t , (c + point : polar (r,2*(i-1)*math.pi/s + dep) ))
   end
  return t
end
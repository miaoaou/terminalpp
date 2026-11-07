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

-- constant
tkzphi      = (1+math.sqrt(5))/2
tkzinvphi   = (math.sqrt(5)-1)/2
tkzsqrtphi  =  math.sqrt(tkzphi)
---------------------------------------------------------------------------
function round(num, idp)
  return topoint(string.format("%." .. (idp or 0) .. "f", num))
end

function tkzround( num, idp )
	local mult = 10 ^ ( idp or 0 )
	return math.floor( num * mult + 0.5 ) / mult
end  

function dot_product (a,b,c)
   return (b-a)..(c-a)
end

function Cramer33(a1,a2,a3,b1,b2,b3,c1,c2,c3)
  return a1*b2*c3+a3*b1*c2+a2*b3*c1-a3*b2*c1-a1*b3*c2-a2*b1*c3
end

function Cramer22(a1,a2,b1,b2)
  return a1*b2-a2*b1
end

function aligned ( m,a,b )
    local z
    z = (b-a)/(m-b)
      if math.abs(z.im) < tkz_epsilon
        then
            return true
        else
            return false
        end
end 

function islinear (z1,z2,z3)
     local dp
    dp = (z2-z1) ^ (z3-z1)
    if math.abs(dp) < tkz_epsilon 
    then 
        return true
    else 
        return false
    end
end

function isortho (z1,z2,z3)
   local dp
   dp = (z2-z1) .. (z3-z1)
   if math.abs(dp) < tkz_epsilon 
    then 
        return true
    else 
        return false
    end
end

function parabola (a,b,c)
   local xa,xb,xc,ya,yb,yc
   xa = a.re
   ya = a.im
   xb = b.re
   yb = b.im
   xc = c.re
   yc = c.im
   D = (xa-xb)*(xa-xc)*(xb-xc)
   A = (xc*(yb-ya) + xb*(ya-yc)+xa*(yc-yb))/D
   B = (xc*xc*(ya-yb)+xb*xb*(yc-ya)+xa*xa*(yb-yc))/D
   C = (xb*xc*(xb-xc)*ya + xc*xa*(xc-xa)*yb +xa*xb*(xa-xb)*yc)/D
   return A,B,C
end

function value (v)
   return scale * v
end

function real (v)
   return v/scale
end

function get_angle (a,b,c)
  return angle_normalize_(get_angle_( a,b,c ))
end 
   
function get_angle_( a,b,c )
	 return point.arg ((c-a)/(b-a))
end

function angle_normalize (a)
return angle_normalize_ (a)
end

function angle_normalize_ (a)
    while a < 0 do
        a = a + 2*math.pi
    end
    while a >= 2*math.pi do
        a = a - 2*math.pi
    end
    return a
end

function barycenter (...)
   return barycenter_ (...)
end

function swap(a,b)
   local t=a
   a=b
   b=t
   return a,b
end

function format_number(number, decimal_places)
    local format_string = string.format("%%.%df", decimal_places)
    return string.format(format_string, number)
end

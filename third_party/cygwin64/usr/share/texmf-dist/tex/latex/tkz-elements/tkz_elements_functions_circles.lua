-- tkz_elements_functions_circles.lua
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

--  define a circle by the center and a radius

-- function circle_cr ( c, r )
--     return c +  point(r,0)
-- end

function midarc_ (o,a,b) -- a -> b
   local phi = 0.5 * get_angle_ ( a,o,b )
   return rotation_ (o,phi,b)
end

function tangent_from_ (c,p,pt)
  local o
   o  = midpoint_ ( c,pt )
 return  intersection_cc_ (o,c,c,p)
end

function tangent_at_ (a,p)
   return rotation_ (p,math.pi/2,a),rotation_ (p,-math.pi/2,a)
end
 
function orthogonal_from_ (a,b,p)
    return tangent_from_ (a,b,p)
end

function orthogonal_through_ (a,b,x,y)
    local d,z
   d =  1/point.mod(x-a)
   z = a +(b-a)*d
   return circum_center_ (x,y,z)
end

 
function inversion_ (c,p,pt)
  local ry = point.abs(c-p)
  local d  = point.abs(c-pt)
  local r  = (ry*ry)/d
  return c+polar_ (r,point.arg(pt-c))
end

function circles_position_ (c1,r1,c2,r2)
   local d,max,min
   d        = point.mod(c1-c2)
   max      = r1+r2
   min      = math.abs ( r1 - r2)
   if d > max then return "outside"
      elseif math.abs(d - max) < tkz_epsilon then return "outside tangent" -- epsilon
      elseif math.abs(d - min) < tkz_epsilon then return "inside tangent" -- epsilon
      elseif d < min then return "inside"
      else return "intersect"
   end
end
 
function radical_axis_ (c1,p1,c2,p2)
   local ci,cj
   r1 = point.abs(c1-p1)
   r2 = point.abs(c2-p2)
   d  = point.abs(c1-c2)
   h  = (r1*r1-r2*r2+d*d)/(2*d)
   ck = radical_center_ (c1,p1,c2,p2)
   cj = rotation_ (ck,-math.pi/2,c1)
   ci = symmetry_ (ck,cj)
   return cj,ci
end

function radical_center_ (c1,p1,c2,p2)
   local d,r1,r2,h
   r1 = point.abs(c1-p1)
   r2 = point.abs(c2-p2)
   d  = point.abs(c1-c2)
   h  = (r1*r1-r2*r2+d*d)/(2*d)
   return h*(c2-c1)/d+c1
end

-- version 1.60 center pour deux cercles ?
function radical_center3 (C1,C2,C3)
   local t1,t2,t3,t4
   t1,t2 = radical_axis_ (C1.center,C1.through,C2.center,C2.through)
   if C3 == nil then
    return  intersection_ll_ (t1,t2,C1.center,C2.center)
   else
   t3,t4 = radical_axis_ (C3.center,C3.through,C2.center,C2.through)
return intersection_ll_ (t1,t2,t3,t4)
end
end


function south_pole_ (c,p)
   local r
   r = point.abs (c-p)
   return c - point (0,r)
 end

function north_pole_ (c,p)
   local r
   r = point.abs (c-p)
   return c + point (0,r)
end

function antipode_ (c,pt)
   return 2 * c - pt
end

function internal_similitude_ (c1,r1,c2,r2)
  return barycenter_ ({c2,r1},{c1,r2})
end

function  external_similitude_ (c1,r1,c2,r2)
  return  barycenter_ ({c2,r1},{c1,-r2})
end

function circlepoint_ (c,t,k)
   local phi = 2*k* math.pi
   return rotation_ (c,phi,t) 
end
 
-- version 1.60 new
function midcircle_(C1,C2)
   local state,r,s,t1,t2,T1,T2,p,a,b,c,d,Cx,Cy,i,j
   state =  circles_position_(C1.center,C1.radius,C2.center,C2.radius)
   i = barycenter_ ({C2.center,C1.radius},{C1.center,-C2.radius})
   j = barycenter_ ({C2.center,C1.radius},{C1.center,C2.radius})
   t1,t2 = tangent_from_ (C1.center,C1.through,i)
   T1,T2 = tangent_from_ (C2.center,C2.through,i)
      
   if (state == 'outside') or (state == 'outside tangent')then
         p = math.sqrt(point.mod(i-t1)*point.mod(i-T1))
   return circle  : radius (i,p)
   elseif state == 'intersect' then
        r,s = intersection (C1,C2)
   return circle  : radius (i,point.mod(r-i)) , circle  : radius (j,point.mod(r-j))
   elseif (state == 'inside') or (state == 'inside tangent') then
      a,b = intersection_lc_ (C1.center,C2.center,C1.center,C1.through)
      c,d = intersection_lc_ (C1.center,C2.center,C2.center,C2.through)
       
      if C1.radius < C2.radius then 
         z.u, z.v, z.r, z.s = a, b, c, d
      else
         z.u, z.v, z.r, z.s = c, d, a, b
      end
      
      if  (in_segment_ (z.s,z.v,z.u) == true) 
      then 
      Cx = circle  :  diameter (z.r,z.v)
      Cy = circle  :  diameter (z.u,z.s)
      else
      Cx = circle  :  diameter (z.s,z.v)
      Cy = circle  :  diameter (z.u,z.r)
      end
      if (Cx.radius) < (Cy.radius) then
         return Cy : orthogonal_from (j) 
      else
         return Cx : orthogonal_from (j)
 end
 end
end
 
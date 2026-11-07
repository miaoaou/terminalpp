-- tkz_elements-circles.lua
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
--                           circles
---------------------------------------------------------------------------
circle = {}
function circle: new (c, t) -- c --> center t --> through
   local type              = 'circle'
   local ct                = line :new (c,t) 
   local opp               = antipode_ (c,t)
   local radius            = point.abs ( c - t )
   local south             = c - point (0,radius)
   local east              = c + point (radius,0)
   local north             = c + point (0,radius)
   local west              = c - point (radius,0)
   local o = { center      = c, 
               through     = t, 
               ct          = ct,
               opp         = opp,
               radius      = radius,
               south       = south,
               east        = east,
               north       = north,
               west        = west,
               type        = type}
   setmetatable(o, self)
   self.__index = self
   return o
end
-- other definition

function circle: radius (center, radius) -- c --> center r --> radius
   return circle : new  (center, center + point( radius,  0 ) )
end

function circle: diameter (za, zb)
   local c = midpoint_(za,zb)
   return circle : new  (c, zb )
end
-----------------------
-- boolean --
-----------------------
function circle: in_out (pt)
    local d
    d = point.abs (pt - self.center)
    if math.abs(d-self.radius) < tkz_epsilon
     then
       return true
    else
       return false
    end
end

function circle: in_out_disk (pt)
    local d
    d = point.abs (pt - self.center)
    if d <= self.radius
     then
       return true
    else
       return false
    end
end

-- new version 1.80 added 
function circle : circles_position (C)
   return circles_position_ (self.center,self.radius,C.center,C.radius)
end

-----------------------
-- real --
-----------------------
function circle: power (pt)
    local d
    d = point.abs (self.center - pt)
    return     d * d - self.radius * self.radius
end
-----------------------
-- points --
-----------------------
function circle: antipode (pt)
   return 2 * self.center - pt
end

function circle: set_inversion (...)
	local tp = table.pack(...)
	local i
    local t = {}
	for i=1,tp.n do
        table.insert( t , inversion_ ( self.center,self.through, tp[i] ) ) 
	end
  return table.unpack ( t )
end

function circle: midarc (z1,z2)
   local phi = 0.5 * get_angle (self.center,z1,z2 )
   return rotation_ (self.center,phi,z1)
end

function circle: point (t)
   local phi = 2*t* math.pi
   return rotation_ (self.center,phi,self.through) 
end

function circle: random_pt (lower, upper)
local t
     math.randomseed( tonumber(tostring(os.time()):reverse():sub(1,6)) )
      phi = lower + math.random() * (upper - lower)
return point (self.center.re+self.radius*math.cos(phi),self.center.im+self.radius*math.sin(phi) )
end

function circle: internal_similitude (C)
  return internal_similitude_ (self.center,self.radius,C.center,C.radius)
end

function circle: external_similitude (C)
  return  external_similitude_ (self.center,self.radius,C.center,C.radius)
end

-----------------------
-- lines --
-----------------------
function circle: tangent_at (pt)
    return line : new ( rotation_ (pt,math.pi/2,self.center),rotation_ (pt,-math.pi/2,self.center))
 end
 
function circle: tangent_from (pt)
   local t1,t2
   t1,t2 = tangent_from_ (self.center,self.through,pt) 
    return line :new (pt,t1),line : new (pt,t2)
 end
 -- version 1.80 
 function circle: radical_axis (C)
    local t1,t2
    if self.radius > C.radius then
    t1,t2 = radical_axis_ (self.center,self.through,C.center,C.through)
 else
    t1,t2 = radical_axis_ (C.center,C.through,self.center,self.through)
 end
  return line :new (t1,t2)
 end
 
 -- version 1.80
function circle: radical_center (C1,C2)
if C2 == nil then
    if self.radius > C1.radius then
       return radical_center_ (self.center,self.through,C1.center,C1.through)
    else
       return radical_center_ (C1.center,C1.through,self.center,self.through)
    end
else
    return radical_center3 (self,C1,C2)
end
end 
 -- version 1.80
function circle : radical_circle (C1,C2)
   local rc
    if C2 == nil then
       rc = self : radical_center (C1)
       return self : orthogonal_from (rc)
    else
       rc = self : radical_center (C1,C2)
       return C1 : orthogonal_from (rc)
    end
end
 
 function circle : external_tangent(C)
   local i,t1,t2,k,T1,T2
      i     = barycenter_ ({C.center,self.radius},{self.center,-C.radius})
      t1,t2 = tangent_from_ (self.center,self.through,i) 
      k     = point.mod((C.center-i)/(self.center-i))
      T1    = homothety_(i,k,t1)
      T2    = homothety_(i,k,t2)
   return line : new (t1,T1),line : new (t2,T2)
 end

 function circle : internal_tangent(C)
   local i,t1,t2,k,T1,T2
      i = barycenter_ ({C.center,self.radius},{self.center,C.radius})
      t1,t2 = tangent_from_ (self.center,self.through,i) 
      k = -point.mod((C.center-i)/(self.center-i))
      T1 = homothety_(i,k,t1)
      T2 = homothety_(i,k,t2)
   return line : new (t1,T1),line : new (t2,T2)
 end
 
 function circle : common_tangent(C)
    local o,s1,s2,t1,t2
   o = external_similitude_ (self.center,self.radius,C.center,C.radius)

  if  self.radius < C.radius then 
     t1,t2 = tangent_from_ (C.center,C.through,o) 
     s1,s2 = tangent_from_ (self.center,self.through,o) 
      return s1,t1,t2,s2
   else
      s1,s2 = tangent_from_ (C.center,C.through,o) 
      t1,t2 = tangent_from_ (self.center,self.through,o) 
       return s1,t1,t2,s2     
   end
 end
 -----------------------
 -- circles --
 -----------------------
function circle: orthogonal_from (pt)
    local t1,t2
     t1,t2 = tangent_from_ (self.center,self.through,pt) 
    return circle : new (pt,t1)
end
 
function circle: orthogonal_through (pta,ptb)
    return circle : new (orthogonal_through_ (self.center,self.through,pta,ptb),pta)
 end

 function circle: inversion_L (L)
    local p,q
    if L: in_out (self.center) then
    return L
  else 
   p =  L: projection (self.center)
   q = inversion_ (self.center,self.through,p) 
   return   circle: new (midpoint_(self.center,q),q) 
  end
 end
   
 function circle: inversion_C (C)
    local p,q,x,y
    if C: in_out (self.center) then
       p = C : antipode (self.center)
       q = inversion_ (self.center,self.through,p) 
       x = ortho_from_ ( q , self.center , p )
       y = ortho_from_ ( q , p, self.center)
    return line : new  (x,y)
  else 
      x,y = intersection_lc_ (self.center,C.center,C.center,C.through)
      X = inversion_ (self.center,self.through,x) 
      Y = inversion_ (self.center,self.through,y) 
      return circle : new (midpoint_(X,Y),X)
   end
 end
 
function circle: inversion (...)
   local obj,nb,t
   local tp = table.pack(...)
   obj = tp[1]
   nb = tp.n
    if nb == 1 then
       if obj.type == "point" then
          return inversion_ (self.center,self.through,obj)
       elseif  obj.type == "line" then
          return self: inversion_L (obj)
       else
          return self: inversion_C (obj)
       end
    else
        t = {}
        for i=1,tp.n do
         table.insert( t , inversion_ (self.center,self.through , tp[i])  ) 
         end
      return table.unpack ( t )     
    end
end
 
function circle: draw ()
   local x,y
    x, y = self.center: get ()
    local r = self.radius
    local frmt = '\\draw (%0.3f,%0.3f) circle [radius=%0.3f];'
    tex.sprint(string.format(frmt, x,y,r))
end 
 
function circle: midcircle(C)
  return midcircle_ (self,C)
end

return circle
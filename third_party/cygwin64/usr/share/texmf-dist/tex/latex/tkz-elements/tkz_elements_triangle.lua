-- tkz_elements_triangles.lua
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

triangle = {}
function triangle: new (za, zb ,zc)
   local type                    = 'triangle'
   local circumcenter            = circum_center_ (za , zb , zc)
   local centroid                = barycenter_ ( {za,1} , {zb,1} , {zc,1} )
   local incenter                = in_center_ (za , zb , zc)
   local orthocenter             = ortho_center_  (za , zb , zc)
   local eulercenter             = euler_center_  (za , zb , zc)
   local spiekercenter           = spieker_center_  (za , zb , zc)
   local c                       = point.abs(zb-za)
   local a                       = point.abs(zc-zb)
   local b                       = point.abs(za-zc)
   local alpha                   = angle_normalize_(point.arg ((zc-za) / (zb-za)))
   local beta                    = angle_normalize_(point.arg ((za-zb) / (zc-zb)))
   local gamma                   = angle_normalize_(point.arg ((zb-zc) / (za-zc)))
   local ab                      = line : new (za,zb)
   local ca                      = line : new (zc,za)
   local bc                      = line : new (zb,zc)
   local o = {  pa               = za, 
                pb               = zb, 
                pc               = zc, 
                type             = type,
                circumcenter     = circumcenter, 
                centroid         = centroid,
                incenter         = incenter, 
                eulercenter      = eulercenter,      
                orthocenter      = orthocenter, 
                spiekercenter    = spiekercenter,
                a                = a, 
                b                = b, 
                c                = c, 
                ab               = ab, 
                ca               = ca, 
                bc               = bc,
                alpha            = alpha, 
                beta             = beta, 
                gamma            = gamma}
    setmetatable(o, self)
    self.__index = self
    return o
end
-----------------------
-- points --
-----------------------
-- version 1.80
function triangle: trilinear (a,b,c)
   return barycenter_ ( {self.pa,a*self.a },{self.pb,b*self.b},{self.pc,c*self.c} )
end

function triangle: barycentric (a,b,c)
   return barycenter_ ( {self.pa,a },{self.pb,b},{self.pc,c} )
end

function triangle: bevan_point ()
   return circum_center_ ( self : excentral_tr())
end

function triangle: mittenpunkt_point ()
   return lemoine_point_ ( self : excentral_tr())
end

function triangle: gergonne_point ()
   return gergonne_point_ ( self.pa , self.pb , self.pc)
end

function triangle: nagel_point ()
return   nagel_point_ ( self.pa , self.pb , self.pc)
end

function triangle: feuerbach_point ()
return   feuerbach_point_ ( self.pa , self.pb , self.pc)
end

function triangle: lemoine_point()
   return lemoine_point_ ( self.pa , self.pb , self.pc)
end

function triangle: symmedian_point()
   return lemoine_point_ ( self.pa , self.pb , self.pc)
end

function triangle: spieker_center()
return spieker_center_ ( self.pa , self.pb , self.pc )
end

function triangle: barycenter (ka,kb,kc)
 return   barycenter_ ({self.pa,ka},{self.pb,kb},{self.pc,kc})
end

function triangle: base (u,v) -- (ab,ac) base coord u,v
    return barycenter_ ({self.pa,(1-u-v)},{self.pb,u},{self.pc,v})
end

function triangle: euler_points ()
   local H
   H = ortho_center_ ( self.pa , self.pb , self.pc )
   return midpoint_ ( H,self.pa ), midpoint_ ( H,self.pb ), midpoint_ ( H,self.pc )
end

function triangle: nine_points ()
   local H,ma,mb,mc,H,ha,hb,hc
    ma,mb,mc = medial_tr_  (  self.pa , self.pb , self.pc)
    ha,hb,hc = orthic_tr_  (  self.pa , self.pb , self.pc)
     H = ortho_center_ ( self.pa , self.pb , self.pc )
    return
    ma,mb,mc,
    ha,hb,hc,
    midpoint_ ( H,self.pa ),
    midpoint_ ( H,self.pb ),
    midpoint_ ( H,self.pc )
end

function triangle : point (t)
   local t1,t2,p
   p = (self.a + self.b + self.c)
   t1 = self.a / p
   t2 = (self.a + self.b) / p
   if t<= t1 then  
      return self.ab : point (t/t1) 
   elseif t <= t2 then
      return self.bc : point ((t*p-self.a)/self.b)
   else
      return self.ca : point ((t*p-self.c-self.b)/self.a)
   end
end

function triangle :  soddy_center ()
   return soddy_center_ (self.pa,self.pb,self.pc)
end
-------------------
-- Result -> line
-------------------
-- N,H,G,O -- check_equilateral_ (a,b,c)
function triangle: euler_line ()
  return  line : new (self.orthocenter,self.circumcenter)
end

function triangle: symmedian_line (n)
   local a,b,c,l
   a = self.pa
   b = self.pb
   c = self.pc
   l = self : lemoine_point ()
   if n==1 then 
           return line : new (b,intersection_ll_ (b,l,a,c)) 
    elseif n==2 then
           return line : new (c,intersection_ll_ (c,l,a,b))
     else
           return line : new (a,intersection_ll_ (a,l,b,c))
    end
end

function triangle: altitude (n)
   local a,b,c,o,p
   a = self.pa
   b = self.pb
   c = self.pc
   o = ortho_center_ (a,b,c)
   if n==1 then 
      p = projection_ (a,c,b)
           return line : new (b,p)
    elseif n==2 then
        p = projection_ (a,b,c)
           return line : new (c,p)
     else
         p = projection_ (b,c,a)
           return  line : new (a,p) 
    end
end

function triangle: bisector (n)
   local a,b,c,i
      a = self.pa
      b = self.pb
      c = self.pc
      i = in_center_ (a,b,c)
      if n==1 then
              return line : new (b,intersection_ll_ (b,i,a,c))
       elseif n==2 then
              return line : new (c,intersection_ll_ (c,i,a,b))
        else
              return  line : new (a,intersection_ll_ (a,i,b,c))
       end
end

function triangle: bisector_ext(n)   -- n =1 swap n=2 2 swap
local a,b,c
   a = self.pa
   b = self.pb
   c = self.pc
 if n==1 then -- ac
         return line : new (b,bisector_ext_ (b,c,a))
  elseif n==2 then -- ab
     
         return line : new (c,bisector_ext_ (c,a,b))
   else -- bc
         return line : new (a,bisector_ext_ (a,b,c))
  end
end

function triangle: antiparallel(pt,n)   -- n =1 swap n=2 2 swap
local a,b,c,i,u,v,w
   a = self.pa
   b = self.pb
   c = self.pc
   i = in_center_ (a,b,c)
 if n==1 then
    u = symmetry_axial_ (b,i,a)
    v = symmetry_axial_ (b,i,c)
    w = ll_from_ ( pt , u , v )
    intersection_ll_ (pt,w,a,b)
        return line : new (intersection_ll_ (pt,w,c,b),intersection_ll_ (pt,w,a,b))
  elseif n==2 then
     u = symmetry_axial_ (c,i,a)
     v = symmetry_axial_ (c,i,b)
     w = ll_from_ ( pt , u , v )
     intersection_ll_ (pt,w,a,c)
         return line : new (intersection_ll_ (pt,w,b,c),intersection_ll_ (pt,w,a,c))
   else
      u = symmetry_axial_ (a,i,b)
      v = symmetry_axial_ (a,i,c)
      w = ll_from_ ( pt , u , v )
      intersection_ll_ (pt,w,b,c)
          return line : new (intersection_ll_ (pt,w,c,a),intersection_ll_ (pt,w,b,a))
  end
end
-----------------------
--- Result -> circles --
-----------------------
function triangle: euler_circle ()
   return circle : new (euler_center_ ( self.pa , self.pb , self.pc),midpoint_( self.pb , self.pc))
end

function triangle: circum_circle()
return   circle : new (circum_circle_ ( self.pa , self.pb , self.pc), self.pa )
end

function triangle: in_circle ()
   local o
   o = in_center_ ( self.pa , self.pb , self.pc)
return  circle : new  (o, projection_ (self.pb , self.pc,o) )
end

function triangle: ex_circle (n)   -- n =1 swap n=2 2 swap
   local a,b,c,o
   a = self.pa
   b = self.pb
   c = self.pc
 if n==1 then 
    o = ex_center_ (b,c,a) 
     return circle : new (o ,   projection_ (c,a,o))
  elseif n==2 then
     o = ex_center_ (c,a,b) 
      return circle : new (o ,   projection_ (a,b,o))
   else
      o = ex_center_ (a,b,c) 
     return  circle : new (o ,   projection_ (b,c,o)) 
  end
end

function triangle: first_lemoine_circle()
local lc,oc
   lc = self: lemoine_point()
   oc = self.circumcenter
return circle : new( midpoint_ (lc,oc),intersection_ll_ (lc,ll_from_ (lc,self.pa,self.pb),self.pa,self.pc))
end

function triangle: second_lemoine_circle()
local lc,a,b,c,r,th
   lc = self: lemoine_point()
    a = point.abs(self.pc-self.pb)
    b = point.abs(self.pa-self.pc)
    c = point.abs(self.pb-self.pa)
    r = ( a*b*c) / (a*a+b*b+c*c)
    th = lc + point(r,0)
  return   circle : new (lc, th )
end

function triangle: spieker_circle()
local ma,mb,mc,sp
      ma,mb,mc =  medial_tr_ ( self.pa , self.pb , self.pc)
      sp       = in_center_ (ma,mb,mc)
return circle : new (sp,projection_ (ma,mb,sp))
end


function triangle :  soddy_circle ()
   local s,i
   s,i = soddy_center_ (self.pa,self.pb,self.pc)
   return circle : new ( s , i )
end
-------------------
-- Result -> triangle
-------------------
function triangle: orthic()
return   triangle : new (orthic_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: medial()
return    triangle : new (medial_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: incentral()
    return   triangle : new (incentral_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: excentral()
    return   triangle : new (excentral_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: intouch()
    return  triangle : new  (intouch_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: contact()
    return  triangle : new  (intouch_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: extouch()
    return  triangle : new  (extouch_tr_ ( self.pa , self.pb , self.pc))
end

function triangle: feuerbach()
    return   triangle : new (feuerbach_tr_ (self.pa , self.pb , self.pc))
end

function triangle: anti ()
     return  triangle : new (anti_tr_  (self.pa,self.pb,self.pc))
end

function triangle: tangential ()
     return triangle : new  (tangential_tr_  (self.pa,self.pb,self.pc))
end

function triangle: cevian (p)
   return triangle : new  (cevian_  (self.pa,self.pb,self.pc,p))
end

function triangle: symmedian ()
   local p
   p = lemoine_point_ ( self.pa , self.pb , self.pc)
   return triangle : new  (cevian_  (self.pa,self.pb,self.pc,p))
end

function triangle: euler ()
   return triangle : new  (euler_points_ (self.pa,self.pb,self.pc) )
end

-------------------
-- Result -> miscellaneous
-------------------
function triangle: get_angle (n)
   local a,b,c
   a = self.pa
   b = self.pb
   c = self.pc
   if n==1 then
         return  point.arg  ((a-b)/(c-b))
    elseif n==2 then
          return  point.arg ((b-c)/(a-c))
     else
          return   point.arg  ((c-a)/(b-a))
    end
end

function triangle: projection (p)
    local x,y,z
    x = projection_ (self.pb,self.pc,p)
    y = projection_ (self.pa,self.pc,p)
    z = projection_ (self.pa,self.pb,p)
  return   x,y,z
end

function triangle: parallelogram ()
    local x = self.pc + self.pa - self.pb
  return  x
end

function triangle: area ()
    return area_(self.pa,self.pb,self.pc)
end

function triangle: barycentric_coordinates (pt)
   return barycentric_coordinates_ (self.pa,self.pb,self.pc,pt)
end

function triangle: in_out (pt)
 return in_out_ (self.pa,self.pb,self.pc,pt)
end

function triangle: check_equilateral ()
 return check_equilateral_ (self.pa,self.pb,self.pc)
end



return triangle
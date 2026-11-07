-- tkz_elements-ellipses.lua
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
ellipse = {}

function ellipse: new(pc, pa ,pb)
-- pc --> center   pa --> through big axe     pb --> little axe
    local type             = 'ellipse'
    local Rx               = point.abs ( pa - pc )
    local Ry               = point.abs ( pb - pc )
    local slope            = slope_ (pc,pa)
    local c                = math.sqrt (Rx*Rx-Ry*Ry) 
    local Fa               = pc + c*(point(math.cos(slope),math.sin(slope)))
    local Fb               = pc - c*(point(math.cos(slope),math.sin(slope)))
    local east             = pa
    local north            = pb
    local west             = 2 * pc - pa
    local south            = 2 * pc - pb
    local vertex           = pa
    local covertex         = pb
    local o = {   center   = pc,
                  vertex   = vertex,
                  covertex = covertex,
                  Rx       = Rx,
                  Ry       = Ry,
                  slope    = slope,
                  Fa       = Fa,
                  Fb       = Fb,
                  type     = type,
                  north    = north,
                  south    = south,
                  east     = east,
                  west     = west  }
    setmetatable(o, self)
    self.__index = self
    return o
end

function ellipse: foci (f1,f2,v )
   local c,a,h,b,cov
   c = midpoint_ (f1,f2)
   a = point.abs(v-c)
   h = point.abs(f1-c)
   b = math.sqrt(a^2-h^2)
   cov = (v-c)*point(0,1)/point.abs(v-c)*b+c
    return ellipse: new (c,v,cov)     
end

function ellipse: radii (c,a,b,sl )
   local z,v,cov
   z = point (a*math.cos(sl),a*math.sin(sl))
   v     =  c +  z
   z.V = v
   cov   =  (v-c)*point(0,1)/point.abs(v-c)*b+c
return ellipse: new (c,v,cov)     
end

function ellipse: point (t)
   local phi = 2*t* math.pi
   local ax,ay,bx,by,cx,cy
   cx =  self.center.re
   cy =  self.center.im
   ax =  self.Rx * math.cos(self.slope) * math.cos(phi)
   ay =  self.Rx * math.sin(self.slope) * math.cos(phi)
   bx = -self.Ry * math.sin(self.slope) * math.sin(phi)
   by =  self.Ry * math.cos(self.slope) * math.sin(phi)
return point (cx+ax+bx,cy+ay+by)
end

function ellipse: tangent_at (pt)
   local zi,u,v
   zi = in_center_ (self.Fa,pt,self.Fb)
   u = pt+(zi-pt)*point(0,1)
   v = pt : symmetry (u)
    return line : new (u,v)
end

function ellipse: tangent_from (pt)
   local da,db,zx,zy,zz,u,v,L,U,V
   zx    = 2 * self.Rx * math.cos(self.slope)
   zy    = 2 * self.Rx * math.sin(self.slope)
   zz    = self.Fb + point(zx,zy)
   s1,s2 = intersection_cc_ (pt,self.Fa,self.Fb,zz)
   u,v   = mediator_ (s2,self.Fa)
   L     = line: new (u,v)
   U,U   = intersection_le (L,self)
   u,v   = mediator_ (s1,self.Fa)
   L     = line: new (pt,u)
   V,V  = intersection_le (L,self)
   return line : new (pt,U), line : new (pt,V)
end

function ellipse: in_out (pt)
     local d,D,an,m
     d = point.abs (pt - self.center)
     an = point.arg (pt - self.center)
     m = point(self.Rx*math.cos(an),self.Ry*math.sin(an))
     D = point.abs (m - self.center)
     if D-d > tkz_epsilon
      then
        return true
     else
        return false
     end
end
return ellipse
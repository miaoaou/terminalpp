-- tkz_elements_lines.lua
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

-- -------------------------------------------------------------------------
--                           Lines
-- -------------------------------------------------------------------------
line = {}
function line: new(za, zb)
    local type             = 'line'
    local mid              = (za+zb)/2
    local north_pa         = rotation_ (za,math.pi/2,zb)
    local south_pa         = rotation_ (za,-math.pi/2,zb)
    local north_pb         = rotation_ (zb,-math.pi/2,za)
    local south_pb         = rotation_ (zb,math.pi/2,za)
    local west             = rotation_ (za,math.pi/2,north_pa)
    local east             = rotation_ (zb,math.pi/2,south_pb)
    local slope            = angle_normalize_(point.arg(zb-za))
    local length           = point.mod(zb-za)
    local vec              = vector : new (za,zb)
    local o = {pa          = za, 
               pb          = zb,
               north_pa    = north_pa, 
               south_pa    = south_pa,
               west        = west, 
               east        = east,
               north_pb    = north_pb, 
               south_pb    = south_pb,
               slope       = slope,
               mid         = mid, 
               type        = type, 
               vec         = vec,
               length      = length}
    setmetatable(o, self)
    self.__index = self
    return o
end

-------------------
-- Result -> real
-------------------
function line: distance (pt)   
    return point.mod(projection(self,pt)-pt)
end

function length(a,b)
  return  point.abs (a-b)
end

function line: slope ()
   return slope_(self.pa,self.pb)
end
-------------------
-- Result -> boolean
-------------------
function line: in_out (pt)
    local sc
    sc = math.abs ((pt-self.pa)^(pt-self.pb))
    if sc <= tkz_epsilon
     then
       return true
    else
       return false
    end
end

function line: in_out_segment (pt)
    local sc
    sc = point.mod (pt-self.pa) + point.mod (pt-self.pb) - point.mod(self.pb-self.pa)
    if sc <= tkz_epsilon
     then
       return true
    else
       return false
    end
end
-------------------
-- Result -> point
-------------------
function line: barycenter (ka,kb)
    return barycenter_({self.pa,ka},{self.pb,kb})
end

function line: point (t) --  t=o A  t=1 B  t = AM / AB 
    return barycenter_({self.pa,1-t},{self.pb,(t)})
end

function line: midpoint ()
    return (self.pa+self.pb)/2
end

function line: harmonic_int (pt)
    return div_harmonic_int_(self.pa,self.pb,pt)
end

function line: harmonic_ext (pt)
    return div_harmonic_ext_(self.pa,self.pb,pt)
end

function line: harmonic_both (k)
    return div_harmonic_both_(self.pa,self.pb,k)
end

function line: gold_ratio()
   return self.pa + (self.pb-self.pa)*tkzinvphi
end

function line: normalize ()
   return  self.pa+(self.pb-self.pa)/point.mod(self.pb-self.pa)
end

function line: normalize_inv ()
   return normalize_ (self.pb,self.pa)
end

function line: _east (d)
   local d = d or 1
   return self.pb+ d/self.length * (self.pb-self.pa)
end

function line: _west (d)
   local d = d or 1
   return self.pa+ d/self.length * (self.pa-self.pb)
end

function line: _north_pa (d)
   local d = d or 1
   return d/self.length * ( self.north_pa - self.pa ) + self.pa
end

function line: _south_pa (d)
   local d = d or 1
   return d/self.length *( self.south_pa - self.pa ) + self.pa
end

function line: _south_pb (d)
   local d = d or 1
   return d/self.length *( self.south_pb - self.pb ) + self.pb
end

function line: _north_pb (d)
   local d = d or 1
   return d/self.length *( self.north_pb - self.pb ) + self.pb
end
-------------- transformations -------------
function line: translation_pt ( pt )
    return translation_ ( self.pb-self.pa,pt )
end

function line: translation_C ( obj )
   local pa,pb,x,y
   pa = obj.center
   pb = obj.through
   x,y = set_translation_ ( self.pb-self.pa,pa,pb )
   return circle : new  (x,y)
end

function line: translation_T ( obj )
   local pa,pb,pc,x,y,z
   pa    = obj.pa
   pb    = obj.pb
   pc    = obj.pc
   x,y,z = set_translation_ ( self.pb-self.pa,pa,pb,pc )
   return triangle : new  (x,y,z)
end

function line: translation_L ( obj )
   local pa,pb,x,y
   pa = obj.pa
   pb = obj.pb
   x,y = set_translation_ ( self.pb-self.pa,pa,pb )
   return line : new  (x,y)
end

function line: translation (...)
   local obj,nb,t
   local tp = table.pack(...)
   obj = tp[1]
   nb = tp.n
    if nb == 1 then
       if obj.type == "point" then
          return translation_ ( self.pb-self.pa,obj )
       elseif  obj.type == "line" then
          return self: translation_L (obj)
       elseif obj.type == "triangle" then
          return self: translation_T (obj)
       else
          return self: translation_C (obj)
       end
    else
      t = {}
       for i=1,tp.n do
      table.insert(t , translation_ ( self.pb-self.pa , tp[i])) 
         end
      return table.unpack ( t )      
    end
end

function line: set_translation ( ...)
    return set_translation_ ( self.pb-self.pa,... )
end

function line: projection (...)
   local obj,nb,t
   local tp = table.pack(...)
   obj = tp[1]
   nb = tp.n
    if nb == 1 then
          return projection_ ( self.pa, self.pb, obj )
    else
        t = {}
        for i=1,tp.n do
            table.insert( t , projection_ (self.pa, self.pb, tp[i])  ) 
         end
      return table.unpack ( t )      
    end
end

function line: set_projection (...)
	local tp = table.pack(...)
	local i
    local t = {}
	for i=1,tp.n do
        table.insert( t , projection_ (self.pa,self.pb , tp[i])  ) 
	end
  return table.unpack ( t )
end

function line: symmetry_axial_L ( obj )
   local pa,pb,x,y
   pa = obj.pa
   pb = obj.pb
   x,y = self:set_reflection ( pa,pb )
   return line : new  (x,y)
end
function line: symmetry_axial_T ( obj )
   local pa,pb,pc,x,y,z
   pa    = obj.pa
   pb    = obj.pb
   pc    = obj.pc
   x,y,z = self:set_reflection ( pa,pb,pc )
   return triangle : new  (x,y,z)
end

function line: symmetry_axial_C ( obj )
   local pa,pb,x,y
   pa = obj.center
   pb = obj.through
   x,y = self:set_reflection ( pa,pb )
   return circle : new  (x,y)
end

function line: reflection (...)
   local obj,nb,t
   local tp = table.pack(...)
   obj = tp[1]
   nb = tp.n
    if nb == 1 then
       if obj.type == "point" then
          return symmetry_axial_ ( self.pa,self.pb,obj )
       elseif  obj.type == "line" then
          return self: symmetry_axial_L (obj)
       elseif obj.type == "triangle" then
          return self: symmetry_axial_T (obj)
       else
         return self: symmetry_axial_C (obj)
       end
    else
        t = {}
        for i=1,tp.n do
            table.insert( t , symmetry_axial_ ( self.pa,self.pb , tp[i])  ) 
         end
      return table.unpack ( t )      
    end
end

function line: set_reflection (...)
    return set_symmetry_axial_ ( self.pb,self.pa,... )
end

-------------------
-- Result -> line
-------------------
function line: ll_from ( pt )
	return line : new (pt,pt+self.pb-self.pa) 
end

function line: ortho_from ( pt )
	return  line : new (pt+(self.pb-self.pa)*point(0,-1),pt+(self.pb-self.pa)*point(0,1))
end

function line: mediator () 
   local m
   m = midpoint_ (self.pa,self.pb)
  return line : new (rotation_ (m,-math.pi/2,self.pb),rotation_ (m,math.pi/2,self.pb)) 
end
-------------------
-- Result -> circle
-------------------
function line: circle ()   
    return circle : new (self.pa,self.pb)
end

function line: circle_swap ()   
    return circle : new (self.pb,self.pa)
end

function line : diameter ()
   local c = midpoint_ (self.pa,self.pb)
  return circle : new  (c,self.pb)
end

function line : apollonius (k)
   local z1,z2,c
    z1     = barycenter_ ({self.pa,1},{self.pb,k})
    z2     = barycenter_ ({self.pa,1},{self.pb,-k})
    c = midpoint_ (z1,z2)
  return circle : new  (c,z2)
end

----------------------
-- Result -> triangle
----------------------
function line: equilateral (swap)
    if swap == nil then
        swap = false
    end
   if swap  then 
        return triangle : new (self.pa,self.pb,rotation_ (self.pa,-math.pi/3,self.pb))
    else
          return triangle : new (self.pa,self.pb,rotation_ (self.pa,math.pi/3,self.pb)) 
end
end

function line: isosceles (phi,swap)
      local pta,ptb
       if swap == nil then
       swap = false
   end
  if swap  then 
     pta = rotation_ (self.pa,-phi,self.pb)
     ptb = rotation_ (self.pb,phi,self.pa)
   return triangle : new (self.pa,self.pb, intersection_ll_ (self.pa,pta,self.pb,ptb ))
  else
    pta = rotation_ (self.pa,phi,self.pb)
    ptb = rotation_ (self.pb,-phi,self.pa)
  return triangle : new (self.pa,self.pb, intersection_ll_ (self.pa,pta,self.pb,ptb ))
end
end

function line: two_angles (alpha,beta)
   local pta,ptb,pt
   pta = rotation_ (self.pa,alpha,self.pb)
   ptb = rotation_ (self.pb,-beta,self.pa)
   pt = intersection_ll_ (self.pa,pta,self.pb,ptb)
   return triangle : new (self.pa,self.pb,pt)
end

function line: school ()
   local pta,ptb,pt
   pta = rotation_ (self.pa,math.pi/6,self.pb)
   ptb = rotation_ (self.pb,-math.pi/3,self.pa)
   pt = intersection_ll_ (self.pa,pta,self.pb,ptb)
   return triangle : new (self.pa,self.pb,pt)
end

function line: half ()
   local x,pt
   x  = midpoint_(self.pa,self.pb)
   pt = rotation_ (self.pb,-math.pi/2,x)
   return triangle : new (self.pa,self.pb,pt)
end

function line: sss (a,b)
   local pta,ptb,i,j
    pta = self.pa + point ( a,  0 )
    ptb = self.pb + point ( -b , 0)
    i,j = intersection_cc_ (self.pa,pta,self.pb,ptb)
   return triangle : new (self.pa,self.pb,i),triangle : new (self.pa,self.pb,j)
end

function line: ssa (a,phi)
   local x,y,pt
    x = rotation_ (self.pb,-phi,self.pa)
    y = self.pa + polar_ ( a , self.slope)
    i,j = intersection_lc_ (self.pb,x,self.pa,y)
   return triangle : new (self.pa,self.pb,i),triangle : new (self.pa,self.pb,j)
end

function line: sas (a,phi)
   local x,pt
    x  = self.pa + polar_ ( a , self.slope)
    pt = rotation_ (self.pa,phi,x)
   return triangle : new (self.pa,self.pb,pt)
end
---- sacred triangles ----

function line: gold (swap)
    local pt
     if swap == nil then
     swap = false
 end
if swap  then  
   pt = rotation_ (self.pa,-math.pi/2,self.pb)
    return triangle : new (self.pa,self.pb, self.pa + (pt-self.pa) * tkzinvphi)
else
    pt = rotation_ (self.pa,math.pi/2,self.pb)
     return triangle : new (self.pa,self.pb, self.pa + (pt-self.pa) * tkzinvphi)
end
end

function line: sublime ()
   local pta,ptb,pt
  pta = rotation_ (self.pa,2*math.pi/5,self.pb)
  ptb = rotation_ (self.pb,-2*math.pi/5,self.pa)
  pt = intersection_ll_ (self.pa,pta,self.pb,ptb)
  return triangle : new (self.pa,self.pb,pt)
end

line.euclid = line.sublime

function line: euclide (swap)
 if swap == nil then
   return triangle : new (self.pa,self.pb, rotation_ (self.pa,math.pi/5,self.pb))
  else  
     return triangle : new (self.pa,self.pb, rotation_ (self.pa,-math.pi/5,self.pb))
   end
  end

function line: divine ()
   local pta,ptb,pt,h
   pta = rotation_ (self.pa,math.pi/5,self.pb)
   ptb = rotation_ (self.pb,-math.pi/5,self.pa)
   pt = intersection_ll_ (self.pa,pta,self.pb,ptb)
   return triangle : new (self.pa,self.pb,pt)
end

function line: cheops ()
   local m,n,pt
   m = midpoint_ (self.pa,self.pb)
   n = rotation_ (m,- math.pi/2,self.pa)
   pt = m + (n-m)* tkzsqrtphi
   return triangle : new (self.pa,self.pb,pt)
end

function line: egyptian ()
   local n,pt
    n = rotation_ (self.pb,- math.pi/2,self.pa)
    pt = self.pb + (n-self.pb)/point.mod(n-self.pb)*self.length* 0.75
   return triangle : new (self.pa,self.pb,pt)
end
line.pythagoras = line.egyptian
line.isis = line.egyptian
line.golden = line.sublime
line.golden_gnomon = line.divine

------------------------------
-- Result -> couple of points
------------------------------
function line: square (swap)
   if swap == nil  
   then
      return square : side (self.pa,self.pb)
   else
      return square : side (self.pa,self.pb,indirect)
   end
end

function line  : report (d,pt)
   local t
   t = d/self.length
   if pt == nil  
   then  
   return barycenter_({self.pa,1-t},{self.pb,(t)})
else 
   return barycenter_({self.pa,1-t},{self.pb,(t)}) +pt-self.pa
end
end

return line
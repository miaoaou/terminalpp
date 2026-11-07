-- tkz_elements_intersections.lua
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

-------------------------------------------------------------------------
-- intersection of lines
-------------------------------------------------------------------------
function intersection_ll (la,lb)
    return intersection_ll_ (la.pa,la.pb,lb.pa,lb.pb)
end
---------------------------------------------------------------------------
-- intersection of a line and a circle
---------------------------------------------------------------------------
function intersection_lc (D,C )
    return intersection_lc_ ( D.pa,D.pb ,C.center,C.through )
end -- function
---------------------------------------------------------------------------
-- intersection of two circles
---------------------------------------------------------------------------
function intersection_cc (Ca , Cb )
   return intersection_cc_(Ca.center,Ca.through,Cb.center,Cb.through)
end -- function

--  line ellipse
function intersection_le (L,E)
   local a,b,c,d,t1,t2,z1,z2,A,B,Bx,By,Ax,Ay,Rx,Ry,sd
   A = (L.pa - E.center)*(point(math.cos(E.slope),-math.sin(E.slope)))
   B = (L.pb - E.center)*(point(math.cos(E.slope),-math.sin(E.slope)))
   Rx  = E.Rx
   Ry  = E.Ry
   Ax  = A.re
   Ay  = A.im
   Bx  = B.re
   By  = B.im
    a  = Rx^2  * (By-Ay)^2 +Ry^2 * (Bx-Ax)^2
    b  = 2 * Rx^2 * Ay * (By-Ay)+ 2  * Ry^2 * Ax * (Bx-Ax)
    c  = Rx^2 * Ay^2 + Ry^2 * Ax^2 -  Rx^2 * Ry^2
    d  =  b^2 - 4 * a * c

   if d > 0 then
      sd = math.sqrt(d)
      t1 = (-(b)+sd)/(2*a)
      t2 = (-(b)-sd)/(2*a)
      z1 = point ( Ax + (Bx-Ax)*t1 ,  Ay + (By-Ay)*t1 )
      z2 = point ( Ax + (Bx-Ax)*t2 ,  Ay + (By-Ay)*t2 )
      if angle_normalize (point.arg(z1)) < angle_normalize (point.arg(z2))
      then
      return z1*(point(math.cos(E.slope),math.sin(E.slope))) + E.center,
             z2*(point(math.cos(E.slope),math.sin(E.slope))) + E.center
          else
             return z2*(point(math.cos(E.slope),math.sin(E.slope))) + E.center,
                    z1*(point(math.cos(E.slope),math.sin(E.slope))) + E.center
            end -- if 
       elseif  math.abs(d) < tkz_epsilon 
       then
           t1 = (-(b))/(2*a)
           z1 = point ( Ax + (Bx-Ax)*t1 ,  Ay + (By-Ay)*t1 )           
          return z1*(point(math.cos(E.slope),math.sin(E.slope))) + E.center,
                 z1*(point(math.cos(E.slope),math.sin(E.slope))) + E.center
       else
         return false,false
      end
end
 
function intersection_ll_ (a,b,c,d)
    local x1,x2,x3,x4,y1,y2,y3,y4,DN,NX,NY
    x1 = a.re
    y1 = a.im
    x2 = b.re
    y2 = b.im
    x3 = c.re
    y3 = c.im
    x4 = d.re
    y4 = d.im
    DN = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4)
    if math.abs ( DN ) < tkz_epsilon then
     return false 
     else
    NX = (x1*y2-y1*x2)*(x3-x4) - (x1-x2)*(x3*y4-y3*x4)
    NY = (x1*y2-y1*x2)*(y3-y4) - (y1-y2)*(x3*y4-y3*x4)
    return point (NX/DN,NY/DN)
    end
end

function intersection_lc_ (pa,pb,c,p)
  local zh, dh, arg_ab, test, phi,c1,c2,r
    r = point.mod (c-p)
    zh = projection_ (pa,pb,c)
    dh = point.abs  (c - zh)
    arg_ab = point.arg (pa - pb)
   if dh < tkz_epsilon
       then
       return
            c + polar_ (r , math.pi + arg_ab),  -- through center
            c + polar_ (r ,  arg_ab)
  elseif math.abs (r - dh) < tkz_epsilon
    then 
        return zh , zh -- tangent
  elseif dh > r
    then return false , false -- no intersection
  else
    phi = math.asin (dh / r)
   -- phi = angle_normalize(phi)
    test = (pa-pb) * point.conj (c-zh)
    if test.im < 0
    then phi = math.pi + phi
      end
      c1 = angle_normalize (arg_ab + phi )
      c2 = angle_normalize (math.pi + arg_ab - phi )
      if c2 < c1 then
    return
    c + polar_ (r, c2) ,
    c + polar_ (r, c1)
 else 
    return
    c + polar_ (r, c1) ,
    c + polar_ (r, c2)
   end -- if
   end -- if

end -- function

function intersection_cc_ (ca,pa,cb,pb )
  local d, cosphi, phi,ra,rb,c1,c2,epsilon
    epsilon = 12
    d  = point.abs (ca - cb)
    ra = point.abs (ca - pa)
    rb = point.abs (cb - pb)
  cosphi = tkzround(((ra * ra + d * d - rb * rb)
                           /( 2 * ra * d )) , epsilon)  
  phi =  tkzround (math.acos(cosphi),epsilon)
  if not phi then 
     return false , false 
  elseif phi == 0 then
     return ca + polar_ (ra, phi + point.arg (cb - ca)) ,
            ca + polar_ (ra, phi + point.arg (cb - ca))
  else
     c1 = angle_normalize ( phi + point.arg(cb - ca))
     c2 = angle_normalize (-phi + point.arg(cb - ca))
  if c1 < c2 then
      return
   ca + polar_(ra,  c1),
   ca + polar_(ra, c2)
else
   return
   ca + polar_(ra,  c2),
   ca + polar_(ra, c1)
      end -- if
   end -- if
 end -- function

function intersection ( X , Y )
   local i,z1,z2
   local t = {}

   if X.type == 'circle'
   then 
         if Y.type == 'circle'
         then
            z1,z2 = intersection_cc ( X , Y )
            table.insert (t , z1 )
            table.insert (t , z2 )
         else -- Y[i] est une droite
            z1,z2 = intersection_lc ( Y , X )
            table.insert (t , z1 )
            table.insert (t , z2 )
         end -- if
   else 
      if X.type == 'line' then
         if Y.type == 'circle'
         then
            z1,z2 = intersection_lc ( X , Y )
            table.insert ( t , z1 )
            table.insert ( t , z2 )
         else 
            if Y.type == 'line' then
            z1 = intersection_ll ( X , Y )
            table.insert (t , z1 )
         else -- ellipse
            z1,z2 = intersection_le ( X , Y )
            table.insert ( t , z1 )
            table.insert ( t , z2 )
         end
         end -- if
      else 
         if  X.type == 'ellipse' then
         z1,z2 = intersection_le ( Y,X)
         table.insert ( t , z1 )
         table.insert ( t , z2 )
       end
      end
   end -- if
   return table.unpack ( t )
end -- function 
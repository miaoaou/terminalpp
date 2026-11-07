% metapost library for the lualatex package luamesh
%
vardef circumcircle(expr M,N,P)=
   % Compute center and radius of the circoncircle of the triangle M N P
  save a,b,c,circle,rad,center,x,y;
  path circle;
  pair  center;
  numeric a,b,c,x[],y[];
  x1 := xpart M;
  y1 := ypart M;
  x2 := xpart N;
  y2 := ypart N;
  x3 := xpart P;
  y3 := ypart P;
  2*x1*a+2*y1*b+c=-(x1**2+y1**2);
  2*x2*a+2*y2*b+c=-(x2**2+y2**2);
  2*x3*a+2*y3*b+c=-(x3**2+y3**2);
  center := (-a,-b);
  rad := sqrt(a**2+b**2-c**2);
  circle := fullcircle scaled (2.0*rad) shifted center;
  circle
enddef


% ======================
% Developed by FHZ -- Brasil
% ======================
#include:tikz

% =======================================================
% Summary of commands
% -- Mirrors
\mirrorSphGauss[arrows]{f}{p}{o}{epsilon}#n
\mirrorSphGaussCoord[arrows]{f}{p}{o}{epsilon}#n
\mirrorSphGaussFixed[arrows]{f}{p}{o}{epsilon}{yM}{xL}{xR}#n
\mirrorSphGaussFixedCoord[arrows]{f}{p}{o}{epsilon}{yM}{xL}{xR}{Co}#n
% -- Lens
\lensSphGauss[arrows]{f}{p}{o}{epsilon}#n
\lensSphGaussCoord[arrows]{f}{p}{o}{epsilon}#n
\lensSphGaussFixed[arrows]{f}{p}{o}{epsilon}{yM}{xL}{xR}#n
\lensSphGaussFixedCoord[arrows]{f}{p}{o}{epsilon}{yM}{xL}{xR}{Co}#n
% =======================================================
\mirrorLensObjIma{p}{pp}{o}{i}#n
\mirrorLensCoord{p}{pp}{o}{i}{f}{Co}#n
% =======================================================
\mirrorBase{f}{yM}{minXaxis}{maxXaxis}#n
\mirrorPts{v}{f}{c}#n
\mirrorRays{p}{pp}{o}{i}#n
% =======================================================
\lensBase{f}{yM}{minXaxis}{maxXaxis}#n
\lensPts{v}{f}{c}#n
\lensRays{p}{pp}{o}{i}#n
% =======================================================
\mirrorMath{f}{p}{o}{epsilon}{yM}#n
\lensMath{f}{p}{o}{epsilon}{yM}#n
\lensMathL{f}{p}{o}{epsilon}{yM}#n
% =======================================================

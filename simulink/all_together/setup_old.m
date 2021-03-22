clc;
close all;
clear;

% Constants
frm2pix = visionhdl.FrameToPixels;
frm2pix.VideoFormat = '480p';
[activePixelsPerLine, activeLines, totalPixels] = getparamfromfrm2pix(frm2pix);
simTime = 10;

evalin('base','pixelcontrolbus');

ROWS = 480;
COLS = 640;

% Initial Parameters
filter_dim = [3, 3];
good_sigma = 0.75;

% Filter Coefficients
Hg = fspecial('gaussian',filter_dim,good_sigma);
[U,S,V]=svd(Hg);
Hv=abs(U(:,1)*sqrt(S(1,1)));
Hh=abs(V(:,1)'*sqrt(S(1,1)));

Hg = [1 0 -1;
       2 0 -2;
       1 0 -1];
[U,S,V]=svd(Hg);
ixsobelv=abs(U(:,1)*sqrt(S(1,1)));
ixsobelh=abs(U(:,1)*sqrt(S(1,1)));

Hg = [1 2 1;
      0 0 0;
     -1 -2 -1];
[U,S,V]=svd(Hg);
iysobelv=U(:,1)*sqrt(S(1,1));
iysobelh=V(:,1)'*sqrt(S(1,1));

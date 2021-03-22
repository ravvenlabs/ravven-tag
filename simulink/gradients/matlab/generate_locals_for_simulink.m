clear;
clc;
close all;

%% Creates data locals for simulink.
% This is automatically ran by the Simulink model

% block settings for 240p, change format as appropriate
frm2pix = visionhdl.FrameToPixels;
frm2pix.VideoFormat = '480p';
[activePixelsPerLine, activeLines, totalPixels] = getparamfromfrm2pix(frm2pix);
numFrames = 10;
simTime = totalPixels * numFrames;

% pixel control bus
evalin('base','pixelcontrolbus');

% Initial Parameters
filter_dim = [3, 3];
good_sigma = 0.75;
good_fixdt = fixdt(1, 16);
good_fixdt_hg = fixdt(1, 16, 15);

% Filter Coefficients
verticle = [-1 -2 -1; 0 0 0; 1 2 1];
horizontal = [1 0 -1; 2 0 -2; 1 0 -1];

Hg = fspecial('gaussian',filter_dim,good_sigma);
[U,S,V]=svd(Hg);
Hv=abs(U(:,1)*sqrt(S(1,1)));
Hh=abs(V(:,1)'*sqrt(S(1,1)));

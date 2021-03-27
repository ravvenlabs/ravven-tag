function quads = quad_ravven4(image_blurred,image_gray,debug)
Debug_Gradient = 0;

width = size(image_gray,2);
height = size(image_gray,1);

%image_blurred = image_blurred .* 255;
%image_blurred = uint8(image_blurred);
%image_blurred = single(image_blurred);
%image_blurred = image_blurred ./ 255;

%Stage 2: Calculating Gradients (Without toolbox)
dx = [ 0, 0,0;...
       1, 0,-1;...
       0, 0,0];
dy = [ 0, 1,0;...
       0, 0,0;...
       0,-1,0];
Ix = conv2(image_blurred,dx,'same');  %Convolve across x direction of image
Iy = conv2(image_blurred,dy,'same');  %Convolve across y direction of image

if(Debug_Gradient == 1)
    Ixn = NormalizeVals(Ix);
    Iyn = NormalizeVals(Iy);
    figure('Name','Stage 2a(Debug): Gradient Magnitue (x direction)');
    imshow(Ixn);
    title('Stage 2a: Gradient Magnitue (x direction)');
    figure('Name','Stage 2a(Debug): Gradient Magnitue (y direction)');
    imshow(Iyn);
    title('Stage 2a: Gradient Magnitue (y direction)');
end

gm = single(Ix.^2 + Iy.^2);   %Magnitude
gd = single(atan2(Iy,Ix));    %Direction

% ix = single(ix);
% iy = single(iy);
% 
% ix = ix./255;
% iy = iy./255;
% 
% gm = single(ix.^2 + iy.^2);   %Magnitude
% gd = single(atan2(iy,ix));    %Direction
% 
% gm = single(imread('../pics/data/mag.tif'));
% gd = single(imread('../pics/data/theta.tif'));
% 
% gd = gd - 32768;
% gd = gd .* pi/32768;
% 
% gm = gm ./ 8192;
% 
% ixx = single(ix);
% iyy = single(iy);
% gm = ixx.^2 + iyy.^2;
% gm = gm./32768;

gm = readmatrix('../pics/data/mag.csv');
gd = readmatrix('../pics/data/theta.csv');

% figure
% subplot(1,2,1)
% imagesc(gm_default)
% subplot(1,2,2)
% imagesc(gm)

if(debug == 1)
figure('Name','Stage 2a: Gradient Magnitue');
imagesc(gm);
colorbar;
title('Stage 2a: Gradient Magnitue');
    thisdir = getOutputdir();
    saveas(gca,[thisdir,'\','GradMag']);

figure('Name','Stage 2b: Gradient Direction');
imagesc(gd .* 180/pi);
colorbar;
title('Stage 2b: Gradient Direction');
    thisdir = getOutputdir();
    saveas(gca,[thisdir,'\','GradDir']);
end

FoundSegs = ravven_detect4(image_gray*256,gm,gd,debug);

if(debug == 1)
    figure('Name','Segments');
    imshow(image_gray);
    title('Segments');
    hold on;
    %Debug Code
    for k = 1:length(FoundSegs)
        LineColor = [146/255,abs(FoundSegs(k,5))/(4*pi),1];
        plot([FoundSegs(k,1),FoundSegs(k,3)],...
           [FoundSegs(k,2),FoundSegs(k,4)],...
           'LineWidth',2,'color',LineColor);%plot the segment
    end
    hold off;
        thisdir = getOutputdir();
    saveas(gca,[thisdir,'\','Segments']);
    
end
if(isempty(FoundSegs))
    quads = [];
    return;
end

%Stage 6: Chain Segments
linked_segments = LinkSegs(FoundSegs);
%Stage 7: Find Quads
quads = QuadDetection(linked_segments,FoundSegs);
if(debug == 1)
    %Debug visualization
    figure('Name','Detected Quads with intersections');
    imshow(image_gray);
    title('Detected Quads with intersections');
    hold on;
    for i = 1:size(quads,1)
        Seg1 = [quads(i,1),quads(i,3); quads(i,2), quads(i,4)];
        Seg2 = [quads(i,3),quads(i,5); quads(i,4), quads(i,6)];
        Seg3 = [quads(i,5),quads(i,7); quads(i,6), quads(i,8)];
        Seg4 = [quads(i,7),quads(i,1); quads(i,8), quads(i,2)];
        
        plot(Seg1(1,:),Seg1(2,:),'r-','LineWidth',2);
        plot(Seg2(1,:),Seg2(2,:),'r-','LineWidth',2);
        plot(Seg3(1,:),Seg3(2,:),'r-','LineWidth',2);
        plot(Seg4(1,:),Seg4(2,:),'r-','LineWidth',2);
        scatter([quads(i,1),quads(i,3),quads(i,5),quads(i,7)],...
            [quads(i,2),quads(i,4),quads(i,6),quads(i,8)],15,'go');
        scatter([sum(Seg1(1,:))/2,sum(Seg2(1,:))/2,sum(Seg3(1,:))/2,sum(Seg4(1,:))/2],...
            [sum(Seg1(2,:))/2,sum(Seg2(2,:))/2,sum(Seg3(2,:))/2,sum(Seg4(2,:))/2],15,'go');
    end
    thisdir = getOutputdir();
    saveas(gca,[thisdir,'\','DetectedQuads']);
end
end

function output = NormalizeVals(input,Max,Min)
    switch nargin
        case 1
            output = (input-min(input(:)))./(max(input(:))-min(input(:)));
        otherwise
            output = (input-Min)./(Max-Min);
    end
end

function dir = getOutputdir
global outputdir
dir = outputdir;
end

function longArray = ArraytoList(Array)
%Turns a NxM array into a 1xN*M list 
Width = size(Array,2);
Height  = size(Array,1);

longArray = zeros(1,Width*Height);
for i = 1:Height
    StartIdx = ((i-1) * Width)+1;
    EndIdx   = (StartIdx + Width)-1;
    longArray(1,StartIdx:EndIdx) = Array(i,:);
end
end
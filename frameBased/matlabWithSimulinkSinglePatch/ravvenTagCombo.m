debug = 1;
fpga = 1;
inputImage = imread('input.bmp');
image_gray = single(rgb2gray(inputImage));

if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_gray);
    axis off
    title('Preprocessing: Grayscale');
    %saveas(FigH,append('outputs\grayscale.png'))
    close(FigH)
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Stage 1: Gaussian Blurring (Without toolbox)
G = [0.0571, 0.1248 ,0.0571;
     0.1248, 0.2725, 0.1248;
     0.0571, 0.1248 ,0.0571];
image_blurred = conv2(image_gray,G,'same'); %Convolve across image

% function Result = frameBasedGaussianFilter(image_gray, G)
%     Result = hdl.npufun(@gaussianKernel, [3 3], ImageIn,'NonSampleInput', data); 
% end
% 
% function e = gaussianKernel(in,data)
%     temp = in.*data;
%     e = sum(sum(temp));
% end

%if fpga == 1
%    sim("models\gaussianFilter\gaussianFilter.slx",1);
%    image_blurred = image_blurred_mdl;
%end

%Displaying the results of blurring
%if(debug == 1)
%    FigH = figure('Position', get(0, 'Screensize'));
%    imagesc(image_blurred);
%    axis off
%    title('Stage 1:Gaussian Blurring');
%    saveas(FigH,append('outputs\gaussianBlurring.png'))
%    close(FigH)
%end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Stage 2: Calculating Gradients (Without toolbox)
width = size(image_gray,2);
height = size(image_gray,1);

dx = single([ 0, 0,0;...
       1, 0,-1;...
       0, 0,0]);
dy = single([ 0, 1,0;...
       0, 0,0;...
       0,-1,0]);
Ix = conv2(image_blurred,dx,'same');  %Convolve across x direction of image
Iy = conv2(image_blurred,dy,'same');  %Convolve across y direction of image

if(debug == 1)
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

if fpga == 1
    dx = fliplr(dx);          % should do this for gaussian kernel as well
    dy = flipud(dy);
    sim("models\comboFilter\comboFilter.slx",1);
    gm = gm_mdl;
    gd = gd_mdl;
end

if(debug == 1)
    figure('Name','Stage 2a: Gradient Magnitue');
    imagesc(gm);
    colorbar;
    title('Stage 2a: Gradient Magnitue');
    thisdir = getOutputdir();
    %saveas(gca,[thisdir,'\','GradMag']);
    
    figure('Name','Stage 2b: Gradient Direction');
    imagesc(gd .* 180/pi);
    colorbar;
    title('Stage 2b: Gradient Direction');
    thisdir = getOutputdir();
    %saveas(gca,[thisdir,'\','GradDir']);
end

%FoundSegs = ravven_detect2(image_gray,gm,gd,debug,fpga);

lineTemp = zeros(1,6);
segments = []; %Array for holding segments
bramData = []; %Array for holding segme
north = single([ 5, 5, 5;
         -3, 0,-3;
         -3,-3,-3]);

northWest = single([ 5, 5,-3;
              5, 0,-3;
             -3,-3,-3]);

west = single([ 5,-3,-3;
         5, 0,-3;
         5,-3,-3]);

southWest = single([-3,-3,-3;
              5, 0,-3;
              5, 5,-3]);

dirMagNorth = abs(conv2(image_gray,north,'same'));
dirMagNorthWest = abs(conv2(image_gray,northWest,'same'));
dirMagWest = abs(conv2(image_gray,west,'same'));
dirMagSouthWest = abs(conv2(image_gray,southWest,'same'));

if fpga == 1
    north = fliplr(flipud(north));
    northWest = fliplr(flipud(northWest));
    west = fliplr(flipud(west));
    southWest = fliplr(flipud(southWest));
    sim("models\kirschFilter\kirschFilter.slx",1);

    dirMagNorth = dirMagNorth_mdl;
    dirMagNorthWest = dirMagNorthWest_mdl;
    dirMagWest = dirMagWest_mdl;
    dirMagSouthWest = dirMagSouthWest_mdl;    
end
for i = 1:1:4 
    %Convolve the gray image with each 3x3 filter
    if i == 1
        DirMag = dirMagNorth;
    elseif i == 2
        DirMag = dirMagNorthWest;
    elseif i == 3
        DirMag = dirMagWest;
    elseif i == 4
        DirMag = dirMagSouthWest;
    end    
    test = DirMag(:);
    test(test == 0) = [];
    avg = mean(test);     %Get the mean of the frame
    stdDev = std(test);   %Get the stddev of the frame
    MagThr = avg + stdDev *7; %Take the top %3 of the magnitudes
    MagThr1 = avg + stdDev *3; %Take the top %6 of the magnitudes

    BW1 = imbinarize(DirMag,MagThr);
    BW2 = imbinarize(gm,.04);
    BW3 = imbinarize(DirMag,MagThr1);
    BW = bitand(BW2, BW3);
    BW = bitor(BW,BW);
    L = bwlabeln(BW,4);
    
    if(debug == 1)
        figure
        imagesc(L);
        title('cca'); 
        hold on
    end
   
    numBlobs = max(max(L));
    for counter = 1:numBlobs
        [row,col] = find(L == counter);
        
        Lt = L';
        indicies = find(Lt(:) == counter);
        
        gmt = gm';
        gdt = gd';
        mags = gmt(indicies);
        thetas = gdt(indicies);
        
        
        % find 4 points
        % 1 - min x coordinate and its associated y
        % 2 - max x coordinate and its associated y
        % 3 - min y coordinate and its associated x
        % 4 - max y coordinate and its associated x
        
        % then compute the longest line and run with that.
        
        % find min x coord
        minXx = min(col);
        index = find(col == minXx);
        minXy = row(index(1));
        
        % find max x coord
        maxXx = max(col);
        index = find(col == maxXx);
        maxXy = row(index(1));
        
        % find min y coord
        minYy = min(row);
        index = find(row == minYy);
        minYx = col(index(1));
        
        % find max y coord
        maxYy = max(row);
        index = find(row == maxYy);
        maxYx = col(index(1));
        
        thetaLength = size(thetas);
        thetaLength = thetaLength(1);
        
        if (thetaLength >10)
            bramData = [bramData;[minXx,minXy,thetas(5)]]; 
            bramData = [bramData;[maxXx,maxXy,thetas(10)]]; 
            if(thetaLength > 15)
                tempTheta = thetas(15); 
            else
                tempTheta = 99;
            end
            bramData = [bramData;[minYx,minYy,tempTheta]];       
            if(thetaLength > 20)
                tempTheta = thetas(20); 
            else
                tempTheta = 99;
            end
            bramData = [bramData;[maxYx,maxYy,tempTheta]]; 
        end
        
        % find longest line
        length = [];
        length(1)  = Pt2PtDist(maxXx,maxXy,minXx,minXy);
        length(2)  = Pt2PtDist(maxXx,maxXy,minYx,minYy);
        length(3)  = Pt2PtDist(maxXx,maxXy,maxYx,maxYy);
        length(4)  = Pt2PtDist(maxYx,maxYy,minXx,minXy);
        length(5)  = Pt2PtDist(maxYx,maxYy,minYx,minYy);
        length(6)  = Pt2PtDist(minXx,minXy,minYx,minYy);
        maxLength = max(length);
        
        index = find(length == maxLength);
        index = index(1);
        
        switch index
            case 1
                lineTemp(1) = maxXx;
                lineTemp(2) = maxXy;
                lineTemp(3) = minXx;
                lineTemp(4) = minXy;
            case 2
                lineTemp(1) = maxXx;
                lineTemp(2) = maxXy;
                lineTemp(3) = minYx;
                lineTemp(4) = minYy;
            case 3
                lineTemp(1) = maxXx;
                lineTemp(2) = maxXy;
                lineTemp(3) = maxYx;
                lineTemp(4) = maxYy;
            case 4
                lineTemp(1) = maxYx;
                lineTemp(2) = maxYy;
                lineTemp(3) = minXx;
                lineTemp(4) = minXy;
            case 5
                lineTemp(1) = maxYx;
                lineTemp(2) = maxYy;
                lineTemp(3) = minYx;
                lineTemp(4) = minYy;
            case 6
                lineTemp(1) = minXx;
                lineTemp(2) = minXy;
                lineTemp(3) = minYx;
                lineTemp(4) = minYy;
        end
        
        hold on
        SegLength = Pt2PtDist(lineTemp(1),lineTemp(2),lineTemp(3),lineTemp(4));
        if((SegLength > 10) && (SegLength < 630))
            lineTemp(6) = SegLength; %Record Segment Length
            lineTemp = FindDirection(lineTemp,mags,thetas); %find the dir
            segments = [segments;lineTemp]; %Add to the good segments
            %plot([lineTemp(1),lineTemp(3)],[lineTemp(2),lineTemp(4)],'LineWidth',1,'Color','r');
        end
    end   
end
size(segments)
save bram.mat bramData
FoundSegs = segments;

if(debug == 1)
    figure('Name','Segments');
    imagesc(image_gray);
    title('Segments');
    hold on;
    %Debug Code
    sizeFoundSegs = size(FoundSegs);
    numSegs = sizeFoundSegs(1);
    for k = 1:numSegs
        LineColor = [146/255,abs(FoundSegs(k,5))/(4*pi),1];
        plot([FoundSegs(k,1),FoundSegs(k,3)],...
           [FoundSegs(k,2),FoundSegs(k,4)],...
           'LineWidth',2,'color',LineColor);%plot the segment
    end
    hold off;
        thisdir = getOutputdir();
    %saveas(gca,[thisdir,'\','Segments']);
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
    imagesc(image_gray);
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
    %saveas(gca,[thisdir,'\','DetectedQuads']);
end

%Stage 8: Decode Quads
Detections = DecodeQuad(quads,single(image_gray),0);

%Stage 9: Remove Duplicates
Detections = RemoveDuplicates(Detections);

if(debug == 1)
    %Debug visualization
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(inputImage);
    axis off
    title('Detected Tags');
    hold on;
    detectionsSize = size(Detections);
    numDetections = detectionsSize(1);
    for i = 1:numDetections
        plot(Detections(i).QuadPts(1:2,1),Detections(i).QuadPts(1:2,2),'g-','LineWidth',2);
        plot(Detections(i).QuadPts(2:3,1),Detections(i).QuadPts(2:3,2),'r-','LineWidth',2);
        plot(Detections(i).QuadPts(3:4,1),Detections(i).QuadPts(3:4,2),'m-','LineWidth',2);
        plot(Detections(i).QuadPts([4,1],1),Detections(i).QuadPts([4,1],2),'b-','LineWidth',2);
        scatter(Detections(i).cxy(1),Detections(i).cxy(2),100,'r','LineWidth',2);
        text(Detections(i).cxy(1)+10,Detections(i).cxy(2)+5,sprintf('#%i',Detections(i).id),'color','r');
    end
    hold off;
    %saveas(FigH,append('outputs\detectedTags.png'))
    %close(FigH)
end

%These are helper / utility functions
function GrayImage = cvtColor(InputImage)
RedConv   = single(InputImage(:,:,1) *  0.299);
GreenConv = single(InputImage(:,:,2) *  0.587);
BlueConv  = single(InputImage(:,:,3) *  0.114);

GrayImage = RedConv + GreenConv + BlueConv;
GrayImage = GrayImage / 255;
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


function distance = Pt2PtDist(P1x,P1y,P2x,P2y)
dx = P1x - P2x; %Change in X
dy = P1y - P2y; %Change in Y
distance = sqrt(dx^2 + dy^2); %Find the Euclidean distance 
end

function LineTemp = FindDirection(LineTemp,mags,thetas)
% x1 y1 x2 y2 dir length
dx = LineTemp(3) - LineTemp(1); %Find the change in x
dy = LineTemp(4) - LineTemp(2); %Find the change in y

tmpTheta = atan2(dy,dx); %'Assumed' direction of the line

%Variables for our votes
noflip = 0;
flip = 0;

% find the correct theta by waiting for 3 exact thetas in a row
% y = diff(thetas);
% goldenThetaIndex = find(y == 0);
% goldenThetaIndex = goldenThetaIndex(1);
%goldenTheta = mode(thetas);

%indices = find(mags > .06);
%goodThetas = thetas(indices);
%goldenTheta = mean(goodThetas);

goldenTheta = thetas(10);
%maxMag = max(mags);
%maxIndex = find(mags == maxMag);
%maxIndex = maxIndex(1);

%goldenTheta = thetas(maxIndex);

%figure
%subplot(2,1,1)
%plot(thetas)
%subplot(2,1,2)
%plot(mags)
err = single(mod2pi(goldenTheta - tmpTheta)); 
if (err > 0) %If it's flipped add PI
    LineTemp(5) = tmpTheta + pi;
else
    LineTemp(5) = tmpTheta;
end

% for i = 1:size(thetas)
%     Get all the thetas of the line
%     theta = thetas(i);
%     
%     Calculate the error of our assumed direction
%     err = single(mod2pi(theta - tmpTheta)); 
%     
%    if(err < 0) %If the error is negative vote for no flip
%        noflip = noflip + mags(i);
%    else           %If the error is positive vote for to flip
%        flip = flip + mags(i);
%    end
% end
% 
% if (flip > noflip) %If it's flipped add PI
%     LineTemp(5) = tmpTheta + pi;
%     disp 'flipped'
% else
%     LineTemp(5) = tmpTheta;
% end

avgTheta = goldenTheta;

avgThetaRotate = avgTheta + pi/2;

avgThetaRotateDeg = avgThetaRotate * 180/pi;
answerDeg = LineTemp(5) * 180/pi;

dyNew = sin(avgThetaRotate) * LineTemp(6);
dxNew = cos(avgThetaRotate) * LineTemp(6);

if (sign(dyNew) ~= sign(dy))
    dyNew = dyNew * -1;
end

if (sign(dxNew) ~= sign(dx))
    dxNew = dxNew * -1;
end

%Check if it's the right direction
dot = dx*cos(LineTemp(5)) + dy*sin(LineTemp(5)); 

if(dot > 0) %If not flip the line direction
    tmpX = LineTemp(1);
    LineTemp(1) = LineTemp(3);
    LineTemp(3) = tmpX;
    
    tmpY = LineTemp(2);
    LineTemp(2) = LineTemp(4);
    LineTemp(4) = tmpY;
    
    % center the new line
    x1 = LineTemp(1);
    y1 = LineTemp(2);
    x2 = LineTemp(3);
    y2 = LineTemp(4);

    yError = (dy - dyNew)/2;
    xError = (dx - dxNew)/2;

    %LineTemp(3) = LineTemp(1) - dxNew - xError;
    %LineTemp(4) = LineTemp(2) - dyNew - yError;

    %LineTemp(3) = LineTemp(1) - dxNew;
    %LineTemp(4) = LineTemp(2) - dyNew;
else
    % center the new line
    x1 = LineTemp(1);
    y1 = LineTemp(2);
    x2 = LineTemp(3);
    y2 = LineTemp(4);
    
    yError = (dy - dyNew)/2;
    xError = (dx - dxNew)/2;
    
    %LineTemp(3) = dxNew + LineTemp(1) - xError;
    %LineTemp(4) = dyNew + LineTemp(2) - yError;
    
    %LineTemp(3) = dxNew + LineTemp(1);
    %LineTemp(4) = dyNew + LineTemp(2);
end
end
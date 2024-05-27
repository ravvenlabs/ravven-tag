debug = 1;
fpga = 1;
inputImage = imread('input.bmp');
image_gray = single(rgb2gray(inputImage));

if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_gray);
    axis off
    title('Preprocessing: Grayscale');
    saveas(FigH,append('outputs\grayscale.png'))
    close(FigH)
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Stage 1: Gaussian Blurring (Without toolbox)
G = [0.0571, 0.1248 ,0.0571;
     0.1248, 0.2725, 0.1248;
     0.0571, 0.1248 ,0.0571];
image_blurred = conv2(image_gray,G,'same'); %Convolve across image

if fpga == 1
    sim("models\gaussianFilter\gaussianFilter.slx",1);
    image_blurred = image_blurred_mdl;
end

%Displaying the results of blurring
if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_blurred);
    axis off
    title('Stage 1:Gaussian Blurring');
    saveas(FigH,append('outputs\gaussianBlurring.png'))
    close(FigH)
end
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
    sim("models\magPhase\magPhase.slx",1);
    gm = gm_mdl;
    gd = gd_mdl;
end

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

FoundSegs = ravven_detect2(image_gray*256,gm,gd,debug);

if(debug == 1)
    figure('Name','Segments');
    imagesc(image_gray);
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
    saveas(gca,[thisdir,'\','DetectedQuads']);
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
    for i = 1:length(Detections)
        plot(Detections(i).QuadPts(1:2,1),Detections(i).QuadPts(1:2,2),'g-','LineWidth',2);
        plot(Detections(i).QuadPts(2:3,1),Detections(i).QuadPts(2:3,2),'r-','LineWidth',2);
        plot(Detections(i).QuadPts(3:4,1),Detections(i).QuadPts(3:4,2),'m-','LineWidth',2);
        plot(Detections(i).QuadPts([4,1],1),Detections(i).QuadPts([4,1],2),'b-','LineWidth',2);
        scatter(Detections(i).cxy(1),Detections(i).cxy(2),100,'r','LineWidth',2);
        text(Detections(i).cxy(1)+10,Detections(i).cxy(2)+5,sprintf('#%i',Detections(i).id),'color','r');
    end
    hold off;
    saveas(FigH,append('outputs\detectedTags.png'))
    close(FigH)
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
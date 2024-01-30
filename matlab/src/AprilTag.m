function [Detections] = AprilTag(image,alg,debug,inputNum)
if(nargin < 3)
    debug = 0;
end

if(nargin < 2)
    alg = 3;
end

%Preprocessing to Grayscale
switch alg
    case {1, 2, 3, 4}
        if(ndims(image) > 2)
            image_gray = cvtColor(image);
        else
            image_gray = single(image);
        end
    case {5, 6}
        image_gray = uint8(readmatrix(sprintf("../pics/data/gray%d.csv", inputNum)));
        % Temp Norm
        image_gray = single(image_gray) ./ 255;
end

if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_gray);
    axis off
    title('Preprocessing: Grayscale');
    saveas(FigH,append('images\grayscale.png'))
    close(FigH)
end

%Stage 1: Gaussian Blurring (Without toolbox)
% G = fspecial('gaussian',3,0.8); %Generate Gausian Filter
G = [0.0571, 0.1248 ,0.0571;
     0.1248, 0.2725, 0.1248;
     0.0571, 0.1248 ,0.0571];
image_blurred = conv2(image_gray,G,'same'); %Convolve across image

%Displaying the results of blurring
if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_blurred);
    axis off
    title('Stage 1:Gaussian Blurring');
    saveas(FigH,append('images\gaussianBlurring.png'))
    close(FigH)
end

switch alg
    case 1
		quads = quad_gradient(image_blurred,image_gray,debug);
    case 2
		quads = quad_thresh(image_blurred,image_gray,debug);
    case 3
		quads = quad_ravven(image_blurred,image_gray,debug);
    case 4
		quads = quad_ravven2(image_blurred,image_gray,debug);
    case 5
		quads = quad_ravven3(image_gray,debug,inputNum);
    case 6
		quads = quad_ravven4(image_gray,debug,inputNum);
    otherwise
        disp('Please select a valid option');
end

%Stage 8: Decode Quads
Detections = DecodeQuad(quads,single(image_gray),0);

%Stage 9: Remove Duplicates
Detections = RemoveDuplicates(Detections);

if(debug == 1)
    %Debug visualization
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image);
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
    saveas(FigH,append('images\detectedTags.png'))
    close(FigH)
end

end

%These are helper / utility functions

function GrayImage = cvtColor(InputImage)
RedConv   = single(InputImage(:,:,1) *  0.299);
GreenConv = single(InputImage(:,:,2) *  0.587);
BlueConv  = single(InputImage(:,:,3) *  0.114);

GrayImage = RedConv + GreenConv + BlueConv;
GrayImage = GrayImage / 255;
end

function dir = getOutputdir
global outputdir
dir = outputdir;
end
debug = 1;
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

%Stage 1: Gaussian Blurring (Without toolbox)
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
    saveas(FigH,append('outputs\gaussianBlurring.png'))
    close(FigH)
end

quads = quad_ravven2(image_blurred,image_gray,debug);

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
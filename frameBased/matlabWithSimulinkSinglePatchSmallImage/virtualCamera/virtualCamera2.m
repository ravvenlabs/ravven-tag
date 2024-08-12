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

% function Result = frameBasedGaussianFilter(image_gray, G)
%     Result = hdl.npufun(@gaussianKernel, [3 3], ImageIn,'NonSampleInput', data); 
% end
% 
% function e = gaussianKernel(in,data)
%     temp = in.*data;
%     e = sum(sum(temp));
% end

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
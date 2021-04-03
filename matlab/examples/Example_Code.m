clc;
clear;
close all;

addpath(genpath('../src'))
addpath(genpath('../Examples'))

DEBUG = 1;

user_input_prompt = 'Which algorithm do you want to use?\n1.April Tag 1\n2.April Tag 2\n3.Ravven Detect\n4.Ravven 2 Detect\n5.Ravven 2 Mag/Theta\n6.Ravven 2 FPGA\n';
UserAlg = input(user_input_prompt);

user_input_prompt = 'Which example do you want to run?\n1.Webcam\n2.Test Image\n3.Single Image\n4.Analyze Video\n5.Run Input Test Suite\n';
usr_input = input(user_input_prompt);

setupOutputDir([pwd,'\outputdir']);

inputFileNumbers = 1:20;

switch usr_input
    case 1
        WebcamDemo;
    case 2
        profile on;
        [Detection] = AprilTag(imread('../pics/test_tag.png'),UserAlg,DEBUG);
        Detection
        profile viewer;
    case 3
        [file, path] = uigetfile('../pics/data/*');
        if isequal(file,0)
            disp('User selected cancel')
        else
            profile on;
            imageNum = -1;
            switch UserAlg
                case {5, 6}
                    imageNum = input("Input the number associated with the image and csvs\n");
                otherwise
                    imageNum = -1;
            end
            [Detection] = AprilTag(imread([path,file]),UserAlg,DEBUG,imageNum);
            Detection
            profile viewer;
        end
    case 4
        [file, path] = uigetfile('../pics/test.mp4');
        if isequal(file,0)
            disp('User selected cancel');
        else
            video  = VideoReader([path,file]);
            videoWidth = video.Width;
            videoHeight = video.Height;
            
            output = struct('cdata',zeros(videoHeight,videoWidth,3,'uint8'),'colormap',[]);
            mkdir vidframes;
            vidDisp = figure;
            fpsPlot = figure;
            FpsBuffer = zeros(1,10);
            k = 1;
            FpsPlot = [];
            
            while hasFrame(video)
                CurrFrame = readFrame(video);
                
                tic;
                [det] = AprilTag(CurrFrame,UserAlg);
                FrameTime = toc;
                
                FpsBuffer(2:10) = FpsBuffer(1:9);
                FpsBuffer(1) = (FrameTime)^-1;
                AvgFps = sum(FpsBuffer)/10;
                FpsPlot = [FpsPlot,FpsBuffer(1)];
                
                figure(fpsPlot);
                plot(FpsPlot);
                
                figure(vidDisp); %Get Figure for displaying video
                imshow(CurrFrame);   %Display Current Frame
                hold on;          %Wait to draw AprilTag detections
                for i = 1:size(det)
                    plot(det(i).QuadPts(1:2,1),det(i).QuadPts(1:2,2),'g-','LineWidth',2);
                    plot(det(i).QuadPts(2:3,1),det(i).QuadPts(2:3,2),'r-','LineWidth',2);
                    plot(det(i).QuadPts(3:4,1),det(i).QuadPts(3:4,2),'m-','LineWidth',2);
                    plot(det(i).QuadPts([4,1],1),det(i).QuadPts([4,1],2),'b-','LineWidth',2);
                    scatter(det(i).cxy(1),det(i).cxy(2),100,'r','LineWidth',2);
                    text(det(i).cxy(1)+10,det(i).cxy(2)+5,sprintf('#%i',det(i).id),'color','r');
                end
                text(videoWidth-(0.15*videoWidth),videoHeight-(0.03*videoHeight),sprintf('%0.1f fps',AvgFps),'color','r');
                hold off;       %Release figure
                
%                 saveas(vidDisp,'test.png');
                annotatedFrame = getframe;
                
                output(k).cdata = annotatedFrame.cdata;
%                 imwrite(CurrFrame,sprintf('vidframes/%05i.png',k));
                k = k + 1;
            end
            
            save fps.mat FpsPlot;
            vout = VideoWriter('test','MPEG-4');
            open(vout);
            writeVideo(vout,output);
            close(vout);
        end
    case 5
        for i = inputFileNumbers
            file = sprintf("../pics/data/input%d.bmp", i)
            detection = AprilTag(imread(file),UserAlg,0,i);
            found(i).file = file;
            found(i).detection = detection;
        end
    otherwise
        disp('Please select a valid option');
end

function setupOutputDir(UsrInput)
global outputdir;
outputdir = UsrInput;
end
    
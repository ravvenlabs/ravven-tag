clc;
clear;
close all;
if(exist('../pics/R','dir') == 0)
    unzip('../pics/TestData.zip','../pics');
end

CData = csvread('../data/TestOutput.csv');

NumOfPics = 61;

Obs_time  = zeros(NumOfPics*3,1);
Obs_points = zeros(NumOfPics*3,10);
Obs_pose   = zeros(NumOfPics*3,10);

LocAddr = 1;

Path = [-30:30]';
TruePitch = [zeros(NumOfPics,1),zeros(NumOfPics,1), Path];

PitchPics = [];
for i = 1:NumOfPics
    PitchPics = [PitchPics;sprintf('../pics/P/%05d.jpg',i)];
end

for j = 1:size(PitchPics,1)
   CurrentPic = imread(PitchPics(j,:));
   [RT_time(LocAddr),RT_points(LocAddr,:)] = TestAlg(3,CurrentPic);
   LocAddr = LocAddr + 1;
end


RollPics = [];
for i = 1:NumOfPics
    RollPics = [RollPics;sprintf('../pics/R/%05d.jpg',i)];
end

for j = 1:size(RollPics,1)
   CurrentPic = imread(RollPics(j,:));
   [RT_time(LocAddr),RT_points(LocAddr,:)] = TestAlg(3,CurrentPic);
   LocAddr = LocAddr + 1;
end

YawPics = [];
for i = 1:NumOfPics
    YawPics = [YawPics;sprintf('../pics/Y/%05d.jpg',i)];
end

for j = 1:size(YawPics,1)
   CurrentPic = imread(YawPics(j,:));
   [RT_time(LocAddr),RT_points(LocAddr,:)] = TestAlg(3,CurrentPic);
   LocAddr = LocAddr + 1;
end

AT2CvsAT1M = abs(CData - AT1_points);
AT2CvsAT2M = abs(CData - AT2_points);
AT2CvsRTM =  (CData - RT_points);

%Ravven detect vs April Tag 2 M code
figure;
scatter(AT2CvsRTM(1,:),AT2CvsRTM(2,:));
axis([-1 1 -1 1]);
title('RavvenDetect.m vs AprilTag2.c');
xlabel('{\Delta} X (pixels)');
ylabel('{\Delta} Y (pixels)');



figure;
%Pitch Output
plotYPR(0,RT_points(:).Pose, 0)

%Roll Output
plotYPR(1,RT_points.Pose, 0)

%Yaw Disp
plotYPR(2,RT_points.Pose, 0)

figure;
%Pitch Diff
plotYPR(0,RT_points.Pose, 1)

%Roll Diff
plotYPR(1,RT_points.Pose, 1)

%Yaw Diff
plotYPR(2,RT_points.Pose, 1)

function [Time,Output,Pose] = TestAlg(AlgSelect,Image)
   StartTime = tic;
   [Detection] = AprilTag(Image,AlgSelect);
   ElapsedTime = toc(StartTime);
   if(~isempty(Detection))
        Output = [Detection(1).cxy(1),Detection(1).cxy(2),...
        Detection(1).QuadPts(1,:),Detection(1).QuadPts(2,:),Detection(1).QuadPts(3,:),Detection(1).QuadPts(4,:)];
        Pose = Detection.Pose;
   else
       Output = zeros(1,10);
       Pose = [];
   end
       
   Time = ElapsedTime;
end

function plotYPR(RowNum,MatData,diff)
switch RowNum
case 0
    PlotTitle = 'Pitch Test:';
case 1
    PlotTitle = 'Roll Test:';
case 2
    PlotTitle = 'Yaw Test:';
end
    
if(diff ~= 1)
    subplot(3,3,1+RowNum);
    axis([-30 30 -40 40]);
    if(RowNum == 1)
        line([-30,30],[-30,30],'Color','green')
    else
        line([-30,30],[0,0],'Color','green')
    end
    hold on;
%     plot([-30:30],CData(:,6)*(180/pi),'-r');
    plot([-30:30],[MatData(:).pitch]','-b');
    
    title([PlotTitle,'Pitch']);
    legend('Unity','C++','Matlab','location','southeast');
    xlabel('True Rotation (Degrees)') % x-axis label
    ylabel('degrees') % y-axis label
    hold off;
    
    subplot(3,3,4+RowNum);
    axis([-30 30 -40 40]);
    if(RowNum == 0)
        line([-30,30],[-30,30],'Color','green')
    else
        line([-30,30],[0,0],'Color','green')
    end
    hold on;
%     plot([-30:30],CData(:,7)*(180/pi),'-r');
    plot([-30:30],[MatData(:).roll]','-b');
    
    title([PlotTitle,'Roll']);
    legend('Unity','C++','Matlab','location','southeast');
    xlabel('True Rotation (Degrees)') % x-axis label
    ylabel('degrees') % y-axis label
    hold off;

    subplot(3,3,7+RowNum);
    axis([-30 30 -40 40]);
    if(RowNum == 2)
        line([-30,30],[-30,30],'Color','green')
    else
        line([-30,30],[0,0],'Color','green')
    end
    hold on;
%     plot([-30:30],CData(:,5)*(180/pi),'-r');
    plot([-30:30],[MatData(:).yaw]','-b');
    title([PlotTitle,'Yaw']);
    legend('Unity','C++','Matlab','location','southeast');
    xlabel('True Rotation (Degrees)') % x-axis label
    ylabel('degrees') % y-axis label
    hold off;
else
    Path = [-30:30];
    
    subplot(3,3,1+RowNum);
    if(RowNum == 1)
%         plot([-30:30],((CData(:,6)*(180/pi)) - Path(:)),'-r');
        hold on;
        plot([-30:30],([MatData(:).pitch]' - Path(:)),'-b');
    else
%         plot([-30:30],(CData(:,6)*(180/pi)),'-r');
        hold on;
        plot([-30:30],([MatData(:).pitch]'),'-b');
    end

    %axis([-30 30 -10 10]);
    title([PlotTitle,'Pitch Diff']);
    legend('C++','Matlab');
    xlabel('True Rotation (Degrees)') % x-axis label
    ylabel('degrees') % y-axis label
    hold off;
    
    subplot(3,3,4+RowNum);
    if(RowNum == 0)
%         plot([-30:30],(CData(:,7)*(180/pi) - Path(:)),'-r');
        hold on;
        plot([-30:30],([MatData(:).roll]' - Path(:)),'-b');
    else
%         plot([-30:30],(CData(:,7)*(180/pi)),'-r');
        hold on;
        plot([-30:30],([MatData(:).roll]'),'-b');
    end
    %axis([-30 30 -10 10]);
    title([PlotTitle,'Roll Diff']);
    legend('C++','Matlab');
    xlabel('True Rotation (Degrees)') % x-axis label
    ylabel('degrees') % y-axis label
    hold off;

    subplot(3,3,7+RowNum);
    if(RowNum == 2)
%         plot([-30:30],(CData(:,5)*(180/pi) - Path(:)),'-r');
        hold on;
        plot([-30:30],([MatData(:).yaw]' - Path(:)),'-b');
    else
%         plot([-30:30],(CData(:,5)*(180/pi)),'-r');
        hold on;
        plot([-30:30],([MatData(:).yaw]'),'-b');
    end
    %axis([-30 30 -10 10]);
    title([PlotTitle,'Yaw Diff']);
    legend('C++','Matlab');
    xlabel('True Rotation (Degrees)') % x-axis label
    ylabel('degrees') % y-axis label
    hold off;
end
end

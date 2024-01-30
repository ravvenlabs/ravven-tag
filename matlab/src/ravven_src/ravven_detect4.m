function Clusters = ravven_detect4(im_gray,gm,gd,Debug,imNum)
    %Get all the filters
    Compass_Filters = KirschFilters();
    lineTemp = zeros(1,6);
    bram = []; %Array for holding maxX, maxY, minX, minY points from CCA
    segments = []; %Array for holding segments

    for i = 1:1:4
        %Convolve the gray image with each 3x3 filter
        DirMag = abs(conv2(im_gray,Compass_Filters(:,:,i),'same'));
        %test = reshape(DirMag, width*height,1); %Reshape to a list to take stats
        test = DirMag(:);
        test(test == 0) = [];
        avg = mean(test);     %Get the mean of the frame
        stdDev = std(test);   %Get the stddev of the frame
        MagThr = avg + stdDev * 7;%1.89; %Take the top %3 of the magnitudes
        MagThr1 = avg + stdDev * 3;%1.56; %Take the top %6 of the magnitudes

        BW1 = imbinarize(DirMag,MagThr);
        BW2 = imbinarize(gm,.04);
        BW3 = imbinarize(DirMag,MagThr1);
        BW = bitand(BW2, BW3);
        BW = bitor(BW,BW1);
        
        [L,minX,maxX,minY,maxY] = dskFeatureDetectorFunction(BW,gd);
        
        if(Debug == 1)
            %figure
            %imagesc(L);
            %title('cca'); 
            %hold on

            FigH = figure('Position', get(0, 'Screensize'));
            imagesc(L);
            axis off
            title(['CCA' num2str(i)]); 
            saveas(FigH,append(['images\cca' num2str(i) '.png']))
            close(FigH)
        end
        
        temp = size(maxX);
        temp = temp(1);
        for i = 1:temp    
            % find a legit blob
            if (maxX(i,4) == 1)
                % we have a valid blob
            
                minXx = minX(i,1);
                minXy = minX(i,2);
                maxXx = maxX(i,1);
                maxXy = maxX(i,2);
                minYx = minY(i,1);
                minYy = minY(i,2);
                maxYx = maxY(i,1);
                maxYy = maxY(i,2);
                            
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
                lineTemp(5) = maxX(i,3);       % theta
            
                SegLength = Pt2PtDist(lineTemp(1),lineTemp(2),lineTemp(3),lineTemp(4));
                if((SegLength > 5) && (SegLength < 630))
                    bram = [bram; lineTemp(1:5)];
                    lineTemp(6) = SegLength; %Record Segment Length
                    lineTemp = FindDirection(lineTemp); %find the dir
                    segments = [segments;lineTemp]; %Add to the good segments
                end
            end
        end
    end
    writematrix(bram, sprintf("../pics/data/bram%d.csv", imNum));
    Clusters = segments;
end

function filters = KirschFilters()
filters = zeros(3,3,8);
%North
filters(:,:,1) = [ 5, 5, 5;
                  -3, 0,-3;
                  -3,-3,-3];
                  
%North-West
filters(:,:,2) = [ 5, 5,-3;
                   5, 0,-3;
                  -3,-3,-3];

%West
filters(:,:,3) = [ 5,-3,-3;
                   5, 0,-3;
                   5,-3,-3];
%South-West         
filters(:,:,4) = [-3,-3,-3;
                   5, 0,-3;
                   5, 5,-3];
%South                  
filters(:,:,5) = [-3,-3,-3;
                  -3, 0,-3;
                   5, 5, 5];
%South-East
filters(:,:,6) = [-3,-3,-3;
                  -3, 0, 5;
                  -3, 5, 5];
%East
filters(:,:,7) = [-3,-3, 5;
                  -3, 0, 5;
                  -3,-3, 5];
%North-East
filters(:,:,8) = [-3, 5, 5;
                  -3, 0, 5;
                  -3,-3,-3];
end

function distance = Pt2PtDist(P1x,P1y,P2x,P2y)
dx = P1x - P2x; %Change in X
dy = P1y - P2y; %Change in Y
distance = sqrt(dx^2 + dy^2); %Find the Euclidean distance 
end

function LineTemp = FindDirection(LineTemp)
% x1 y1 x2 y2 dir length
dx = LineTemp(3) - LineTemp(1); %Find the change in x
dy = LineTemp(4) - LineTemp(2); %Find the change in y

tmpTheta = atan2(dy,dx); %'Assumed' direction of the line

goldenTheta = LineTemp(5);
err = single(mod2pi(goldenTheta - tmpTheta)); 
if (err > 0) %If it's flipped add PI
    LineTemp(5) = tmpTheta + pi;
else
    LineTemp(5) = tmpTheta;
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
end
end
function Clusters = ravven_detect2(im_gray,gm,gd,Debug)
    %Get all the filters
    Compass_Filters = KirschFilters();
    lineTemp = zeros(1,6);
    segments = []; %Array for holding segments
    bramData = []; %Array for holding segments
    for i = 1:1:4
        %Do an inital convolution to get stats about the camera 
        %(Can be done realtime)
        
        %Convolve the gray image with each 3x3 filter
        DirMag = abs(conv2(im_gray,Compass_Filters(:,:,i),'same'));
        %test = reshape(DirMag, width*height,1); %Reshape to a list to take stats
        test = DirMag(:);
        test(test == 0) = [];
        avg = mean(test);     %Get the mean of the frame
        stdDev = std(test);   %Get the stddev of the frame
        MagThr = avg + stdDev *7; %Take the top %3 of the magnitudes
        MagThr1 = avg + stdDev *3; %Take the top %6 of the magnitudes
        %if(Debug == 1)
        %    figure;
        %    imagesc(DirMag);
        %    title('initial');
        %end
    
        BW1 = imbinarize(DirMag,MagThr);
        BW2 = imbinarize(gm,.04);
        %BW2 = imbinarize(gm,.3);
        BW3 = imbinarize(DirMag,MagThr1);
        BW = bitand(BW2, BW3);
        BW = bitor(BW,BW1);
        L = bwlabeln(BW,4);
        
        if(Debug == 1)
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
function Clusters = ravven_detect(im_gray,gm,width,height,Debug)
%Get all the filters
Compass_Filters = KirschFilters();

%Inital pre-allocate for edges
Groups = zeros(100000,4);
GroupAddr = 1;
GroupID = 1;

%Preallocating buffers
LB_Groups = zeros(2,width);
% Output = zeros(height,width);
Con1 = 0;
Con2 = 0;
Con3 = 0;
Con4 = 0;
GroupNum = 0;

if(Debug == 1)
    Output = zeros(height,width);
end

for i = 1:1:4
    %Do an inital convolution to get stats about the camera 
    %(Can be done realtime)
    
    %Convolve the gray image with each 3x3 filter
    DirMag = abs(conv2(im_gray,Compass_Filters(:,:,i),'same'));
    test = reshape(DirMag, width*height,1); %Reshape to a list to take stats
    test(test == 0) = [];
    avg = mean(test);     %Get the mean of the frame
    stdDev = std(test);   %Get the stddev of the frame
    MagThr = avg + stdDev *7; %Take the top %3 of the magnitudes
    MagThr1 = avg + stdDev *3; %Take the top %6 of the magnitudes
    if(Debug == 1)
        figure;
        imagesc(DirMag);
    end
%     DirMag(MagThr > DirMag) = 0; %Set everything less than the average to zero
    CurrentRow = zeros(1,width);
    for y = 5:height-5 %Loop through columns
        CurrentRow(1,:) = DirMag(y,:);
        for x = 5:width-5 %Loop through rows
            
            if(CurrentRow(x) > MagThr || (gm(y,x) > 0.04 && CurrentRow(x) > MagThr1) ) %Check if this pixel is over
               
               if(LB_Groups(1,x-1) ~= 0) %Left Connection
                   if(GroupNum == 0 || GroupNum > LB_Groups(1,x-1))
                       GroupNum = LB_Groups(1,x-1);
                   end
                   Con1 = 1; 
               end

               
               if(LB_Groups(2,x-1) ~= 0) %Up-Left Connection
                   if(GroupNum == 0 || GroupNum > LB_Groups(2,x-1))
                       GroupNum = LB_Groups(2,x-1);
                   end
                   Con2 = 1;
               end

               
               if(LB_Groups(2,x) ~= 0)%Up Connection
                   if(GroupNum == 0 || GroupNum > LB_Groups(2,x))
                       GroupNum = LB_Groups(2,x);
                   end
                    Con3 = 1;
               end

               
               if(LB_Groups(2,x+1) ~= 0) %Up-Right Connection
                   if(GroupNum == 0 || GroupNum > LB_Groups(2,x+1))
                       GroupNum = LB_Groups(2,x+1);
                   end
                   Con4 = 1;
               end

                if(GroupNum == 0) %If no group # is found get a new one
                    LB_Groups(1,x) = GroupID;
                    if(Con1 == 1)
                        LB_Groups(1,x-1) = GroupID;
                    end
                    if(Con2 == 1)
                        LB_Groups(2,x-1) = GroupID;
                    end
                    if(Con3 == 1)
                        LB_Groups(2,x) = GroupID;
                    end
                    if(Con4 == 1)
                        LB_Groups(2,x+1) = GroupID;
                    end
                    GroupID = GroupID + 1; %Increment Group ID
                else
                    LB_Groups(1,x) = GroupNum;
                    if(Con1 == 1)
                        if(LB_Groups(1,x-1) ~= 0)
                            LB_Groups(LB_Groups(1,x-1) == LB_Groups) = GroupNum;
                        else
                            LB_Groups(1,x-1) = GroupNum;
                        end
                    end
                    if(Con2 == 1)
                        if(LB_Groups(2,x-1) ~= 0)
                            LB_Groups(LB_Groups(2,x-1) == LB_Groups) = GroupNum;
                        else
                            LB_Groups(2,x-1) = GroupNum;
                        end
                    end
                    if(Con3 == 1)
                        if(LB_Groups(2,x) ~= 0)
                            LB_Groups(LB_Groups(2,x) == LB_Groups) = GroupNum;
                        else
                            LB_Groups(2,x) = GroupNum;
                        end
                    end
                    if(Con4 == 1)
                        if(LB_Groups(2,x+1) ~= 0)
                            LB_Groups(LB_Groups(2,x+1) == LB_Groups) = GroupNum;
                        else
                            LB_Groups(2,x+1) = GroupNum;
                        end
                    end
                end
            end
            Con1 = 0;
            Con2 = 0;
            Con3 = 0;
            Con4 = 0;
            GroupNum = 0;
            
            if(LB_Groups(2,x) ~= 0)
                Groups(GroupAddr,:) = [x,y-2,gm(y-2,x),LB_Groups(2,x)];
                GroupAddr = GroupAddr + 1;
             end
            
            
        end
        if(Debug == 1)
            Output(y-2,:) = LB_Groups(2,:);
        end
        
        LB_Groups(2,:) = LB_Groups(1,:); %Fifo Shift
        LB_Groups(1,:) = zeros(1,width); %Clear out the last line
    end
    if(Debug == 1)
        figure;
        imagesc(Output);
    end
end
Clusters = sortrows(Groups,4); %Sorted for next step in alg
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
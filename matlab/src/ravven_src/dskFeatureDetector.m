load BW.mat

% dataIn = [0 0 0 0 0 1 1;
%           0 0 1 1 1 1 1;
%           1 1 1 0 1 0 0];

dataIn = BW;
 
temp = size(dataIn);
width = temp(2);
height = temp(1);
 
mergerLookup = zeros(1000);

% intil the merger table
for i = 1:length(mergerLookup)
    mergerLookup(i) = i;
end

minX = zeros(100,3);
maxX = zeros(100,3);
minY = zeros(100,3);
maxY = zeros(100,3);

dataOut = zeros(1,width*height);

blobID = 0;

dataInT = dataIn';
dataIn = reshape(dataInT,1,width*height);

buffer = zeros(1,width-1);

assignment = 0;
above = 0;
left = 0;

counter = 1;
for y = 1:height
    for x = 1:width
        pixel = dataIn(counter);

        % start of row set left to 0
        if (x == 1)
            left = 0;
        end

        if (pixel == 0)
            assignment = 0;  
        else
            if ((left ~= 0) && (above ~= 0))
                % merger
                if (left < above)
                    assignmet = left;
                    mergerLookup(above) = left;
                    
                    maxX(left,:) = maxX(above,:);
                    
                    minX(above,3) = 0;
                    maxX(above,3) = 0;
                    minY(above,3) = 0;
                    maxY(above,3) = 0;
                elseif (above < left)
                    assignment = above;
                    mergerLookup(left) = above;
                    
                    minX(above,:) = minX(left,:);
                    
                    minX(left,3) = 0;
                    maxX(left,3) = 0;
                    minY(left,3) = 0;
                    maxY(left,3) = 0;
                else
                    maxX(left,:) = maxX(above,:);
                end
            elseif ((left ~= 0) && (above == 0))
                % take left id
                assignment = left;
                
                % only update the maxX point
                if (x > maxX(left,1))
                    maxX(left,:) = [x,y,1];
                end
            elseif ((left == 0) && (above ~= 0))
                % take above id
                assignment = above;
                
                % only update the maxY point
                maxY(above,:) = [x,y,1];
            else
                % new ID
                blobID = blobID + 1;
                assignment = blobID;
                
                % load in all bram entries and mark as valid
                minX(blobID,:) = [x,y,1];
                maxX(blobID,:) = [x,y,1];
                minY(blobID,:) = [x,y,1];
                maxY(blobID,:) = [x,y,1];
            end
        end
        
        buffer = circshift(buffer,1);
        buffer(1) = left;
        left = assignment;
        aboveTemp = buffer(width-1);
        if (aboveTemp ~= 0)
            above = mergerLookup(aboveTemp);
        else
            above = 0;
        end
        dataOut(counter) = left;
        counter = counter + 1;
    end
end
dataOut = reshape(dataOut,width,height);
dataOut = dataOut';
figure
imagesc(dataOut)

bounding = dataOut;
%bounding = bounding .* 0;

counter = 0;
for i = 1:blobID
  if (minX(i,3) ~= 0)
    bounding(minX(i,2),minX(i,1)) = 2000;
    bounding(maxX(i,2),maxX(i,1)) = 2000;
    bounding(minY(i,2),minY(i,1)) = 2000;
    bounding(maxY(i,2),maxY(i,1)) = 2000;
    counter = counter + 1;
  end
end
figure
imagesc(bounding)
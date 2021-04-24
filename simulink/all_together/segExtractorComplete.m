%% Segment Extractor script for verification.
% Expects the raw data inside the 'frameOut'.
% Expects the input data inside the 'frameIn'.

disp("Running seg extractor");
    
% Add to end for number of frames
frameData = out.frameOut.data(:,:,end);
dim = size(frameData);

% Uncomment for multiple frames
%for frame = 1:dim(3)
    dataIndex = 1;
    segCount = 1;
    segmentsN = zeros(2048, 5);
    segmentsNW = zeros(2048, 5);
    segmentsW = zeros(2048, 5);
    segmentsSW = zeros(2048, 5);
    for i = 1:dim(1)
        for j = 1:dim(2)
            pixelData = uint64(frameData(i, j, end));
            % Ignore zero pixels that may have been appended
            if (pixelData == 0 && dataIndex == 1)
                continue;
            end

            segmentsN(segCount, dataIndex) = bitsra(bitand(pixelData, uint64(str2num('0xFFFF000000000000')), 'uint64'), 48);
            segmentsNW(segCount, dataIndex) = bitsra(bitand(pixelData, uint64(str2num('0x0000FFFF00000000')), 'uint64'), 32);
            segmentsW(segCount, dataIndex) = bitsra(bitand(pixelData, uint64(str2num('0x00000000FFFF0000')), 'uint64'), 16);
            segmentsSW(segCount, dataIndex) = bitand(pixelData, uint64(str2num('0x000000000000FFFF')), 'uint64');
            dataIndex = dataIndex + 1;
            if (dataIndex == 6)
                segCount = segCount + 1;
                dataIndex = 1;
            end
        end
    end
    
    foundSegments = [...
        segmentsN(1:find(any(segmentsN,2),1,'last'),:);...
        segmentsNW(1:find(any(segmentsNW,2),1,'last'),:);...
        segmentsW(1:find(any(segmentsW,2),1,'last'),:);...
        segmentsSW(1:find(any(segmentsSW,2),1,'last'),:)...
    ];
    foundSegmentsSize = size(foundSegments);
    disp("Found segs: " + num2str(foundSegmentsSize(1)));
    
    refFrame = out.frameIn.data(:,:,end);
    refFrame = circshift(refFrame, [2 2]);
    
    figure;
    imshow(refFrame);
    title('North Segments');
    hold on;
    for segC = 1:segCount
        plot([segmentsN(segC, 1) segmentsN(segC, 3)], [segmentsN(segC, 2) segmentsN(segC, 4)], 'LineWidth', 1, 'Color', 'r');
    end
    hold off;
    
    figure;
    imshow(refFrame);
    title('Northwest Segments');
    hold on;
    for segC = 1:segCount
        plot([segmentsNW(segC, 1) segmentsNW(segC, 3)], [segmentsNW(segC, 2) segmentsNW(segC, 4)], 'LineWidth', 1, 'Color', 'r');
    end
    
    figure;
    imshow(refFrame);
    title('West Segments');
    hold on;
    for segC = 1:segCount
        plot([segmentsW(segC, 1) segmentsW(segC, 3)], [segmentsW(segC, 2) segmentsW(segC, 4)], 'LineWidth', 1, 'Color', 'r');
    end
    
    figure;
    imshow(refFrame);
    title('Southwest Segments');
    hold on;
    for segC = 1:segCount
        plot([segmentsSW(segC, 1) segmentsSW(segC, 3)], [segmentsSW(segC, 2) segmentsSW(segC, 4)], 'LineWidth', 1, 'Color', 'r');
    end
    
    figure;
    imshow(refFrame);
    title('All Segments');
    hold on;
    for segC = 1:segCount
        plot([segmentsN(segC, 1) segmentsN(segC, 3)], [segmentsN(segC, 2) segmentsN(segC, 4)], 'LineWidth', 1, 'Color', 'r');
        plot([segmentsNW(segC, 1) segmentsNW(segC, 3)], [segmentsNW(segC, 2) segmentsNW(segC, 4)], 'LineWidth', 1, 'Color', 'b');
        plot([segmentsW(segC, 1) segmentsW(segC, 3)], [segmentsW(segC, 2) segmentsW(segC, 4)], 'LineWidth', 1, 'Color', 'g');
        plot([segmentsSW(segC, 1) segmentsSW(segC, 3)], [segmentsSW(segC, 2) segmentsSW(segC, 4)], 'LineWidth', 1, 'Color', 'k');
    end
%end

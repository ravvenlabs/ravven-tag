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
    segments = zeros(2048, 5);
    for i = 1:dim(1)
        for j = 1:dim(2)
            pixelData = frameData(i, j, end);
            % Ignore zero pixels that may have been appended
            if (pixelData == 0 && dataIndex == 1)
                continue;
            end

            segments(segCount, dataIndex) = pixelData;
            dataIndex = dataIndex + 1;
            if (dataIndex == 6)
                segCount = segCount + 1;
                dataIndex = 1;
            end
        end
    end
    
    disp("Found segs: " + num2str(segCount - 1));
    
    figure;
    imagesc(out.frameIn.data(:,:,end));
    hold on;
    for segC = 1:segCount
        plot([segments(segC, 1) segments(segC, 3)], [segments(segC, 2) segments(segC, 4)], 'LineWidth', 1, 'Color', 'r');
    end
%end

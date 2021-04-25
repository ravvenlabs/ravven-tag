%% Segment Extractor script for verification.
% Expects the raw data inside the 'frameOut'.
% Expects the input data inside the 'frameIn'.
function foundSegments = segExtractorComplete(frameIn, frameOut)
    disp("Running seg extractor");

    % Add to end for number of frames
    frameData = frameOut(:,:,end);
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

                segmentsN(segCount, dataIndex) = getSegmentData(pixelData, dataIndex, str2num('0xFFFF000000000000'), 48);
                segmentsNW(segCount, dataIndex) = getSegmentData(pixelData, dataIndex, str2num('0x0000FFFF00000000'), 32);
                segmentsW(segCount, dataIndex) = getSegmentData(pixelData, dataIndex, str2num('0x00000000FFFF0000'), 16);
                segmentsSW(segCount, dataIndex) = getSegmentData(pixelData, dataIndex, str2num('0x000000000000FFFF'), 0);
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

        refFrame = frameIn(:,:,end);
        % Shift to line up correctly due to filtering
        refFrame = circshift(refFrame, [2 2]);

        displaySegments(refFrame, 'North Segments', segmentsN)
        displaySegments(refFrame, 'Northwest Segments', segmentsNW)
        displaySegments(refFrame, 'West Segments', segmentsW)
        displaySegments(refFrame, 'Southwest Segments', segmentsSW)

        displaySegments(refFrame, 'All Segments', foundSegments)
    %end
end

function displaySegments(refFrame, frameTitle, segments)
    figure;
    imshow(refFrame);
    title(frameTitle);
    hold on;
    for segC = 1:size(segments,1)
        plot([segments(segC, 1) segments(segC, 3)], [segments(segC, 2) segments(segC, 4)], 'LineWidth', 1, 'Color', 'r');
    end
end

function parsedData = getSegmentData(pixelData, dataIndex, mask, shift)
    parsedData = bitsra(bitand(pixelData, uint64(mask), 'uint64'), shift);
    if (dataIndex == 5)
        parsedData = reinterpretcast(fi(parsedData, 0, 10, 0), numerictype(1, 10, 7));
    end
    parsedData = single(parsedData);
end

%% Main entry point for Simulink model end results analysis.
function detections = SoftwarePipeline(gray, raw_segments)
    % Cast
    raw_segments = single(raw_segments);
    % Step 7b
    dim = size(raw_segments);
    correctedSegments = zeros(dim(1), 6);
    for raw_seg = 1:dim(1)
        correctedSegment = SegmentCorrection(...
            raw_segments(raw_seg, 1), raw_segments(raw_seg, 2),...
            raw_segments(raw_seg, 3), raw_segments(raw_seg, 4),...
            raw_segments(raw_seg, 5),...
            Pt2PtDist(...
                raw_segments(raw_seg, 1), raw_segments(raw_seg, 2),...
                raw_segments(raw_seg, 3), raw_segments(raw_seg, 4))...
            );
        correctedSegments(raw_seg,:) = correctedSegment(:);
    end
    % Step 8
    connectedSegments = SegmentConnection(correctedSegments);
    % Step 9
    quads = CreateQuads(connectedSegments, correctedSegments);
    % Step 10
    detections = DecodeQuads(quads, single(gray));
    % Step 11
    detections = DuplicationRemoval(detections);
    % Step 12
    % PoseEstimation is already done...
end
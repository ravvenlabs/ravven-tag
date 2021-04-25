%% Main entry point for Simulink model end results analysis.
function detections = SoftwarePipeline(gray, raw_segments)
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
    detections = DecodeQuads(quads, single(gray), 0);
    % Step 11
    detections = DuplicationRemoval(detections);
    % Step 12
    % PoseEstimation is already done...
    % Show detected tag, if any
    if ~isempty(detections)
        figure('Name','Detected Tags');
        imshow(gray);
        title('Detected Tags');
        hold on;
        for i = 1:length(detections)
            plot(detections(i).QuadPts(1:2,1),detections(i).QuadPts(1:2,2),'g-','LineWidth',2);
            plot(detections(i).QuadPts(2:3,1),detections(i).QuadPts(2:3,2),'r-','LineWidth',2);
            plot(detections(i).QuadPts(3:4,1),detections(i).QuadPts(3:4,2),'m-','LineWidth',2);
            plot(detections(i).QuadPts([4,1],1),detections(i).QuadPts([4,1],2),'b-','LineWidth',2);
            scatter(detections(i).cxy(1),detections(i).cxy(2),100,'r','LineWidth',2);
            text(detections(i).cxy(1)+10,detections(i).cxy(2)+5,sprintf('#%i',detections(i).id),'color','r');
        end
        hold off;
    end
end
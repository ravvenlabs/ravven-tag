function connectedSegments = SegmentConnection(segments)
    if(isempty(segments))
        connectedSegments = [];
        return;
    end
    %Get all the first points of line segments
    StartPoints = [segments(:,1),segments(:,2)];

    %Get all the second points of line segments
    EndPoints   = [segments(:,3),segments(:,4)]; 

    %Find the nearest 20 start points to each end point using knn
    ValidPts = knnsearch(StartPoints,EndPoints,'K',10);

    connectedSegments = struct([]); %Allocating a struct to add segments to

    for i = 1:size(segments,1)
       %Checks if the segment has any successors
       tmp = CheckSuccessor(i,ValidPts(i,:)',segments);

        %Adds it to the list
       connectedSegments = [connectedSegments; tmp];
    end
end

function LinkedSegments = CheckSuccessor(Pos,PossibleIdx, SegmentList)
    LinkedSegments = struct('SegNum',Pos,'LSeg',[]); %struct to hold seg data

    GoodPts = PossibleIdx;

    ValidPts = [];
    for i = 1:size(GoodPts)
        %Checks that the next segment has the right winding
        if(mod2pi(SegmentList(GoodPts(i),5) - SegmentList(Pos,5)) < 0)
            ValidPts = [ValidPts,GoodPts(i)];
        end
    end

    if(isempty(ValidPts)) %Oops we don't have any successors
        return;
    end

    %Pair of x/y coordinates of Parent Line
    ParentLine = [SegmentList(Pos,1),SegmentList(Pos,2);...
        SegmentList(Pos,3), SegmentList(Pos,4)];

    %Pair of x/y coordinates of Child Line
    ChildLine =  SegmentList(ValidPts,1:4); 

    IntersectionPts = []; %Empty matrix

    for j = 1:size(ValidPts,2)
        %Temp variable to keep things simple
        CurrentChild = [ChildLine(j,1),ChildLine(j,2);...
            ChildLine(j,3),ChildLine(j,4)];

        %calculates the intersections
        tmp = DiffIntersect([ParentLine;CurrentChild]);

        %Adds that point to list of intersections
        IntersectionPts = [IntersectionPts;tmp];
    end

    %finds the distance between the intersection points and parent
    ParentDist = Pt2PtDist(ParentLine(2,1),ParentLine(2,2),...
        IntersectionPts(:,1),IntersectionPts(:,2));

    %finds the distance between the intersection points and child
    ChildDist  = Pt2PtDist(ChildLine(:,1),ChildLine(:,2),...
        IntersectionPts(:,1),IntersectionPts(:,2));

    for i = 1:size(ParentDist,1)
        %if the distance is less than the length of parent add to list
        if(max(ParentDist(i),ChildDist(i)) < SegmentList(Pos,6))
            LinkedSegments.LSeg = [LinkedSegments.LSeg, ValidPts(i)];
        end
    end

end
function distance = Pt2PtDist(P1x,P1y,P2x,P2y)
    dx = P1x - P2x; %Change in X
    dy = P1y - P2y; %Change in Y
    distance = sqrt(dx.^2 + dy.^2); %Find the Euclidean distance 
end
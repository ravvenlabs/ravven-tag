function segment = SegmentCorrection(x1, y1, x2, y2, theta, length)
    segment = zeros(1, 6);
    % x1 y1 x2 y2 dir length
    dx = x2 - x1; %Find the change in x
    dy = y2 - y1; %Find the change in y

    calcTheta = atan2(dy,dx); %'Assumed' direction of the line

    err = single(mod2pi(theta - calcTheta)); 
    if (err > 0) %If it's flipped add PI
        calcTheta = calcTheta + pi;
    end

    %Check if it's the right direction
    dot = dx*cos(calcTheta) + dy*sin(calcTheta); 

    if(dot > 0) %If not flip the line direction
        segment(1) = x2;
        segment(2) = y2;
        segment(3) = x1;
        segment(4) = y1;
    else
        segment(1) = x1;
        segment(2) = y1;
        segment(3) = x2;
        segment(4) = y2;
    end
    segment(5) = calcTheta;
    segment(6) = length;
end
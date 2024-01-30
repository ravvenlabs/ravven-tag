function quads = quad_ravven4(image_gray,debug,inputNum)
gm = single(reinterpretcast(uint16(readmatrix(sprintf("../pics/data/mag%d.csv", inputNum))), numerictype(0,16,15)));
gd = single(reinterpretcast(uint16(readmatrix(sprintf("../pics/data/theta%d.csv", inputNum))), numerictype(1,16,13)));

if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(gm);
    axis off
    title('Stage 2a: Gradient Magnitue');
    saveas(FigH,append('images\gradientMagnitude.png'))
    colorbar;
    close(FigH)

    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(gd .* 180/pi);
    axis off
    title('Stage 2b: Gradient Direction');
    saveas(FigH,append('images\gradientDirection.png'))
    colorbar;
    close(FigH)
end

FoundSegs = ravven_detect4(image_gray,gm,gd,debug,inputNum);

if(debug == 1)
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_gray);
    axis off
    title('Segments');
    hold on;

    %Debug Code
    for k = 1:length(FoundSegs)
        LineColor = [146/255,abs(FoundSegs(k,5))/(4*pi),1];
        plot([FoundSegs(k,1),FoundSegs(k,3)],...
           [FoundSegs(k,2),FoundSegs(k,4)],...
           'LineWidth',2,'color',LineColor);%plot the segment
    end
    hold off;
    saveas(FigH,append('images\segments.png'))
    close(FigH)

end
if(isempty(FoundSegs))
    quads = [];
    return;
end

%Stage 6: Chain Segments
linked_segments = LinkSegs(FoundSegs);
%Stage 7: Find Quads
quads = QuadDetection(linked_segments,FoundSegs);
if(debug == 1)
    %Debug visualization
    FigH = figure('Position', get(0, 'Screensize'));
    imagesc(image_gray);
    axis off
    title('Detected Quads with intersections');
    hold on;
    for inputNum = 1:size(quads,1)
        Seg1 = [quads(inputNum,1),quads(inputNum,3); quads(inputNum,2), quads(inputNum,4)];
        Seg2 = [quads(inputNum,3),quads(inputNum,5); quads(inputNum,4), quads(inputNum,6)];
        Seg3 = [quads(inputNum,5),quads(inputNum,7); quads(inputNum,6), quads(inputNum,8)];
        Seg4 = [quads(inputNum,7),quads(inputNum,1); quads(inputNum,8), quads(inputNum,2)];
        
        plot(Seg1(1,:),Seg1(2,:),'r-','LineWidth',2);
        plot(Seg2(1,:),Seg2(2,:),'r-','LineWidth',2);
        plot(Seg3(1,:),Seg3(2,:),'r-','LineWidth',2);
        plot(Seg4(1,:),Seg4(2,:),'r-','LineWidth',2);
        scatter([quads(inputNum,1),quads(inputNum,3),quads(inputNum,5),quads(inputNum,7)],...
            [quads(inputNum,2),quads(inputNum,4),quads(inputNum,6),quads(inputNum,8)],15,'go');
        scatter([sum(Seg1(1,:))/2,sum(Seg2(1,:))/2,sum(Seg3(1,:))/2,sum(Seg4(1,:))/2],...
            [sum(Seg1(2,:))/2,sum(Seg2(2,:))/2,sum(Seg3(2,:))/2,sum(Seg4(2,:))/2],15,'go');
    end
    saveas(FigH,append('images\detectedQuads.png'))
    close(FigH)
end
end

function dir = getOutputdir
global outputdir
dir = outputdir;
end
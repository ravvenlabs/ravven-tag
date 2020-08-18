function quads = quad_thresh(image_blurred,image_gray,debug,progressbar)
quads = [];
threshim = threshold(image_gray,5/255);
if(debug == 1)
    figure;
    imshow(threshim/255);
    thisdir = getOutputdir();
    saveas(gca,[thisdir,'\','SegmentOut']);
end
segments = segmentation(threshim,debug);
quads = FitQuads(segments,image_gray,debug);
end

function dir = getOutputdir
global outputdir
dir = outputdir;
end

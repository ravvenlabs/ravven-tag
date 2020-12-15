HOMEIMAGES = 'C:\zcaro\Databases\LabelMe\Images'; 
HOMEANNOTATIONS = 'C:\zcaro\Databases\LabelMe\Annotations';

system('git clone https://github.com/CSAILVision/LabelMeToolbox.git');

addpath(genpath('LabelMeToolbox'));
addpath(genpath('../src'))

LMinstall(HOMEIMAGES,HOMEANNOTATIONS,'flat');

imagefiles = dir([HOMEIMAGES,'*.jpg']);
info = zeros(size(imagefiles,1),2);

nbins = 200;
npics = size(imagefiles,1);
binsz = floor(npics/nbins);

for j = 37:nbins
parfor i = 1:binsz
    currentFile = fullfile(HOMEIMAGES,imagefiles(i+(j-1)*binsz).name);
    [~,det,stats] = AprilTag(imread(currentFile));
    info(i,:) = stats;
end
save(sprintf('%d_data',j),'info');
end


save('LabelMeData');
% system('shutdown -s');
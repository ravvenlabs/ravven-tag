% Dr. Kaputa
% Virtual Camera Demo
% must run matlabStereoServer.py first on the FPGA SoC

width = 752;
height = 480;

%Initialization Parameters
server_ip   = '192.168.2.17';     % IP address of the server
server_port = 9999;                % Server Port of the sever

client = tcpclient(server_ip,server_port);
fprintf(1,"Connected to server\n");

inputImage = imread('input.bmp');
image_gray = single(rgb2gray(inputImage));
sim("singleToUint8.slx",1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% send raw frames
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
for x = 1:20
write(client,'0');
flush(client);
%data = imread('input.bmp');
%data = uint8(data);

% mark the image number on the image
%data = insertText(data,[100 100],x);

%dataGray = im2gray(data);
imageStack = uint32(ones(height,width,2));
imageStack(:,:,1) = image_uint32;
imageStack(:,:,2) = image_uint32;
imageStack = permute(imageStack,[3 2 1]);
write(client,imageStack(:));
temp = read(client,1)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% receive processed frames
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
if x < 10
    % receive feedthrough frame
    write(client,'1');
    flush(client);
else
    % receive processed frame
    write(client,'2');
    flush(client); 
end
dataLeft = read(client,width*height);   
temp = reshape(dataLeft,[width,height]);
leftProcessed = permute(temp,[2 1]);

dataRight = read(client,width*height);
temp    = reshape(dataRight,[width,height]);
rightProcessed = permute(temp,[2 1]);

image_received = leftProcessed;
sim("uint8toSingle.slx",1);

pause(1)
end
t = tiledlayout(1,2, 'Padding', 'none', 'TileSpacing', 'compact'); 
t.TileSpacing = 'compact';
t.Padding = 'compact';
 
nexttile    
imagesc(data)
axis off
nexttile
imagesc(leftProcessed);
colormap gray
axis off
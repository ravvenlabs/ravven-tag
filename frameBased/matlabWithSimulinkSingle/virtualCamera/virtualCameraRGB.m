% Dr. Kaputa
% Virtual Camera Demo
% must run matlabStereoServer.py first on the FPGA SoC

% set this to what your resolution is
width = 1920;
height = 1080;

%Initialization Parameters
server_ip   = '192.168.2.15';     % IP address of the server
server_port = 9999;                % Server Port of the sever

client = tcpclient(server_ip,server_port,"Timeout",30);
fprintf(1,"Connected to server\n");

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% send raw frames
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   
for x = 1:20
write(client,'0');
flush(client);
data = imread('sailboat.jpg');

data = imresize(data,[height width]);
%imshow(data)
data = uint8(data);

% mark the image number on the image
data = insertText(data,[100 100],x,FontSize=42);

dataGray = im2gray(data);
imageStack = uint8(ones(height,width,8));
imageStack(:,:,1:3) = data;
imageStack(:,:,5:7) = data;
imageStack(:,:,4) = dataGray;
imageStack(:,:,8) = dataGray;
imageStack(1,1,:) = 0;
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
dataLeft = read(client,width*height*3);   
temp = reshape(dataLeft,[3,width,height]);
leftProcessed = permute(temp,[3 2 1]);

dataRight = read(client,width*height*3);
temp = reshape(dataRight,[3,width,height]);
rightProcessed = permute(temp,[3 2 1]);
imagesc(leftProcessed);
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
clear;
clc;
close all;

sim_time = 2014;
mdl = "../simulation/atan2_mag_cordic_simu_v4";
input_iX_file = "../hardware/tb/iX_data.txt";
input_iY_file = "../hardware/tb/iY_data.txt";
angle_hex_file = "../hardware/tb/angle_hex_tb_data.txt";
mag_hex_file = "../hardware/tb/mag_hex_tb_data.txt";

load_system(mdl);
simmed = sim(mdl);

iX = simmed.iX.data;
iY = simmed.iY.data;
angle_data = simmed.cordic_angle_mag.data(:, 1);
mag_data = simmed.cordic_angle_mag.data(:, 2);
valid = simmed.valid.data;

valid_angle_data = angle_data(valid.data == 1);
valid_mag_data = mag_data(valid.data == 1);

if length(valid_mag_data) ~= length(valid_angle_data)
    printf('Invalid testing data. Size mismatch!');
    return;
end

save_to_file(angle_hex_file, valid_angle_data);
save_to_file(mag_hex_file,  valid_mag_data);

save_to_file(input_iX_file, iX(1:2000));
save_to_file(input_iY_file, iY(1:2000));

function [] = save_to_file(filename, data)
    f = fopen(filename, 'w');
    for k = 1:length(data)
        fprintf(f, '%s\n', bin(data(k)));
    end
    fclose(f);
end

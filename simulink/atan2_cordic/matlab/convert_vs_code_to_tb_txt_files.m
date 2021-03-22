clear;
clc;
close all;

%% Takes the output files from the visual studio C code intermediate
% stages and outputs a text file that can be consumed by Vivado for
% testing.
%
% IN-PROGRESS...
%
% NOTE: This replaces any generated files.

visual_studio_output_path = "../../../visualStudio/images/output/cordic/";
hardware_tb_path = "../hardware/tb/";

input_vs_ix_csv = visual_studio_output_path + "step4-ix.txt";
input_vs_iy_csv = visual_studio_output_path + "step4-iy.txt";
input_vs_angle_csv = visual_studio_output_path + "step4-theta.txt";
input_vs_mag_csv = visual_studio_output_path + "step4-mag.txt";

output_ix_path = hardware_tb_path + "iX_data.txt";
output_iy_path = hardware_tb_path + "iY_data.txt";
angle_hex_file = hardware_tb_path + "angle_hex_tb_data.txt";
mag_hex_file = hardware_tb_path + "mag_hex_tb_data.txt";

vs_ix = sfi(reshape(readmatrix(input_vs_ix_csv), [], 1), 16, 13);
vs_iy = sfi(reshape(readmatrix(input_vs_iy_csv), [], 1), 16, 13);
vs_angle = sfi(reshape(readmatrix(input_vs_angle_csv), [], 1), 16, 13);
vs_mag = sfi(reshape(readmatrix(input_vs_mag_csv), [], 1), 16, 13);

valid = fi(ones(length(vs_ix), 1), 0, 1, 0);

save_to_file(output_ix_path, vs_ix);

function [] = save_to_file(filename, data)
    f = fopen(filename, 'w');
    for k = 1:length(data)
        fprintf(f, '%s\n', bin(data(k)));
    end
    fclose(f);
end

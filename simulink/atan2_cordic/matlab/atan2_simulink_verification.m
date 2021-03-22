clear;
clc;
close all;

sim_time = 2014;
mdl = "../simulation/atan2_mag_cordic_simu_v4";

load_system(mdl);
out = sim(mdl);

iX = out.iX.data;
iY = out.iY.data;

cordic_angle_data = double(out.cordic_angle_mag.data(:, 1));
cordic_mag_data = double(out.cordic_angle_mag.data(:, 2));
atan2_angle_data = out.atan2.data(:, 1);
atan2_mag_data = out.mag.data(:, 1);

magnitude_diff = abs(atan2_mag_data - cordic_mag_data) ./ atan2_mag_data;
angle_diff = abs(atan2_angle_data - cordic_angle_data);

magErr = sqrt(mean((atan2_mag_data - cordic_mag_data).^2));
angErr = sqrt(mean((atan2_angle_data(1001:2000) - cordic_angle_data(1001:2000)).^2));

plot(magnitude_diff);
title("Magnitude Differences");
xlabel("Time");
ylabel("Percent Error");
figure, plot(angle_diff(1001:2000));
title("Angle Differences (Trimmed)");
xlabel("Time");
ylabel("Differences");

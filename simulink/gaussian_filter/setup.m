close;
clear all;
clc;

evalin('base','pixelcontrolbus');

ROWS = 480;
COLS = 752;
Ts = 1;

cap_1 = fixdt(0,32,31);
cap_2 = fixdt(0,32,30);

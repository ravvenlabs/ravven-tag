close;
clear all;
clc;

evalin('base','pixelcontrolbus');

ROWS = 480;
COLS = 752;
Ts = 1;

NUM_OF_BITS = 32;
MSB = NUM_OF_BITS-1;
cap_1 = fixdt(0,NUM_OF_BITS,NUM_OF_BITS-1);
cap_2 = fixdt(0,NUM_OF_BITS,NUM_OF_BITS-2);

%% Copy the cordic lut below when using this algo (64 points)
%cordicLut = [1.57079632679490;0.785398163397448;0.463647609000806;0.244978663126864;0.124354994546761;0.0624188099959574;0.0312398334302683;0.0156237286204768;0.00781234106010111;0.00390623013196697;0.00195312251647882;0.000976562189559320;0.000488281211194898;0.000244140620149362;0.000122070311893670;6.10351561742088e-05;3.05175781155261e-05;1.52587890613158e-05;7.62939453110197e-06;3.81469726560650e-06;1.90734863281019e-06;9.53674316405961e-07;4.76837158203089e-07;2.38418579101558e-07;1.19209289550781e-07;5.96046447753906e-08;2.98023223876953e-08;1.49011611938477e-08;7.45058059692383e-09;3.72529029846191e-09;1.86264514923096e-09;9.31322574615479e-10;4.65661287307739e-10;2.32830643653870e-10;1.16415321826935e-10;5.82076609134674e-11;2.91038304567337e-11;1.45519152283669e-11;7.27595761418343e-12;3.63797880709171e-12;1.81898940354586e-12;9.09494701772928e-13;4.54747350886464e-13;2.27373675443232e-13;1.13686837721616e-13;5.68434188608080e-14;2.84217094304040e-14;1.42108547152020e-14;7.10542735760100e-15;3.55271367880050e-15;1.77635683940025e-15;8.88178419700125e-16;4.44089209850063e-16;2.22044604925031e-16;1.11022302462516e-16;5.55111512312578e-17;2.77555756156289e-17;1.38777878078145e-17;6.93889390390723e-18;3.46944695195361e-18;1.73472347597681e-18;8.67361737988404e-19;4.33680868994202e-19;2.16840434497101e-19];

%% Cordic algorithm utilizing floating point arithmetic
function [mag, angle] = atan2_mag_cordic(iy, ix, cordicLut)
    % Special cases
    if ix == 0 && iy == 0
        mag = 0;
        angle = 0;
        return;
    end

    % Approximation (1/A)
    K = 0.607253;
    % The next x value
    xo = iy*zsign(iy);
    % The next y value
    yo = ix*-zsign(iy);
    % The Atan
    zo = -zsign(iy) * cordicLut(1);
    z = zo + cordicLut(2)*-zsign(yo);
    zo = z;

    for iter = 3:12
        x = xo + bitsra(yo, iter-3) * zsign(yo);
        y = yo + bitsra(xo, iter-3) * -zsign(yo);
        z = zo + -zsign(y) * cordicLut(iter); % requires lookup table
        zo = z;
        yo = y;
        xo = x;
    end
    mag = x * K;
    angle = -z;
end

%% Helper function to treat 0 as positive
function [s] = zsign(v)
    if v < 0
        s = -1;
    else
        s = 1;
    end
end
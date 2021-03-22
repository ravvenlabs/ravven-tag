clear;
clc;
close all;

%% Quick script to locate good pairs of fixed bit lengths and sigmas when
% creating a gaussian filter that is seperable.

filter_dim = [5,5];
simgas = 0.01:0.01:0.99;
is = 8:1:24;

good_pairs = [];

for sigma = simgas
    Hg = fspecial('gaussian',filter_dim,sigma);

    [U,S,V]=svd(Hg);
    Hv=abs(U(:,1)*sqrt(S(1,1)));
    Hh=abs(V(:,1)'*sqrt(S(1,1)));

    for i = is
        Hgfi = fi(Hg,0,i);
        Hvfi = fi(Hv,0,i);
        Hhfi = fi(Hh,0,i);

        rankDoubleI = rank(double(Hgfi), 1/(2^(i+1)));
        if rankDoubleI ~= 1
            continue;
        end

        sumHgfi = sum( Hgfi(:) );
        sumHvfi = sum( Hvfi );
        sumHhfi = sum( Hhfi );

        if sumHgfi ~= 1 || sumHvfi ~= 1 || sumHhfi ~= 1
            continue;
        end

        Hcfi = fi(Hvfi * Hhfi,0,i,'fimath',fimath('RoundingMethod','Convergent'));
        equalTest = all( Hcfi(:)==Hgfi(:) );

        if equalTest
            pair.sigma = sigma;
            pair.bit_width = i;
            pair.Hg = Hgfi;
            pair.Hv = Hvfi;
            pair.Hh = Hhfi;
            good_pairs = [good_pairs; pair];
        end
    end
end

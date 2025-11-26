%% Plotter lookup-kurven med avstand på x-aksen
% og punkter kun på de originale kalibreringspunktene.

clc;
clear;
close all;

%% === Les lookup-tabellen ===
filnavn = 'lookup.csv';

if ~isfile(filnavn)
    error('Fant ikke fila "%s".', filnavn);
end

data = readtable(filnavn);

% Full lookup-tabell
avstand_mm  = data.distance_mm;
spenning_mV = data.mV;

%% === Fjern punkter > 1000 mm (stopper linja på siste målepunkt) ===
gyldig = avstand_mm <= 1000;
avstand_mm  = avstand_mm(gyldig);
spenning_mV = spenning_mV(gyldig);

%% === Originale kalibreringspunkter ===
kalibrering_avstand = [200 300 400 500 600 700 800 900 1000]';
kalibrering_mV      = [2337 1741 1299 1029  841  712  615  521  462]';

%% === Plot ===
figur = figure;
hold on; grid on; box on;

% 1) Plot linje for gyldige punkter (avstand på x)
plot(avstand_mm, spenning_mV, 'LineWidth', 1.5);

% 2) Kun punkter for kalibreringspunktene
scatter(kalibrering_avstand, kalibrering_mV, ...
        55, 'filled', 'MarkerFaceColor', 'r');

xlabel('Avstand [mm]');
ylabel('Spenning [mV]');
title('Lookup-tabell: Spenning som funksjon av avstand');

set(gca, 'FontSize', 12);

%% x-ticks hver 100 mm
xticks(200:100:1000);
xlim([177 1000]);
ylim([462 2500])
hold off;

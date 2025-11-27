clear; clc; close all;

%% === Last inn data ===
data = readmatrix("bevis_kalibring.csv");
t = data(:,1);
y = data(:,2);

%% === Definer nivåene ===
niv = (200:100:1000)';     % nominelle nivåer
tol = 30;                  % ±30 mm vindu
th  = 2;                   % terskel for stasjonaritet

%% === Finn perioder der vogna står stille ===
dy = abs([0; diff(y)]);
stall = dy < th;

%% === Beregn maksimal ± amplitude for hvert nivå ===
amp_per_level = NaN(size(niv));
t_level = NaN(size(niv));

for k = 1:numel(niv)
    d = niv(k);

    idx = stall & abs(y - d) < tol;
    vals = y(idx);

    if numel(vals) < 5
        continue
    end

    avvik = vals - d;

    max_plus  = max(avvik);
    max_minus = min(avvik);

    amp = max(abs(max_plus), abs(max_minus));   % maksimal amplitude

    amp_per_level(k) = amp;
    t_level(k) = mean(t(idx));                  % plasser punktet
end

%% === Plot ===
figure;

yyaxis left
plot(t, y, 'Color', [0.6 0.6 0.6], 'LineWidth', 1.2);
ylabel('Målt avstand [mm]');

yyaxis right
plot(t_level, amp_per_level, 'r-o', 'MarkerSize', 8, 'LineWidth', 1.5);
ylabel('Støy (maks ± amplitude) [mm]');

grid on; box on;
xlabel('Tid [s]');
title('Råmålinger med støy (maks ± amplitude) som punkter med linje');
legend({'Råmålinger', 'Støy (maks ± amplitude)'}, 'Location', 'best');

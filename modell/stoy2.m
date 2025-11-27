clear; clc; close all;

%% === LaTeX som standard ===
set(groot,'defaultTextInterpreter','latex');
set(groot,'defaultLegendInterpreter','latex');
set(groot,'defaultAxesTickLabelInterpreter','latex');

%% === Les inn data ===
data = readmatrix("bevis_kalibring.csv");
t = data(:,1);
y = data(:,2);

%% === Nivaer ===
niv = (200:100:900)';     
tol = 30;
th  = 2;

std_per_level = NaN(size(niv));
t_level       = NaN(size(niv));

for k = 1:numel(niv)
    d = niv(k);

    idx_niv = find(abs(y - d) < tol);

    if numel(idx_niv) < 20
        continue
    end

    % ta bare platoet
    i1 = idx_niv(1);
    i2 = idx_niv(end);

    L = i2 - i1 + 1;
    trim = max(round(0.1*L), 10);

    i1t = i1 + trim;
    i2t = i2 - trim;

    if i2t <= i1t
        continue
    end

    idx_plato = i1t:i2t;
    vals = y(idx_plato);

    std_per_level(k) = std(vals);
    t_level(k)       = mean(t(idx_plato));
end

%% === Plot ===
figure;
ax = gca;

yyaxis(ax,'left');
h1 = plot(t, y, 'Color', [0.2 0.2 0.2], 'LineWidth', 1.0);
ylabel('M\aa lt avstand [mm]', 'Color', 'k');
set(ax, 'YColor', 'k');

yyaxis(ax,'right');
h2 = plot(t_level, std_per_level, 'r-o', 'LineWidth', 1.5, 'MarkerSize', 8);
ylabel('St\o y (std) [mm]', 'Color', 'r');

ylim([0, max(std_per_level,[],'omitnan') * 1.2]);

grid on; box on;
xlabel('Tid [s]');
title('Avstandsm\aa ling med standardavvik st\o y som punkter med linje');

lgd = legend(ax,[h1 h2], {'Avstand','St\o y (std)'}, 'Location', 'best');
lgd.AutoUpdate = 'off';

set(gcf,'Renderer','painters');

% lagre (velg en av disse)
% exportgraphics(gcf,'stoystd.pdf','ContentType','vector');
% print(gcf,'stoystd','-dpdf','-painters');

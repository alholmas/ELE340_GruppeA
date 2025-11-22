%% LaTeX-styling (globalt)
set(groot, 'defaultTextInterpreter', 'latex');
set(groot, 'defaultLegendInterpreter', 'latex');
set(groot, 'defaultAxesTickLabelInterpreter', 'latex');

%% --- Hent data fra simulering (out.r, out.y, out.u) ---
r_ts = out.r;    % referanse (timeseries)
y_ts = out.y;    % prosessverdi (timeseries)
u_ts = out.u;    % pådrag (timeseries)

t = y_ts.Time;
y = y_ts.Data;
r = r_ts.Data;
u = u_ts.Data;

%% --- Finn tidspunkt for sprang i referansen ---
r0 = r(1);
idx_sprang = find(abs(r - r0) > 1e-6, 1, 'first');
if isempty(idx_sprang)
    error('Fant ikke sprang i referansen.');
end
t_sprang = t(idx_sprang);

%% --- 63 % av steget ---
r_slutt = r(end);
delta_r = r_slutt - r0;
y_63    = r0 + 0.63*delta_r;

if delta_r >= 0
    idx_63 = find(t >= t_sprang & y >= y_63, 1, 'first');
else
    idx_63 = find(t >= t_sprang & y <= y_63, 1, 'first');
end
t_63 = t(idx_63);

tau   = 0.1;
t_tau = t_sprang + tau;

delta_t_63 = t_63 - (t_sprang + tau);

%% --- Oversving ---
idx_after_step = idx_sprang;
[y_max, idx_rel] = max(y(idx_after_step:end));
idx_ymax = idx_rel + idx_after_step - 1;

oversving_pct = (y_max - r_slutt) / abs(delta_r) * 100;

%% --- 2 % settling time ---
feil      = y - r;
toleranse = 0.02 * abs(delta_r);
N_vindu   = 2000;

idx_kandidater = find(t >= t_sprang & abs(feil) <= toleranse);
t_settle   = NaN;
idx_settle = NaN;

for k = 1:length(idx_kandidater)
    i0 = idx_kandidater(k);
    i1 = i0 + N_vindu - 1;
    if i1 <= length(t)
        if max(abs(feil(i0:i1))) <= toleranse
            t_settle   = t(i0);
            idx_settle = i0;
            break;
        end
    end
end

%% --- Print resultater ---
fprintf('\n--- Analyse av steprespons ---\n');
fprintf('t_sprang = %.4f s\n', t_sprang);
fprintf('t_63%% = %.4f s\n', t_63);
fprintf('t_63%% - (t_sprang + tau) = %.4f s\n', delta_t_63);
fprintf('Oversving: %.2f %%\n', oversving_pct);
if isnan(t_settle)
    fprintf('2%% settling time: ikke funnet\n');
else
    fprintf('2%% settling time = %.4f s\n', t_settle);
end

%% --- Plot (alt på VENSTRE akse) ---
fig = figure;
set(fig, 'Color', 'w');
hold on; grid on; box on;
pl_r = plot(t, r, 'LineWidth', 1.8, 'Color', 'b', 'LineStyle','-'); % blå
pl_y = plot(t, y, 'LineWidth', 1.8, 'Color', 'k',     'LineStyle','-'); % sort
pl_u = plot(t, u, 'LineWidth', 1, 'Color', 'r',         'LineStyle','-'); 

ylabel('Avstand [mm]','FontSize',15);   % felles akse
yline(250, LineStyle='--', Color='k', LineWidth=0.7)
text(mean(t),260,'Integratorbegrensning','BackgroundColor','w')
%% --- Markører og tekster ---
plot(t_63, y(idx_63), 'ko', 'MarkerFaceColor', 'k', 'MarkerSize', 6);

y_span = max(y) - min(y);
text(t_63-0.34, y(idx_63)+20, ...
    sprintf('$t_c = %.3f\\,s$', delta_t_63), 'BackgroundColor','w');
line([t_tau, t_63], [y_63, y_63], ...
     'Color', 'k', 'LineStyle', '--', 'LineWidth', 1);
plot(t(idx_ymax), y(idx_ymax), 'ks', 'MarkerFaceColor','k','MarkerSize',6);
yl_span = max(y) - min(y);
text(t(idx_ymax), y(idx_ymax) + 0.05*yl_span, ...
    sprintf('$M_p = %.1f\\,\\%%$', oversving_pct), ...
    'HorizontalAlignment','center','VerticalAlignment','bottom');

if ~isnan(t_settle)
    plot(t_settle, y(idx_settle), 'kd', 'MarkerFaceColor','k','MarkerSize',6);
    text(t_settle, y(idx_settle) + 0.05*yl_span, ...
        sprintf('$2\\%%\\ settling = %.3f\\,s$', t_settle), ...
        'HorizontalAlignment','center','VerticalAlignment','bottom');
end

xline(t_tau,'--','LineWidth',1.2);

xlabel('t [s]','FontSize',15);
title('Sprangrespons','FontSize',18);

%% --- Legend ---
%% --- Akser --- 
xticks(0:0.1:max(t));
ax = gca;
ax.XTickLabel = arrayfun(@(x) sprintf('%.1f', x), ax.XTick, 'UniformOutput', false);

% Litt padding rundt y
y_min = min([y; u; r]);
y_max = max([y; u; r]);
dy    = y_max - y_min;
if dy == 0, dy = 1; end
ylim([y_min - 0.3*dy, y_max + 0.1*dy]);

ax.YAxis(1).Color = [0 0 0];
ax.XAxis.Color    = [0 0 0];
hold on

%save('uawu','t','y','r','u')
load('uawu')
awu = plot(t, y, LineWidth=1, Color='k', LineStyle='--')
awy = plot(t, u, 'LineWidth', 1, 'Color', 'r',         'LineStyle','--'); % rød

idx_after_step = idx_sprang;
[y_max, idx_rel] = max(y(idx_after_step:end));
idx_ymax = idx_rel + idx_after_step - 1;

oversving_pct = (y_max - r_slutt) / abs(delta_r) * 100;
text(t(idx_ymax), y(idx_ymax) + 0.05*yl_span, ...
    sprintf('$M_p = %.1f\\,\\%%$', oversving_pct), ...
    'HorizontalAlignment','center','VerticalAlignment','bottom');
legend([pl_r, pl_y, pl_u,awu,awy], {'$R(t)$','$y_{awu}(t)$','$u_{awu}(t)$','$y(t)$','$u(t)$'}, 'Location','best');
hold off;
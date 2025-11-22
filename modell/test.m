%% LaTeX-styling (globalt)
set(groot, 'defaultTextInterpreter', 'latex');
set(groot, 'defaultLegendInterpreter', 'latex');
set(groot, 'defaultAxesTickLabelInterpreter', 'latex');

%% --- Hent data ---
e_ts       = out.e;
u_ts       = out.u;
u_unsat_ts = out.u_unsat;
kp_ts      = out.kp;
ki_ts      = out.ki;
kb_ts      = out.kb;
kif_ts     = out.kif;
fsum_ts    = out.fsum;      % fsum-signal

t          = e_ts.Time;
e_v        = e_ts.Data;
u_v        = u_ts.Data;
u_unsat_v  = u_unsat_ts.Data;
kp_v       = kp_ts.Data;
ki_v       = ki_ts.Data;
kb_v       = kb_ts.Data;
kif_v      = kif_ts.Data;
fsum_v     = fsum_ts.Data;

%% --- Klipp tidsrom ---
t_max  = 1.3;
t_mask = t <= t_max;

t_plot       = t(t_mask);
e_v_plot     = e_v(t_mask);
u_v_plot     = u_v(t_mask);
u_unsat_plot = u_unsat_v(t_mask);
kp_plot      = kp_v(t_mask);
ki_plot      = ki_v(t_mask);
kb_plot      = kb_v(t_mask);
kif_plot     = kif_v(t_mask);
fsum_plot    = fsum_v(t_mask);

%% =====================================================================
%  FIGUR 2 – Anti-windup-mekanikk
%% =====================================================================

fig_aw = figure;
set(fig_aw, 'Color', 'w');

%% Subplot 1: e(t), u(t), u_unsat(t) (kun når u_unsat > u)
subplot(2,1,1);
hold on; grid on; box on;
pl_u  = plot(t_plot, u_v_plot, 'LineWidth', 1.4, 'LineStyle','-');
pl_e  = plot(t_plot, e_v_plot, 'LineWidth', 1.4, 'LineStyle','-');


% Vis u_unsat kun der den er over u(t)
u_unsat_over = u_unsat_plot;
u_unsat_over(u_unsat_over <= u_v_plot) = NaN;

pl_uu = plot(t_plot, u_unsat_over, 'LineWidth', 1.4, 'LineStyle','-.' );

% Vertikal linje ned til u(t) i første punkt der u_unsat > u
idx_first = find(u_unsat_plot > u_v_plot, 1, 'first');
if ~isempty(idx_first)
    t_f   = t_plot(idx_first);
    y_uns = u_unsat_plot(idx_first);
    y_u   = u_v_plot(idx_first);
    plot([t_f t_f], [y_u y_uns], '-', ...
    'LineWidth', 1.2, ...
    'Color', pl_uu.Color, 'LineStyle','-.');     % same color as unsat-kurven
end

ylabel('Signalverdi');
title('$e(t),\ u(t),\ u_{unsat}(t)$');

yline(250,'--','LineWidth',1,'Color','k');
text(0.05, 280, 'Integratorbegrensning','BackgroundColor','w');

legend([pl_e, pl_u, pl_uu], ...
       {'$e(t)$', '$u(t)$', '$u_{unsat}(t) > u(t)$'}, ...
       'Location','best');

ylim([-50  max(u_unsat_plot)*1.05]);
xlim([0 t_max]);
xticks(0:0.1:t_max);

%% Subplot 2: Kp og Ki
subplot(2,1,2);
hold on; grid on; box on;

pl_kp  = plot(t_plot, kp_plot, 'LineWidth', 1.4, 'LineStyle','-');
pl_ki  = plot(t_plot, ki_plot, 'LineWidth', 1.4, 'LineStyle','--');

xlabel('Tid [s]');
ylabel('Regulatorparametre');
title('$K_p,\ K_i$','Interpreter','latex');

legend([pl_kp, pl_ki], ...
       {'$K_p$', '$K_i$'}, ...
       'Location','best');

xlim([0 t_max]);
xticks(0:0.1:t_max);
ylim([min(ki_plot)*1.1  max(kp_plot)*1.1]);


%% =====================================================================
%  FIGUR 3 – Regulatorparametre + fsum
%% =====================================================================

fig_param = figure;
set(fig_param, 'Color', 'w');
hold on; grid on; box on;

pl_fsum = plot(t_plot, fsum_plot, 'LineWidth', 1.4, 'LineStyle','-','Color',[0.3 0.3 1]);
pl_ki2  = plot(t_plot, ki_plot,   'LineWidth', 1.4, 'LineStyle','--');
pl_kb2  = plot(t_plot, kb_plot,   'LineWidth', 1.4, 'LineStyle','-.');
pl_kif2 = plot(t_plot, kif_plot,  'LineWidth', 1.4, 'LineStyle',':');

xlabel('Tid [s]');
ylabel('Regulatorparametre');
title('$K_i(t),\ K_{aw}(t),\ K_{if}(t),\ fsum(t)$');

legend([pl_fsum, pl_kb2, pl_kif2, pl_ki2], ...
       {'$e \cdot K_i$', 'Bidrag fra aw', 'F\o r integrator', 'Integratorbidrag'}, ...
       'Location','best');

xlim([0 t_max]);
xticks(0:0.1:t_max);

ymin3 = min([ki_plot; kb_plot; kif_plot; fsum_plot]);
ymax3 = max([ki_plot; kb_plot; kif_plot; fsum_plot]);

if ymin3 == ymax3
    ymin3 = ymin3 - 1;
    ymax3 = ymax3 + 1;
end

ylim([ymin3*1.05  ymax3*1.05]);

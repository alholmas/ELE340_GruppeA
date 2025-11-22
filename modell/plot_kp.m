%% =====================================================================
%  EGEN FIGUR MED 2 SUBPLOTS
%% =====================================================================

fig_pid = figure;
set(fig_pid, 'Color', 'w');

%% Hent timeseries
e_ts       = out.e;
u_ts       = out.u;
u_unsat_ts = out.u_unsat;
kp_ts      = out.kp;
ki_ts      = out.ki;
kb_ts      = out.kb;
kif_ts     = out.kif;     % ny: K_if

t          = e_ts.Time;
e_v        = e_ts.Data;
u_v        = u_ts.Data;
u_unsat_v  = u_unsat_ts.Data;
kp_v       = kp_ts.Data;
ki_v       = ki_ts.Data;
kb_v       = kb_ts.Data;
kif_v      = kif_ts.Data;

%% Lag maske for t <= 1.2 s
t_mask       = t <= 1.22;

t_plot       = t(t_mask);
e_v_plot     = e_v(t_mask);
u_v_plot     = u_v(t_mask);
u_unsat_plot = u_unsat_v(t_mask);
kp_plot      = kp_v(t_mask);
ki_plot      = ki_v(t_mask);
kb_plot      = kb_v(t_mask);
kif_plot     = kif_v(t_mask);   % filtrert K_if

%% =====================================================================
%  SUBPLOT 1 – e(t), u(t), u_unsat(t)
%% =====================================================================
subplot(2,1,1);
hold on; grid on; box on;

plot(t_plot, e_v_plot,      'LineWidth', 1.4, 'LineStyle','-');
plot(t_plot, u_v_plot,      'LineWidth', 1.4, 'LineStyle','-');
plot(t_plot, u_unsat_plot,  'LineWidth', 1.4, 'LineStyle','--' );

ylabel('Signalverdi');
title('$e(t),\ u(t),\ u_{unsat}(t)$','Interpreter','latex');

% Integratorbegrensning
yline(250, 'LineWidth',0.5, 'LineStyle','--', 'Color','k');
text(0.05, 280, 'Integratorbegrensning', 'BackgroundColor','w');

legend({'$e(t)$', '$u(t)$', '$u_{unsat}(t)$'}, ...
       'Location','best');

ylim([-50  max(u_unsat_plot)*1.05]);
xlim([0 1.2]);
xticks(0:0.1:1.2);

%% =====================================================================
%  SUBPLOT 2 – Kp, Ki, Kb, Kif, u(t)
%% =====================================================================
subplot(2,1,2);
hold on; grid on; box on;

pl_kp  = plot(t_plot, kp_plot,  'LineWidth', 1.4, 'LineStyle','-');
pl_ki  = plot(t_plot, ki_plot,  'LineWidth', 1.4, 'LineStyle','--');
xlabel('Tid [s]');
ylabel('Regulatorparametre og u(t)');
title('$K_p,\ K_i$','Interpreter','latex');

legend([pl_kp, pl_ki], ...
       {'$K_p$', '$K_i$'}, ...
       'Location','best');

xlim([0 1.2]);
xticks(0:0.1:1.2);

% Y-limits: dekk både gains, K_if og u(t)
ymin = min([kp_plot; ki_plot; kb_plot; kif_plot; u_v_plot]);
ymax = max([kp_plot; ki_plot; kb_plot; kif_plot; u_v_plot]);

ylim([ymin*1.05  ymax*1.05]);

hold off;

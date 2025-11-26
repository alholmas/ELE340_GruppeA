%% =====================================================================
%  EGEN FIGUR MED SKRAVERINGSOMRÅDE MELLOM u(t) OG u_unsat(t)
%% =====================================================================

fig_pid = figure;
set(fig_pid, 'Color', 'w');

%% Hent timeseries
e_ts       = out.e;
u_ts       = out.u;
u_unsat_ts = out.u_unsat;

t          = e_ts.Time;
e_v        = e_ts.Data;
u_v        = u_ts.Data;
u_unsat_v  = u_unsat_ts.Data;

%% Lag maske for t <= 1.22 s
t_mask       = t <= 1.22;

t_plot       = t(t_mask);
e_v_plot     = e_v(t_mask);
u_v_plot     = u_v(t_mask);
u_unsat_plot = u_unsat_v(t_mask);

%% =====================================================================
%  SUBPLOT 1 – e(t), u(t), u_unsat(t)
%% =====================================================================
subplot(1,1,1);
hold on; grid on; box on;

%% --- SKRAVERINGSMASKE ---
% Finn øvre og nedre verdi i hvert punkt (skravering funker begge veier)
y_overst = max(u_v_plot, u_unsat_plot);
y_nederst = min(u_v_plot, u_unsat_plot);

% Lag skravert område
fyll = fill([t_plot; flip(t_plot)], ...
            [y_overst; flip(y_nederst)], ...
            [0.8 0.8 0.8], ...       % lys grå
            'LineStyle','none', ...
            'FaceAlpha',0.4);        % gjennomskinnelighet

uistack(fyll,'bottom');               % send skravering bak kurvene

%% --- VANLIGE KURVER ---
plot(t_plot, e_v_plot,     'LineWidth', 1.4, 'LineStyle','-');
plot(t_plot, u_v_plot,     'LineWidth', 1.4, 'LineStyle','-');
plot(t_plot, u_unsat_plot, 'LineWidth', 1.4, 'LineStyle','--');

ylabel('Signalverdi');
title('$e(t),\ u(t),\ u_{unsat}(t)$','Interpreter','latex');

% Integratorbegrensning
yline(250, 'LineWidth',0.5, 'LineStyle','--', 'Color','k');
text(0.6, 280, 'Pådragsbegrensning', 'BackgroundColor','w');

legend({'Integreres opp', '$e(t)$', '$u(t)$', '$u_{unsat}(t)$'}, ...
       'Location','best');

ylim([-50  max(u_unsat_plot)*1.05]);
xlim([0 1.2]);
xticks(0:0.1:1.2);

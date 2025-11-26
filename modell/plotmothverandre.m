%% LaTeX-styling
set(groot,'defaultTextInterpreter','latex');
set(groot,'defaultLegendInterpreter','latex');
set(groot,'defaultAxesTickLabelInterpreter','latex');

%% ============================
%  HENT SIMULERTE SIGNALER
%% ============================
t       = out.y.Time;
y       = out.y.Data;
r       = out.r.Data;
u       = out.u.Data;
e_sim   = out.e.Data;

% Her bruker vi dine "originale" navngitte signaler fra modellen:
up_sim  = out.kp.Data;   % proporsjonalt bidrag
ui_sim  = out.ki.Data;   % integraldel
ud_sim  = out.kd.Data;   % derivatdel

%% ============================
%  LES MÅLT DATA FRA FIL
%% ============================
malt = readmatrix("plots/350 steg kp521.txt");

% Kolonner:
% 1: tid [s]
% 2: avstand y [mm]
% 3: settpunkt [mm]
% 4: feil e [mm]
% 5: u_total (må / 1000)
% 6: up (må / 1000)
% 7: ui (må / 1000)
% 8: ud (må / 1000)

t_m      = malt(:,1);
y_m      = malt(:,2);
r_m      = malt(:,3);
e_m      = malt(:,4);           % feil direkte fra logg (mm)

u_m_raw  = malt(:,5);
up_raw   = malt(:,6);
ui_raw   = malt(:,7);
ud_raw   = malt(:,8);

% Skaler pådragene (samme skala som simulert u)
skalafaktor = 1000;
u_m   = u_m_raw  / skalafaktor;
up_m  = up_raw   / skalafaktor;
ui_m  = ui_raw   / skalafaktor;
ud_m  = ud_raw   / skalafaktor;

%% ============================
%  FINN SPRANG I SIMULERT OG MÅLT SETTPUNKT
%% ============================
% simulert
r0_sim       = r(1);
idx_sprang_s = find(abs(r - r0_sim) > 1e-6, 1, 'first');
if isempty(idx_sprang_s)
    error("Fant ikke sprang i simulert settpunkt!");
end
t_sprang_sim = t(idx_sprang_s);

% målt
r0_m = r_m(1);
idx_sprang_m = find(abs(r_m - r0_m) > 1e-6, 1, 'first');
if isempty(idx_sprang_m)
    error("Fant ikke sprang i målt settpunkt!");
end
t_sprang_m = t_m(idx_sprang_m);

%% ============================
%  SYNKRONISER TID OG RESAMPLE MÅLT DATA
%% ============================
% Skift målt tid slik at spranget havner på samme tidspunkt som simulert:
delta_t  = t_sprang_sim - t_sprang_m;
t_m_synk = t_m + delta_t;

% Resample alle målte signaler til simulert tidsvektor t
r_m_rs  = interp1(t_m_synk, r_m,  t, 'linear', 'extrap');
y_m_rs  = interp1(t_m_synk, y_m,  t, 'linear', 'extrap');
e_m_rs  = interp1(t_m_synk, e_m,  t, 'linear', 'extrap');

u_m_rs  = interp1(t_m_synk, u_m,  t, 'linear', 'extrap');
up_m_rs = interp1(t_m_synk, up_m, t, 'linear', 'extrap');
ui_m_rs = interp1(t_m_synk, ui_m, t, 'linear', 'extrap');
ud_m_rs = interp1(t_m_synk, ud_m, t, 'linear', 'extrap');

%% ============================
%  FIGUR 1 – Sprangrespons simulert vs målt (synkronisert)
%% ============================
fig1 = figure; clf; hold on; grid on; box on;
set(fig1,'Color','w');

% Simulert
plot(t, r, 'b', 'LineWidth',1.8);
plot(t, y, 'k', 'LineWidth',1.8);
plot(t, u, 'r', 'LineWidth',1.2);

% Målt, synket og resamplet
plot(t, r_m_rs, 'b:', 'LineWidth',1.6);
plot(t, y_m_rs, 'k:', 'LineWidth',1.6);
plot(t, u_m_rs, 'r:', 'LineWidth',1.2);

xlabel('Tid [s]');
ylabel('Signalverdi');
title('Sprangrespons – simulert vs målt');

legend({'$r$ simulert','$y$ simulert','$u$ simulert', ...
        '$r$ m\aa lt','$y$ m\aa lt','$u$ m\aa lt'}, ...
        'Location','best');

%% ============================
%  FIGUR 2 – 4 DELPLOTS (alle på samme tidsakse t)
%% ============================
fig2 = figure; clf; set(fig2,'Color','w');

%% --- 1: u vs e ---
subplot(3,1,1); hold on; grid on; box on;
plot(t, u,      'r',  'LineWidth',1.4);
plot(t, u_m_rs, 'r:', 'LineWidth',1.4);
plot(t, e_sim,  'k',  'LineWidth',1.4);
plot(t, e_m_rs, 'k:', 'LineWidth',1.4);

legend({'$u$ simulert', '$u$ m\aa lt', ...
        '$e$ simulert', '$e$ m\aa lt'}, 'Location','best');
ylabel('$u / e$');
title('u og e – simulert vs målt');

sett_ylim([u; u_m_rs; e_sim; e_m_rs]);   % <--- 5% padding


%% --- 2: up ---
subplot(3,1,2); hold on; grid on; box on;
plot(t, up_sim,   'b',  'LineWidth',1.4);
plot(t, up_m_rs,  'b:', 'LineWidth',1.4);
ylabel('$u_p$');
legend({'$u_p$ simulert','$u_p$ m\aa lt'}, 'Location','best');
title('PID-deler – simulert vs målt');

sett_ylim([up_sim; up_m_rs]);   % <--- 5% padding


%% --- 3: ui ---
subplot(3,1,3); hold on; grid on; box on;
plot(t, ui_sim,   'g',  'LineWidth',1.4);
plot(t, ui_m_rs,  'g:', 'LineWidth',1.4);
ylabel('$u_i$');
legend({'$u_i$ simulert','$u_i$ m\aa lt'}, 'Location','best');

sett_ylim([ui_sim; ui_m_rs]);   % <--- 5% padding


% %% --- 4: ud ---
% subplot(4,1,4); hold on; grid on; box on;
% plot(t, ud_sim,   'm',  'LineWidth',1.4);
% plot(t, ud_m_rs,  'm:', 'LineWidth',1.4);
% ylabel('$u_d$');
% xlabel('Tid [s]');
% legend({'$u_d$ simulert','$u_d$ m\aa lt'}, 'Location','best')

function sett_ylim(data)
    dmin = min(data);
    dmax = max(data);
    if dmin == dmax
        % Hvis signalet er helt konstant
        dmin = dmin - 1;
        dmax = dmax + 1;
    end
    span = dmax - dmin;
    padding = 0.05 * span;   % 5%
    ylim([dmin - padding, dmax + padding]);
end

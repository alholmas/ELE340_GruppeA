% ==============================================================
% Plotter én step-måling fra CSV-fil + pådragsplata
% ==============================================================

clear; clc; close all;

% Mappe (ikke brukt her, men beholdes)
plotmappe = fullfile(pwd, 'plots');

% Last inn måledata
data = load('C:\Users\alexa\OneDrive\Dokumenter\GitHub\ELE340\GUI\ELE340_GruppeA-1\GUI\plots\step10cm_inn1.csv');

tid     = data(:, 1);
avstand = data(:, 2);

% --------------------------------------------------------------
% Figur og avstandsplot (venstre akse)
% --------------------------------------------------------------
figure;

% Lagre handle til avstands-plottet
h_avstand = plot(tid, avstand, 'LineWidth', 1, 'Color', 'black');
grid on;
hold on;

xlabel('Tid (s)');
ylabel('Avstand (mm)');
title('Prosesskonstant');
% --------------------------------------------------------------
% Markør for 63% punkt (tidskonstantpunkt)
% --------------------------------------------------------------

t_tau = 0.9;
y_tau63 = 330;

% Punktmarkør
% plot(t_tau, 337.5, 'ko', 'MarkerSize', 7, 'MarkerFaceColor', 'yellow');

% Stiplede hjelpelinjer
%plot([t_tau t_tau], [min(avstand) max(avstand)], 'k--', 'LineWidth', 1);
%plot([0 max(tid)], [y_tau63 y_tau63], 'k--', 'LineWidth', 1);

% Label


% Vertikale linjer
x1 = 0.2;
x2 = 1.2;
xticks(0:0.1:1.5)
xline(x1, 'Color', 'r', 'LineWidth', 1, 'LineStyle','--');
xline(x2, 'Color', 'r', 'LineWidth', 1, 'LineStyle','--');
% Prosessverdi ved t_tau (hentet fra selve avstandskurven)
y_under = interp1(tid, avstand, t_tau);
% --------------------------------------------------------------
% To punktmarkører på prosesskurven ved t = 0.2 s og t = 1.2 s
% --------------------------------------------------------------

% Hent y-verdiene direkte fra avstandskurven
y_x1 = interp1(tid, avstand, x1);
y_x2 = interp1(tid, avstand, x2);

% Plot punktmarkører
plot(x1, y_x1, 'ko', 'MarkerSize', 7, 'MarkerFaceColor', 'blue', 'LineWidth',1.2);
plot(x2, y_x2, 'ko', 'MarkerSize', 7, 'MarkerFaceColor', 'blue', 'LineWidth',1.2);

% (valgfritt) legg inn etiketter
text(x1+0.03, y_x1-0.4, ...
    sprintf('$y_{m_1}$ = %i mm', y_x1), ...
    'Interpreter','latex','FontSize',12 );

text(x2+0.02, y_x2+3.5, sprintf('$y_{m_2}$ = %i mm', y_x2), ...
    'Interpreter','latex','FontSize',12 );

% Vertikal linje fra 63%-punktet og ned til kurven
% plot([t_tau t_tau], [y_tau63 337.5], 'k--', 'LineWidth', 1)
% plot([0.2 0.9], [y_tau63 y_tau63],'k', LineWidth=1.5)

% --------------------------------------------------------------
% Pil for prosesskonstant K
% --------------------------------------------------------------
y_pil = 350;   % manuelt valgt høyde



% Linje for τ
plot([0.1 0.2], [y_tau63 y_tau63], 'k', 'LineWidth', 1.5);
y_tau = 360;

% Pilhoder
markerstorrelse = 7;
skyv = 0.011;
plot(0.1+skyv, y_tau63, 'k<', 'MarkerSize', markerstorrelse, 'MarkerFaceColor', 'k');
plot(0.2-skyv, y_tau63, 'k>', 'MarkerSize', markerstorrelse, 'MarkerFaceColor', 'k');
%plot(0.2+skyv, y_tau63, 'k<', 'MarkerSize', markerstorrelse, 'MarkerFaceColor', 'k');
%plot(0.9-skyv, y_tau63, 'k>', 'MarkerSize', markerstorrelse, 'MarkerFaceColor', 'k');

% Tekst over pila (K)
text( ...
     0.64,370, ...    
    '$|K|=\frac{|\Delta y_m|}{\Delta t \cdot u}=0.96[\mu m]$', ...
    'Interpreter', 'latex', ...
    'BackgroundColor','white', ...
    'FontSize', 17);

% Tekst for τ og Δt
text(0.2, y_tau63+5, '$\tau=0.1\,\mathrm{s}$', ...
    'BackgroundColor', 'white', ...
    'Interpreter', 'latex', 'FontSize',15);

text(0.4, y_tau63+5, '$\Delta t = 1\,\mathrm{s}$', ...
    'BackgroundColor', 'white', ...
    'Interpreter', 'latex', 'FontSize',15);
plot([x1 x2], [y_tau63 y_tau63], 'k', 'LineWidth', 1.5);
plot(x1+skyv, y_tau63, 'k<', 'MarkerSize', markerstorrelse, 'MarkerFaceColor', 'k');
plot(x2-skyv, y_tau63, 'k>', 'MarkerSize', markerstorrelse, 'MarkerFaceColor', 'k');
% --------------------------------------------------------------
% Pådragsplata på høyre y-akse
% --------------------------------------------------------------
% --------------------------------------------------------------
% Pådragsplata på høyre y-akse
% --------------------------------------------------------------
yyaxis right   % bytt til høyre akse

% Sett høyreaksens farge til blå (samme som pådraget)
ax = gca;
ax.YAxis(2).Color = [0 0.4 1];

% Lag pådragsignal
dt = 0.001;
tid_pad = 0:dt:1.5;
pad = zeros(size(tid_pad));
pad(tid_pad >= 0.1 & tid_pad < 1.1) = 100;   % 100 kHz plateau

% Plot pådrag og lagre handle
h_padrag = plot(tid_pad, pad, 'LineWidth', 1.5, ...
                'Color', [0 0.4 1], 'LineStyle', '--');

ylabel('Pådrag (kHz)');
ylim([-10 120])   % litt luft rundt 0 og 100

% --------------------------------------------------------------
% Legend med riktige serier
% --------------------------------------------------------------
yyaxis left   % bare for å være eksplisitt om aktiv akse

leg = legend([h_avstand, h_padrag], {'Avstand', 'Pådrag'}, ...
       'Location', 'best', ...
       'FontSize', 15, ...
       'Interpreter','none');   % ingen tex-magi i legend

leg.AutoUpdate = 'off';
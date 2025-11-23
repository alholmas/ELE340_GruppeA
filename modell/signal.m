%% --- Hent data fra Simulink ---
t = out.u.Time;     % felles tidsakse

u = out.u.Data;     % pådrag
y = out.y.Data;     % målt posisjon
e = out.e.Data;     % feil
r = out.r.Data;     % referanse

%% --- Plot alle i samme figur ---
figure('Color','w'); hold on; grid on; box on;

plot(t, u, 'LineWidth', 1.4);
plot(t, y, 'LineWidth', 1.4);
plot(t, e, 'LineWidth', 1.4);
%plot(t, r, 'LineWidth', 1.4);

xlabel('Tid [s]');
ylabel('Signalverdi');

legend({'u(t) – pådrag', 'y(t) – posisjon', 'e(t) – feil', 'r(t) – referanse'}, ...
       'Location','best');

title('Signalplot: u(t), y(t), e(t), r(t)');

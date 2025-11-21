Kp = 4;
Ti = 0;
Td = 0;
K = 0.89;
Tau = 0.04;

setpoint = 200;
avstand_t0 = 400;

if Ti ~= 0
    Ki = Kp / Ti;
else
    Ki = 0;
end
Kd = Kp*Td;




Kp = 5.21;
Ti = 0.8;
Td = 0 ;
K = 0.96;
Tau = 0.1;
kb=4;
setpoint = 100;
avstand_t0 = 0;
sat = 20;

if Ti ~= 0
    Ki = Kp / Ti;
else
    Ki = 0;
end
Kd = Kp*Td;




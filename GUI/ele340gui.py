"""

Baud rate - 115200
dataformat - 8N1
Datastruktur - 
Hvordan sender stm32 avstandsdata, pådrag, akselerasjon osv (float/int?) - Data blir sendt via USART2 (?) 
    Hvilken rekkefølge kommer dataene i? kommer alt samtidig? 
    Trenger eksakt format på dataene fra STM
Hvordan mottar stm32 kommandoer (P, I, D) Hvordan tolker STM disse meldingene
    Skal det sendes som tekst eller binært?
Matplotlib - hvordan sette opp en sanntids graf + threading
    * Sanntidsgraf
    * Start/stopp med tastetrykk (K/S)
    * logging av data til fil (.txt fil f. eks) - lagres løpende 
samplingstid (10ms eller 30ms?)
Må lese fra serieporten kontinuerlig uten å blokkere GUI threaden (threading eller asynkron I/O)
        * Threading lar oss kjøre flere operasjoner samtidig i samme program uten at python stopper opp mens den venter på mer data fra STM

Skal avstandsreferanse settes med GUI eller hardkodes i STM?
    skal det gå an å endre avstandsreferanse under kjøring?
        
Se på rammeverket i STM for å se hvordan de sender data via USART2 - Hvordan har aksel lagt det inn for å snede data

Skal det være en full binær/hex protokoll eller en kombinasjon av tekst og binær data?



Signaler:
    fra PC:
        PID
        start/stop logging
        Homing (avstands kalibrering ved start)
    til PC:
        avstand
        pådrag - PID
        akselerasjon
        error
        tid











"""


# Biblioteker

import numpy as np
import matplotlib.pyplot as plt
import serial
import threading
import time
from collections import deque # FIFO prinsippet, first in first out
import sys

# Konfigurasjon

port = "/dev/tty.usbserial-A602ZP6R" # COM...(3, 4, 5..)
baudrate = 115200 # bits per sekund
forespørsel_periode = 0.03 # hvor ofte python sender forespørsel om ny måling. 30ms
forespørsel_kommando = b"f\n" # kommando for å be om ny data - f for forespørsel 
start_kommando = b"k\n" # kommando for å starte logging (kjør)
stopp_kommando = b"s\n" # kommando for å stoppe logging (stopp)

"""
i C: Kjøres når data mottas fra PC

void USART2_IRQHandler(void) {
    char c = USART_ReceiveData(USART2);
    if (c == 'f') {
        send_measurement();  // Les sensorer og send linje
    }
    else if (c == 'k') {
        start_logging();
    }
    else if (c == 's') {
        stop_logging();
    }
}
"""

loggfil = "logg.txt" # filnavn for lagring av data
deque_maxlengde = 2000 # maks antall datapunkter som skal vises i grafen

#

running_lock = threading.Lock() # lås for å beskytte running variabelen
running = True # indikerer om logging er aktiv eller ikke
stopp_program = False # indikerer om programmet skal avsluttes


ax_q = deque(maxlengde=deque_maxlengde)      # akselerasjon i x-retning # q = queue
u_q  = deque(maxlengde=deque_maxlengde)      # pådrag
dist_q = deque(maxlengde=deque_maxlengde)    # avstand
err_q = deque(maxlengde=deque_maxlengde)      # error
t_q  = deque(maxlengde=deque_maxlengde)      # lokal PC-tid

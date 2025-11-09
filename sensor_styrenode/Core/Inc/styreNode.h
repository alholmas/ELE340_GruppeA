#ifndef STYRE_NODE_H
#define STYRE_NODE_H

#include "main.h"
#include "pid.h"

// Modul-API for Styre-Node (kontrollnode)
void StyreNode_Init(void);
void StyreNode_Loop(void);

void set_linmot_paadrag(pid_t *pid);

#endif // STYRE_NODE_H


#ifndef _SIM_H
#define _SIM_H

#include "../airplane/airplane.h"
#include "../converts/converts.h"

#define PORT_MUX 8778 //The port on which to send data
#define PORT2BROAD 10505   //The port on which to send data
#define PORT2APILOT 8888   //The port on which to send data


#define BROADCAST_PERIOD 1 //seconds
#define SIMULATION_PERIOD 0.5 //seconds
#define SIM_STEP 1 //seconds
#endif // _SIM_H

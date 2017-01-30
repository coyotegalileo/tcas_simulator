#ifndef _REC_H
#define _REC_H

#include "../converts/converts.h"
#include "../airplane/airplane.h"

// Ficheiro com waypoints
#define PATH_FILE "flight.path"


#define PORT_BROADCAST 10505 //The port on which to receive data
#define PORT_OUT 8228 //The port on which to send data

#define TOLERANCE_WP 50.0 // Tolerância para mudar way point (metros)


void error(const char *msg);


#endif // _AUTO_PILOT_H

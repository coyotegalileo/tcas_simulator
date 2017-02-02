#ifndef _AUTO_PILOT_H
#define _AUTO_PILOT_H

#include "../Converts/converts.h"
#include "../Airplane/airplane.h"

// Ficheiro com waypoints
#define PATH_FILE "flight.path"

#define PORT2MUX 8558   //The port on which to send data
#define PORT_BROAD 8888 //The port on which to receive data

#define TOLERANCE_WP 50.0 // Tolerância para mudar way point (metros)
#define TOLERANCE_WP_LAT 0.1*3.1415926535897932384626/180 // 12 metros
#define TOLERANCE_WP_LON 0.1*3.1415926535897932384626/180 // 12 metros

void auto_pilot(const std::string& fileName);
void read_waypoint(const std::ifstream inf);
void determine_velocity(Airplane *p_pombo, double xt, double yt, double zt, float groundspeed, float vertspeed);
void error(const char *msg);


#endif // _AUTO_PILOT_H

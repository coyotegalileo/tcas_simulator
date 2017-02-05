 #ifndef _CONVERTS_H
#define _CONVERTS_H

#include <cmath>

#define PI 3.14159265359
#define _TORAD (PI/180)
#define _RT 6371000.0 // Raio da terra metros
#define _MSTOFTMIN 196.850393701


float knots_to_ms(float airspeed);
void geo_to_wgs(float lat, float lon, float alt, double* x, double* y, double* z);
void wgs_to_geo(double x, double y, double z, float* lon, float* lat, float* alt);
void wgs_to_enu(double x, double y, double z,double* x_enu, double* y_enu, double* z_enu, float lat, float lon);
void enu_to_wgs(double x, double y, double z,double* x_enu, double* y_enu, double* z_enu, float lat, float lon);

#endif // _CONVERTS_H

#include "converts.h"

float knots_to_ms(float airspeed)
{
   float airspeed_ms;
   airspeed_ms = airspeed * 0.514444;
   return airspeed_ms;
}

void geo_to_wgs(float lat, float lon, float alt, double* x, double* y, double* z)
{
   // Input em graus
   lon = lon * _TORAD;
   lat = lat * _TORAD;

   // WGS84 ellipsoid constants
   double a = 6378137; // radius
   double b = 6356752.314245;  // eccentricity

   double x_aux, y_aux, z_aux;
   double N;

   N = a*a / (sqrt(a*a * cos(lat)*cos(lat) + b*b * sin(lat)*sin(lat) ));

   x_aux = (N + alt) * cos(lon) * cos(lat);
   y_aux = (N + alt) * sin(lon) * cos(lat);
   z_aux = ((N*b*b/(a*a)) + alt) * sin(lat);

   *x = x_aux;
   *y = y_aux;
   *z = z_aux;


}

void wgs_to_geo(double x, double y, double z, float* lat_back, float* lon_back, float* alt_back)
{
   // WGS84 ellipsoid constants
   double a = 6378137; // radius
   double e = 8.1819190842622e-2;  // eccentricity

   double asq = pow(a,2);
   double esq = pow(e,2);

   double b = sqrt( asq * (1-esq) );
   double bsq = pow(b,2);
   double ep = sqrt( (asq - bsq)/bsq);
   double p = sqrt( pow(x,2) + pow(y,2) );
   double th = atan2(a*z, b*p);

   double lon = atan2(y,x);
   double lat = atan2( (z + pow(ep,2)*b*pow(sin(th),3) ), (p - esq*a*pow(cos(th),3)) );
   double N = a/( sqrt(1-esq*pow(sin(lat),2)) );
   double alt = p / cos(lat) - N;

   // mod lat to 0-2pi
   lon = lon - floor((lon+PI)/(2*PI)) *(2*PI);

   // correction for altitude near poles left out.


   *lon_back = lon;
   *lat_back = lat;
   *alt_back = alt;
}


void wgs_to_enu(double x, double y, double z,double* x_enu, double* y_enu, double* z_enu,float lat, float lon)
{



   *x_enu = x * -sin(lon) + y * cos(lon);
   *y_enu = x * -cos(lon) * sin(lat) + y * -sin(lon) * sin(lat) + z * cos(lat);
   *z_enu = x * cos(lon) * cos(lat) + y * sin(lon) * cos(lat) + z * sin(lat);



}

void enu_to_wgs(double x_enu, double y_enu, double z_enu,double* x, double* y, double* z, float lat, float lon)
{


   *x = x_enu * -sin(lon) + y_enu * -cos(lon) * sin(lat) + z_enu * cos(lon) * cos(lat);
   *y = x_enu * cos(lon) + y_enu * -sin(lon) * sin(lat) + z_enu * sin(lon) * cos(lat);
   *z = y_enu * cos(lat) + z_enu * sin(lat);



}

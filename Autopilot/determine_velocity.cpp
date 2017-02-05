#include "auto_pilot.h"

#define MAX_VU_FTMIN 2600
#define MIN_VU_FTMIN -1800
#define MAX_AIRSPEED_MS 320*0.51444444444   
#define MIN_VSTALL_MS 121*0.51444444444 

void determine_velocity(Airplane *p_pombo, double xt, double yt, double zt, float groundspeed, float vertspeed)
{
   float lat0, lon0, alt0;
   float lat1, lon1, alt1;
   double ve, vn, vu;
   double vx, vy, vz;   
   double dx, dy, dz;
   double fake; 

   // groundspeed vem em nos do ficheiro
   groundspeed = knots_to_ms(groundspeed);

   // vertspeed em ft/min no ficheiro
   vertspeed = vertspeed / _MSTOFTMIN;

   //llh do aviao e do waypoint
   wgs_to_geo(p_pombo->x, p_pombo->y, p_pombo->z, &lat0, &lon0, &alt0);
   wgs_to_geo(xt, yt, zt, &lat1, &lon1, &alt1);   

   // Vector diferença em wgs
   dx = xt - p_pombo->x;
   dy = yt - p_pombo->y;
   dz = zt - p_pombo->z;  
   
   //Dieferença em enu
   wgs_to_enu(dx, dy, dz, &ve, &vn, &fake, lat0, lon0);

   // Controlo para mudar o waypoint
   if(fabs(lat0-lat1) < TOLERANCE_WP_LAT && fabs(lon0-lon1) < TOLERANCE_WP_LON)
      p_pombo->change_target = true;   

   // Velocidade Vertical
   if(fabs(alt1 - alt0) > 120 && vertspeed != 0)
         vu = (alt1 - alt0) / fabs(alt1 - alt0) * vertspeed;
   else
   {
      vu = (alt1 - alt0) * 0.02;      
   }   
      

   // Normalizacao dos vectores e multiplicao pelo groundspeed
   float norma = sqrt( ve*ve + vn*vn );
   if(norma != 0)
   {
      ve = ve * groundspeed / norma;
      vn = vn * groundspeed / norma;
      
   }
   else
   {

   }

   // Limites de Velcidades
   if(vu > MAX_VU_FTMIN / _MSTOFTMIN)
   {
      vu = MAX_VU_FTMIN / _MSTOFTMIN; 
      printf("\nATTENTION MAX VUP\n");
   }
   if(vu < MIN_VU_FTMIN / _MSTOFTMIN)
   {
      vu = MIN_VU_FTMIN / _MSTOFTMIN;
      printf("\nATTENTION MAX VUP\n");
   }

   float airspeed = std::sqrt(ve*ve+vn*vn+vu*vu);
   
   if(airspeed > MAX_AIRSPEED_MS)
   {
      printf("\nATTENTION MAX ASPD\n");
      ve = ve *  MAX_AIRSPEED_MS / airspeed;
      vn = vn *  MAX_AIRSPEED_MS / airspeed;
      vu = vu *  MAX_AIRSPEED_MS / airspeed;
   }
   if(airspeed < MIN_VSTALL_MS )
   {
      printf("\nATTENTION STALL ASPD\n");
      ve = ve *  MIN_VSTALL_MS / airspeed;
      vn = vn *  MIN_VSTALL_MS / airspeed;
      vu = vu *  MIN_VSTALL_MS / airspeed;
   }

   // reconversao das variaveis
   enu_to_wgs(ve, vn, vu, &vx, &vy, &vz, lat0, lon0 );

   p_pombo->vx = vx;
   p_pombo->vy = vy;
   p_pombo->vz = vz;




}


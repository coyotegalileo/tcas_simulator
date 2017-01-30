#include "mux.h"

#define VUP_MAX 30 // m/s
#define GANHO_VUP 1
#define BROADCAST_PERIOD 1 //seconds

int main( int argc, char *argv[])
{      
   //Port from command line
   int port_sim, port_auto, port_tcas;
   if(argc == 3)
   {
      port_sim = atoi(argv[1]);
      port_auto = atoi(argv[2]);
      port_tcas = TCAS_PORT;
      
   }
   else
   {
      if(argc == 4)
      {
         port_sim = atoi(argv[1]);
         port_auto = atoi(argv[2]);
         port_tcas = atoi(argv[3]);
         
      }   
      else
      {
         port_sim = SIM_PORT;
         port_auto = AUTO_PORT;
         port_tcas = TCAS_PORT;
      }
   }
   Airplane pombo, pombo_tcas, pombo_ap;
    
    // Temporizador
   std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   double t_end, t_start;
   t_start= (double) millis_new /1000 ;
   t_end = t_start;


   //Socket Receive AUTO
   int sockAUTO;
   struct sockaddr_in serv_addr_auto;
   pombo.newSocks(&sockAUTO, &serv_addr_auto, port_auto, false );

   //Socket Receive TCAS
   int sockTCAS;
   struct sockaddr_in serv_addr_tcas;
   pombo.newSocks(&sockTCAS, &serv_addr_tcas, port_tcas, false );

   //Socket Send SIM
   int sockSIM;
   struct sockaddr_in serv_addr_sim;   
   pombo.newSocks(&sockSIM, &serv_addr_sim , port_sim, true);

   // TCAS VEL
   double vE_tcas;
   double vN_tcas;
   double vU_tcas;

   // APILOT VEL
   double vE_apilot;
   double vN_apilot;
   double vU_apilot;

   // AIRPLANE VEL
   double vE_due;
   double vN_due;
   double vU_due;
  
   // GEO Position
   float lat;
   float lon;
   float alt;

   // Retorno
   double signal;

   // Resolving
   bool isResolving;
   char resolving_str[16] = "RESOLVING\0";

   

   while(true)
   {
      //Get Velocities
      // From Autopilot
      pombo_ap.receiveData(sockAUTO);

      

      // Actua com Broadcast Frequency
      now = std::chrono::system_clock::now();
      duration = now.time_since_epoch();
      millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      t_end = (double) millis_new /1000;
      // From TCAS
      if(fabs(t_end-t_start) > BROADCAST_PERIOD)
      {
         pombo_tcas.receiveData(sockTCAS);
         t_start = t_end;         
      }
      // Inicialmente é o estado do piloto automatico
      pombo = pombo_ap;

      // Variaveis que vêm do TCAS sempre
      pombo.intr_id = pombo_tcas.intr_id;
      pombo.resol_value = pombo_tcas.resol_value;
      for(int i=0; i<16; i++)
      {
         pombo.tcas_status[i] = pombo_tcas.tcas_status[i];
         pombo.resolution[i] = pombo_tcas.resolution[i];
      }
     
      // Check is resolving
      for(int i=0; i<8; i++)
      {
         if(resolving_str[i] == pombo.tcas_status[i])
            isResolving=true;
         else
         {
            isResolving= false;
         }
         
      }

      // Caso em RA colocar resolução
      if(pombo_tcas.resol_value != 0 && isResolving)
      {
         
         //Determina se sobe ou desce
         if(pombo_tcas.resolution[0]=='C')
         {
            signal = 1;
         }
         else
         {
            signal = -1;
         }

         // Latitude e Altitude
         wgs_to_geo(pombo_ap.x, pombo_ap.y, pombo_ap.z, &lat, &lon, &alt);

         // Conversao para ENU         
         wgs_to_enu(pombo_ap.vx, pombo_ap.vy, pombo_ap.vz, &vE_apilot, &vN_apilot, &vU_apilot, lat, lon);

         // Velocidade Direcional do Auto Pilot e vertical do TCAS
         vE_due = vE_apilot;
         vN_due = vN_apilot;
         vU_due = pombo_tcas.resol_value * signal;

         // Conversão para WGS + Atribuição
         enu_to_wgs(vE_due, vN_due, vU_due, &pombo.vx, &pombo.vy, &pombo.vz, lat, lon);    

      }
     
      
      //Send to Simulation
      pombo.sendMsg(sockSIM, &serv_addr_sim);    
      
   }

   return 0;
}






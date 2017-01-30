#include "simulator.h"



#define ACCEL_LIMIT 2.4525
#define FASTFORWARD 1

int main( int argc, char *argv[] )
{
    
   int port_2apilot, port_mux;
   if(argc == 3)
   {
      port_mux = atoi(argv[1]);
      port_2apilot = atoi(argv[2]);
   }
   else
   {
      port_mux = PORT_MUX;
      port_2apilot = PORT2APILOT;
   }

   // Temporizador
   std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   

   double dt;
   double t_start, t_end;
   double t_broad_start, t_broad_end;
   double vx, vy, vz;
   double dvx, dvy, dvz;
   double vx1, vy1, vz1;
   double vn, ve, vu;
   double vn1, ve1, vu1;
   float lat, lon, alt;
   double x0, y0, z0;
   double x1, y1, z1;
   double DV2, V1, newDV2;
   
   bool first = true;

   Airplane pombo;

   
   const std::string fileName = "initial_position.path";

   // Variavel para usar ficheiro e abertura
   std::ifstream inf(fileName.c_str());

   // If we couldn't open the output file stream for reading
   if (!inf)
   {
      std::cerr << "Ups, parece que o ficheiro ("<< fileName.c_str() <<") nao pode ser lido!" << std::endl;
      exit(1);
   }

   // Waypoint Inicial = Posição Inicial
   if(inf.is_open())
   {
      double lat, lon, alt, groundspeed;
      inf >> lat;
      inf >> lon;
      inf >> alt;
      inf >> groundspeed;

      // Get Position in WGS
      geo_to_wgs(lat, lon, alt, &x0, &y0, &z0);
      
   }

   //Socket Receive MUX
   int sockMUX;
   struct sockaddr_in serv_addr_mux;
   pombo.newSocks(&sockMUX, &serv_addr_mux, port_mux, false );

   //Socket Send Apilot
   int sockAPILOT;
   struct sockaddr_in serv_addr_apilot;      
   pombo.newSocks(&sockAPILOT, &serv_addr_apilot , port_2apilot, true);


   //Socket Send Broadcast
   int sockBROAD;
   struct sockaddr_in serv_addr_broad;      
   //pombo.newSocks(&sockBROAD, &serv_addr_broad , PORT2BROAD, true);
   //pombo.newSocks(&sockBROAD, &serv_addr_broad , PORT2BROAD, true, "192.168.43.250");
   //pombo.newSocks(&sockBROAD, &serv_addr_broad , PORT2BROAD, true, "194.210.156.114");
   pombo.newSocks(&sockBROAD, &serv_addr_broad , PORT2BROAD, true, ADDRESS_NET);

   // Simulation Timer INIT
   now = std::chrono::system_clock::now();
   duration = now.time_since_epoch();
   millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   t_start= (double) millis_new /1000 ;
   dt = SIMULATION_PERIOD;

   pombo.x=x0;
   pombo.y=y0;
   pombo.z=z0;
   pombo.sendMsg(sockAPILOT, &serv_addr_apilot );
   pombo.sendMsg(sockBROAD, &serv_addr_broad );
   
   // Inicializaçao do timer de broadcast
   now = std::chrono::system_clock::now();
   duration = now.time_since_epoch();
   millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   t_broad_end = (double) millis_new /1000 ;   
   t_broad_start = t_broad_end;

   while(true)
   {      

      //Get Velocity
      pombo.receiveData(sockMUX);
      vx = pombo.vx;
      vy = pombo.vy;
      vz = pombo.vz;

      
      // Latitude e Altitude
      wgs_to_geo(pombo.x, pombo.y, pombo.z, &lat, &lon, &alt);

      // Conversao para ENU         
      wgs_to_enu(pombo.vx, pombo.vy, pombo.vz, &ve, &vn, &vu, lat, lon);
   
      std::cout << vu1 << "<-vu1 " << vu << "<-vu" << std::endl;
      
      if(first)
         first = false;
      else
      {
         if(vu-vu1 > ACCEL_LIMIT)
            vu = vu1 + ACCEL_LIMIT * dt;
         if(vu-vu1 < -ACCEL_LIMIT)
            vu = vu1 - ACCEL_LIMIT * dt;

      }
      
      // Guarda Anterior
      vu1 = vu;

      // Conversão para WGS + Atribuição
      enu_to_wgs(ve, vn, vu, &vx, &vy, &vz, lat, lon);    
      
      // Passa para o airplane
      pombo.vx =  vx;
      pombo.vy =  vy;
      pombo.vz =  vz;
      

      //Compute Position
      x1 = x0 + dt * vx;
      y1 = y0 + dt * vy;
      z1 = z0 + dt * vz;
    
      //Attribution
      pombo.x = x1;
      pombo.y = y1;
      pombo.z = z1;
        
      // Faster Simulation Frequency than TCAS      
      do{
         now = std::chrono::system_clock::now();
         duration = now.time_since_epoch();
         millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
         t_end = (double) millis_new /1000 ;
          
         
      }while(t_end - t_start < SIMULATION_PERIOD*FASTFORWARD);
      
      printf("%f e %f  - %lu\n", t_end, t_start, millis_new);

      // Reset Timer
      now = std::chrono::system_clock::now();
      duration = now.time_since_epoch();
      millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      t_start= (double) millis_new /1000 ;

      printf("Time = %ld\n", millis_new);
   
      //Send Apilot
      pombo.sendMsg(sockAPILOT, &serv_addr_apilot );

      // Time for Broadcast
      now = std::chrono::system_clock::now();
      duration = now.time_since_epoch();
      millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      t_broad_end= (double) millis_new /1000 ;

      //Send Broadcast ( Broadcast Frequency )
      if(t_broad_end - t_broad_start > BROADCAST_PERIOD * FASTFORWARD)      
      {
         pombo.sendMsg(sockBROAD, &serv_addr_broad );
         t_broad_start = t_broad_end; 
      }  
      //Position Renewal
      x0 = x1;
      y0 = y1;
      z0 = z1;
      
   }

    return 0;
}




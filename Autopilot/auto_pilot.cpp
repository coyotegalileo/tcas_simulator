#include "auto_pilot.h"


int main( int argc, char *argv[] )
{
   //Port from command line
   int port_broad, port_2mux;
   if(argc == 3)
   {
      port_2mux = atoi(argv[1]);
      port_broad = atoi(argv[2]);
   }
   else
   {
      port_2mux = PORT2MUX;
      port_broad = PORT_BROAD;
   }

   //Auto pilot Related
   bool stop = false;
   float lat, lon, alt, groundspeed, vertspeed; // para waypoint
   float lat_now, lon_now, alt_now; // só para debug
   double x, y, z; // waypoint em wsg
   Airplane pombo;

   //PARA LINUX
   //Relacionados com o socket
   int sockfd;
   struct sockaddr_in serv_addr;
   pombo.newSocks( &sockfd, &serv_addr, port_2mux, true);

   //Socket Receive BROAD
   int sockBROAD;
   struct sockaddr_in serv_addr_broad;
   pombo.newSocks(&sockBROAD, &serv_addr_broad, port_broad, false);

   //Waypoints File
   const std::string fileName = PATH_FILE;
   // Variavel para usar ficheiro e abertura
   std::ifstream inf(fileName.c_str());

   // If we couldn't open the output file stream for reading
   if (!inf)
   {
      std::cerr << "Ups, parece que o ficheiro ("<< fileName.c_str() <<") nao pode ser lido!" << std::endl;
      exit(1);
   }

   // Waypoint Inicial
   if(inf.is_open())
   {
      inf >> lat;
      inf >> lon;
      inf >> alt;
      inf >> groundspeed;
      inf >> vertspeed;

      // Get Target in WGS
      geo_to_wgs(lat, lon, alt, &x, &y, &z);

   }
     
   while(!stop)
   {                
      // Fetch Position            
      pombo.receiveData(sockBROAD);
       
      // Check Waypoint
      if(pombo.change_target && inf.is_open())
      {
         pombo.change_target = false;
         if(!inf.eof())
         {
            inf >> lat;
            inf >> lon;
            inf >> alt;
            inf >> groundspeed;
            inf >> vertspeed;

            // Get Target in WGS
            geo_to_wgs(lat, lon, alt, &x, &y, &z);
         }
         else
         {
            stop = true;
         }
      }      
      
      // Compute New Velocity
      determine_velocity(&pombo, x, y, z, groundspeed, vertspeed);

      
      // Send to UDP Broadcast
      pombo.sendMsg(sockfd, &serv_addr);
        
      /*---- DEBUG ----*/         
      // Visualize 
      wgs_to_geo(pombo.x, pombo.y, pombo.z, &lat_now, &lon_now, &alt_now);             
      printf("lon: %.4f :: lat: %.4f :: alt: %.2f \n" ,lon_now /_TORAD, lat_now / _TORAD, alt_now);
      
        
   }

   // Fechar socket
   close(sockfd);
   close(sockBROAD);

   //Mensagem de despedida
   printf("\n AIRPLANE FINISHED FLIGHT\n");
   printf("\n THANK YOU FOR FLYING WITH US!\n\n");

}

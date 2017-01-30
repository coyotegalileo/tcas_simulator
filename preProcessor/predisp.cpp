#include "predisp.h"

#define VUP_MAX 30 // m/s
#define GANHO_VUP 1

void enviaDisplay(int sockfd, struct sockaddr_in* sockadd, double vU , int climb);

int main( int argc, char *argv[] )
{      
   Airplane pombo, pombo_tcas, pombo_ap;
   int climb;
   int port_sim, port_disp;
   
    
   if(argc == 3)
   {
      port_sim = atoi(argv[1]);
      port_disp = atoi(argv[2]);
   }
   else
   {
      port_sim = SIM_PORT;
      port_disp = DISP_PORT;
   }
   

   //Socket Receive SIM
   int sockSIM;
   struct sockaddr_in serv_addr_sim;
   pombo.newSocks(&sockSIM, &serv_addr_sim, port_sim, false );

   //Socket Send SIM
   int sockDISP;
   struct sockaddr_in serv_addr_disp;   
   pombo.newSocks(&sockDISP, &serv_addr_disp , port_disp, true);

   // AIRPLANE VEL
   double vE;
   double vN;
   double vU;
  
   // GEO Position
   float lat;
   float lon;
   float alt;
   float groundspeed;

   
   //Log File
   const std::string fileName = "log_navigation.dat";
   // Variavel para usar ficheiro e abertura
   std::ofstream inf(fileName.c_str());

   inf << "\nNew FLIGHT DATA\n\n";

   // If we couldn't open the output file stream for reading
   if (!inf)
   {
      std::cerr << "Ups, parece que o ficheiro ("<< fileName.c_str() <<") nao pode ser lido!" << std::endl;
      exit(1);
   }

   while(true)
   {
      //Get Data From Simulation
      pombo.receiveData(sockSIM);
      
      // Convert to desired
      // Latitude e Altitude
      wgs_to_geo(pombo.x, pombo.y, pombo.z, &lat, &lon, &alt);

      // Conversao para ENU         
      wgs_to_enu(pombo.vx, pombo.vy, pombo.vz, &vE, &vN, &vU, lat, lon);
      groundspeed = std::sqrt(vE*vE + vN*vN)*1.944;
      // Print na Consola
      std::system("clear");
      printf("\t FLIGHT DATA \n\n\n");
      printf("\t GS : %.1f [kn]\t VS : %.1f [feet/min]\n", groundspeed, vU * 196.850393701);
      printf("\n\t Position : LAT [%f] \t LON [%f] \t ALT [%.1f] \n", lat/_TORAD, lon/_TORAD, alt);
      printf("\n\n \t TCAS: STATUS(%s) \t RESOLUTION(%s) \t VALUE(%.1f) ID-%lu IDINT-%lu \n\n", pombo.tcas_status, pombo.resolution,pombo.resol_value,pombo.id ,pombo.intr_id  );

   
      //CLIMB or DESCENT
      if(pombo.resolution[0] == 'C')
         climb = 1;
      else
         if(pombo.resolution[0] == 'D')
            climb = 2;
         else
            climb = 0;


      // Logs
      if(inf.is_open())
      {
         inf << lat;
         inf << "\t";
         inf << lon;
         inf << "\t";
         inf << alt;
         inf << "\t";
         inf << groundspeed;
         inf << "\t";
         inf << vU;
         inf << "\n";

      }      

      //Send to Display
      enviaDisplay(sockDISP, &serv_addr_disp, vU * _MSTOFTMIN, climb);    
      
   }

   return 0;
}

void enviaDisplay(int sockfd, struct sockaddr_in* sockadd, double vU, int climb )
{
 
   /* Structured */

   // Formata a mensagem a enviar
   struct iovec io[2];
   io[0].iov_base = &vU;
   io[0].iov_len = sizeof(vU);
   io[1].iov_base = &climb;
   io[1].iov_len = sizeof(climb);
   
   struct msghdr message;
   message.msg_name=sockadd;
   message.msg_namelen=sizeof(struct sockaddr_in);
   message.msg_iov=io;
   message.msg_iovlen=2;
   message.msg_control=0;
   message.msg_controllen=0;  

   // Envio da mensagem
   if (sendmsg(sockfd,&message,0)==-1) 
   {
       printf("ERROR writing to socket\n\n"); 
   }

}




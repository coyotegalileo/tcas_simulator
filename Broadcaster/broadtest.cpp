#include "airplane.h"

#define BROAD_PORT 10505

int main()
{      
   Airplane pombo;
   std::clock_t time; 
   double time_end, time_start;

   //Socket Send SIM
   int sockSIM;
   struct sockaddr_in serv_addr_sim;   
   //pombo.newSocks(&sockSIM, &serv_addr_sim , BROAD_PORT, true, "192.168.43.250");
   pombo.newSocks(&sockSIM, &serv_addr_sim , BROAD_PORT, true, "194.210.156.114");

   pombo.id=155;

   while(true)
   {
      time = std::clock();
      time_start = time / (CLOCKS_PER_SEC/1000); 
      do 
      {    
      time = std::clock();
      time_end = time / (CLOCKS_PER_SEC/1000); 
      }while(time_end-time_start < 500);
      printf("Sent\n");
    
      //Send to Simulation
      pombo.sendMsg(sockSIM, &serv_addr_sim);    
      
   }

   return 0;
}



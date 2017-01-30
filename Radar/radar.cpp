#include "radar.h"
#include <vector>
#include <chrono>
#define TCAS_RATE 0.99

int verificaID(Airplane pombo, std::vector< Airplane > airspace, int size);

int main( int argc, char *argv[] )
{

   //Port from command line
   int port_tcas;
   if(argc == 2)
   {
      port_tcas = atoi(argv[1]);      
   }
   else
   {
      port_tcas = PORT_OUT;      
   }


   std::string address2broad;
   if(argc == 1)
   {
      address2broad = argv[0];
   }
   else
   {
      address2broad = ADDRESS_NET;
   }
   // Lista
   //Airplane* airspace;
   std::vector< Airplane > airspace;

   Airplane pombo;
   int list_size = 0;
   int index_lista;

   // Temporizador
   std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis_old = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   auto millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

   double time_pre, time_pos;   

   //PARA LINUX
   //Relacionados com o socket
   // Socket para enviar a lista
   int sockfd;
   struct sockaddr_in serv_addr;
   pombo.newSocks( &sockfd, &serv_addr, port_tcas, true);

   //Socket Receive BROAD
   int sockBROADCAST;
   struct sockaddr_in serv_addr_broad;
   //pombo.newSocks(&sockBROADCAST, &serv_addr_broad, PORT_BROADCAST, false);
   //pombo.newSocks(&sockBROADCAST, &serv_addr_broad, PORT_BROADCAST, false, "192.168.43.250");
   //pombo.newSocks(&sockBROADCAST, &serv_addr_broad, PORT_BROADCAST, false, "194.210.156.114");
   //pombo.newSocks(&sockBROADCAST, &serv_addr_broad, PORT_BROADCAST, false, ADDRESS_NET);
   pombo.newSocks(&sockBROADCAST, &serv_addr_broad, PORT_BROADCAST, false, address2broad.c_str());

   time_pre = (double) millis_old/1000;     

   while(true)
   {    
                  
      // Recebe dados do broadcast
      pombo.receiveData(sockBROADCAST);
      
              

      // Verifica se avião está na lista
      index_lista = -1;
      index_lista = verificaID(pombo, airspace, list_size);

         
      
      if(index_lista > -1)
      {
         // Actualiza Elemento
         airspace[index_lista] = pombo;
      }
      else
      {
          
         // Adiciona à lista
         airspace.push_back(pombo);         
         list_size ++;         
      }

      
      // Envia lista actual (1 Hz)
      now = std::chrono::system_clock::now();
      duration = now.time_since_epoch();
      millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      time_pos = (double) millis_new /1000;

      //printf("\n %lu %lu \n", millis_new, millis_old);

      if(fabs(time_pos-time_pre) > TCAS_RATE)
      {      
         pombo.sendMsgList(sockfd, &serv_addr, airspace, list_size);
         time_pre = time_pos;
      }
      
      // DEBUG
      printf("Tamanho - %d\n", list_size);
      
   }

   // Fechar socket
   close(sockfd);
   close(sockBROADCAST);

   //Mensagem de despedida
   printf("\n AIRPLANE FINISHED FLIGHT\n");
   printf("\n THANK YOU FOR FLYING WITH US!\n\n");

}

int verificaID(Airplane pombo, std::vector< Airplane > airspace, int size)
{
   int index;
   
   for(index=0;index<size;index++)
   {
      if(airspace[index].id == pombo.id)
      {
         return index;
      }
   }
   
   // Not on List
   return -1;
}


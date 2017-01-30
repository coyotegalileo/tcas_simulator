#include "recebe.h"
#include <vector>



int main()
{

   // Lista
   //Airplane* airspace;
   std::vector< Airplane > airspace;

   Airplane pombo;
   int list_size = 0;
   int i;

   //PARA LINUX
   //Relacionados com o socket
   int sockfd;
   struct sockaddr_in serv_addr;
   pombo.newSocks( &sockfd, &serv_addr, PORT_OUT, false);

      
     
   while(true)
   {                
      // Recebe dados do broadcast
      pombo.receiveDataList(sockfd, airspace);

      printf(" bacalhau\n");

      // DEBUG
      printf("Tamanho - %ld\n", airspace.size() );
      for(i=0;i<airspace.size();i++)
      {
         printf("ID - %ld\n", airspace[i].id );
      }
      
      
   }

   // Fechar socket
   close(sockfd);


   //Mensagem de despedida
   printf("\n AIRPLANE FINISHED FLIGHT\n");
   printf("\n THANK YOU FOR FLYING WITH US!\n\n");

}



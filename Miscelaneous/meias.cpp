#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <sys/uio.h>
#include <iomanip>


void newSocks(int* sockback, struct sockaddr_in* sockadd_back, int portno, bool send, const char * specific_addr = "Nope" )
{
   // Estruturas dos Sockets
   struct sockaddr_in serv_addr;
   struct hostent *server;
   int sockfd;
   int so_broadcast;
   int so_reuseaddr;
   

   // Receive
   socklen_t clilen;
   struct sockaddr_in cli_addr;   

   //Socket creation
   sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

   
   so_broadcast = 1;
   so_reuseaddr = 1;
   // Configure Socket (enable broadcasting)
   setsockopt( sockfd, 
               SOL_SOCKET, 
               SO_BROADCAST,
               &so_broadcast, 
               sizeof(so_broadcast));
   // Configure Socket (enable adressreuse)
   setsockopt( sockfd, 
               SOL_SOCKET, 
               SO_REUSEADDR,
               &so_reuseaddr, 
               sizeof(so_reuseaddr));
   

   // Configure socckaddr
   bzero((char *) &serv_addr, sizeof(serv_addr));
   // Family
   serv_addr.sin_family = AF_INET;
   // Adress
    if(send)
   {
      // Get Address
      server = gethostbyname("localhost");
      if (server == NULL) {
         fprintf(stderr,"ERROR, no such host\n");
         exit(0);
      }      
      //bcopy((char *)server->h_addr,  
      //      (char *)&serv_addr.sin_addr.s_addr, 
      //      server->h_length);

      // Caso address especifico
      if(strcmp(specific_addr,"Nope") !=0 )
      {         
         serv_addr.sin_addr.s_addr = inet_addr(specific_addr);
      }
      else
      {
         serv_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
      }
   }
   else
   {
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   }
   
   // Port 
   serv_addr.sin_port = htons(portno);

   // Connect or Bind
   if(send)
   {
      if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
         error("ERROR connecting");
   }
   else
   {
      if (bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
         error("ERROR connecting");
   }

   // Return
   *sockback = sockfd;
   *sockadd_back = serv_addr;

}


void receiveData(int sockBROAD, double * vU, bool * climb)
{
   struct sockaddr_storage src_addr;

   /*Structured*/
   
   // Formato da mensagem
   struct iovec io[2];
   io[0].iov_base = &vU;
   io[0].iov_len = sizeof(vU);
   io[1].iov_base = &climb;
   io[1].iov_len = sizeof(climb);
   

   struct msghdr message;
   message.msg_name=&src_addr;
   message.msg_namelen=sizeof(src_addr);
   message.msg_iov=io;
   message.msg_iovlen=2;
   message.msg_control=0;
   message.msg_controllen=0;

   // receção de mensagens           
   ssize_t count=recvmsg(sockBROAD,&message,0);
   if (count==-1) {             
       error("ERROR reading from socket");
   } else if (message.msg_flags&MSG_TRUNC) {
       printf("datagram too large for buffer: truncated\n");
   } else {
           

   }  
}






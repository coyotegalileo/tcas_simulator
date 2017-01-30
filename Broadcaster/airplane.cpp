#include "airplane.h"

#define MESSAGE_SIZE 124

Airplane::Airplane()
{
   int i;
   char pre_header[16] = "ACIP TCAS  V01\0";
   for (i=0;i<16;i++)
   {
      header[i] = pre_header[i];
   }
   
   id = AIRPLANE_ID;

   x = 0;
   y = 0;
   z = 0;

   vx = 0;
   vy = 0;
   vz = 0;

   char pre_status[16] = "CLEAR\0";
   for (i=0;i<16;i++)
   {
      tcas_status[i] = pre_status[i];
   }

   intr_id = 0;
   resolution[0] = '\0';
   resol_value = 0;

}

Airplane::Airplane(double x0, double y0, double z0,double vx0,double vy0, double vz0)
{
   int i;
   char pre_header[16] = "ACIP TCAS  V01\0";
   for (i=0;i<16;i++)
   {
      header[i] = pre_header[i];
   }
   id = AIRPLANE_ID;

   x = x0;
   y = y0;
   z = z0;

   vx = vx0;
   vy = vy0;
   vz = vz0;

   
   char pre_status[16] = "CLEAR\0";
   for (i=0;i<16;i++)
   {
      tcas_status[i] = pre_status[i];
   }
   intr_id = 0;
   resolution[0] = '\0';
   resol_value = 0;  
}

Airplane::~Airplane()
{
   //dtor
}

void Airplane::sendMsg(int sockfd, struct sockaddr_in* sockadd )
{
 
   int n;
   char buffer[256];

   /* Structured */

   // Formata a mensagem a enviar
   struct iovec io[13];
   io[0].iov_base = header;
   io[0].iov_len = sizeof(header);
   io[1].iov_base = &id;
   io[1].iov_len = sizeof(id);
   io[2].iov_base = &x;
   io[2].iov_len = sizeof(x);
   io[3].iov_base = &y;
   io[3].iov_len = sizeof(y);
   io[4].iov_base = &z;
   io[4].iov_len = sizeof(z);
   io[5].iov_base = &vx;
   io[5].iov_len = sizeof(vx);
   io[6].iov_base = &vy;
   io[6].iov_len = sizeof(vy);
   io[7].iov_base = &vz;
   io[7].iov_len = sizeof(vz);
   io[8].iov_base = tcas_status;
   io[8].iov_len = sizeof(tcas_status);
   io[9].iov_base = &intr_id;
   io[9].iov_len = sizeof(intr_id);
   io[10].iov_base = resolution;
   io[10].iov_len = sizeof(resolution);
   io[11].iov_base = &resol_value;
   io[11].iov_len = sizeof(resol_value);

   // TODO fazer calculo do checksum 
   crc = 0;
   io[12].iov_base = &crc;
   io[12].iov_len = sizeof(crc);
   
   //printf("%d", crc);


   struct msghdr message;
   message.msg_name=sockadd;
   message.msg_namelen=sizeof(struct sockaddr_in);
   message.msg_iov=io;
   message.msg_iovlen=13;
   message.msg_control=0;
   message.msg_controllen=0;  


   if (sendmsg(sockfd,&message,0)==-1) 
   {
       printf("ERROR writing to socket\n\n"); 
   }

}


void Airplane::sendMsgList(int sockfd, struct sockaddr_in* sockadd, std::vector< Airplane > airspace,  int list_size )
{
 
   int n;
   int i, offset;
   char buffer[256];

   /* Structured */

   // Formata a mensagem a enviar
   struct iovec io[13*list_size];
   for(i=0;i<list_size;i++)
   {
      offset = i *13;

      io[0 + offset].iov_base = airspace[i].header;
      io[0 + offset].iov_len = sizeof(airspace[i].header);
      io[1 + offset].iov_base = &airspace[i].id;
      io[1 + offset].iov_len = sizeof(airspace[i].id);
      io[2 + offset].iov_base = &airspace[i].x;
      io[2 + offset].iov_len = sizeof(airspace[i].x);
      io[3 + offset].iov_base = &airspace[i].y;
      io[3 + offset].iov_len = sizeof(airspace[i].y);
      io[4 + offset].iov_base = &airspace[i].z;
      io[4 + offset].iov_len = sizeof(airspace[i].z);
      io[5 + offset].iov_base = &airspace[i].vx;
      io[5 + offset].iov_len = sizeof(airspace[i].vx);
      io[6 + offset].iov_base = &airspace[i].vy;
      io[6 + offset].iov_len = sizeof(airspace[i].vy);
      io[7 + offset].iov_base = &airspace[i].vz;
      io[7 + offset].iov_len = sizeof(airspace[i].vz);
      io[8 + offset].iov_base = airspace[i].tcas_status;
      io[8 + offset].iov_len = sizeof(airspace[i].tcas_status);
      io[9 + offset].iov_base = &airspace[i].intr_id;
      io[9 + offset].iov_len = sizeof(airspace[i].intr_id);
      io[10 + offset].iov_base = airspace[i].resolution;
      io[10 + offset].iov_len = sizeof(airspace[i].resolution);
      io[11 + offset].iov_base = &airspace[i].resol_value;
      io[11 + offset].iov_len = sizeof(airspace[i].resol_value);

      // TODO fazer calculo do checksum 
      airspace[i].crc = 0;
      io[12 + offset].iov_base = &airspace[i].crc;
      io[12 + offset].iov_len = sizeof(airspace[i].crc);

   }
   

   
  


   struct msghdr message;
   message.msg_name=sockadd;
   message.msg_namelen=sizeof(struct sockaddr_in);
   message.msg_iov=io;
   message.msg_iovlen=13*list_size;
   message.msg_control=0;
   message.msg_controllen=0;  


   if (sendmsg(sockfd,&message,0)==-1) 
   {
       printf("ERROR writing to socket\n\n"); 
   }

}





void Airplane::newSocks(int* sockback, struct sockaddr_in* sockadd_back, int portno, bool send, const char * specific_addr )
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
         serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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




void Airplane::receiveData(int sockBROAD)
{
   struct sockaddr_storage src_addr;

   /*Structured*/
   
   // Formato da mensagem
   struct iovec io[13];
   io[0].iov_base = header;
   io[0].iov_len = sizeof(header);
   io[1].iov_base = &id;
   io[1].iov_len = sizeof(id);
   io[2].iov_base = &x;
   io[2].iov_len = sizeof(x);
   io[3].iov_base = &y;
   io[3].iov_len = sizeof(y);
   io[4].iov_base = &z;
   io[4].iov_len = sizeof(z);
   io[5].iov_base = &vx;
   io[5].iov_len = sizeof(vx);
   io[6].iov_base = &vy;
   io[6].iov_len = sizeof(vy);
   io[7].iov_base = &vz;
   io[7].iov_len = sizeof(vz);
   io[8].iov_base = tcas_status;
   io[8].iov_len = sizeof(tcas_status);
   io[9].iov_base = &intr_id;
   io[9].iov_len = sizeof(intr_id);
   io[10].iov_base = resolution;
   io[10].iov_len = sizeof(resolution);
   io[11].iov_base = &resol_value;
   io[11].iov_len = sizeof(resol_value);
   io[12].iov_base = &crc;
   io[12].iov_len = sizeof(crc);

   struct msghdr message;
   message.msg_name=&src_addr;
   message.msg_namelen=sizeof(src_addr);
   message.msg_iov=io;
   message.msg_iovlen=13;
   message.msg_control=0;
   message.msg_controllen=0;

   // receção de mensagens           
   ssize_t count=recvmsg(sockBROAD,&message,0);
   if (count==-1) {             
       error("ERROR reading from socket");
   } else if (message.msg_flags&MSG_TRUNC) {
       printf("datagram too large for buffer: truncated\n");
   } else {

      printf("\nHeader = %s \n",header);
      printf("ID = %lu", id);
      printf("x = %f y = %f z = %f\n", x,y,z);
      printf("vx = %f vy = %f vz = %f\n",vx,vy,vz);
      printf("STATUS = %s Intruder = %lu Resolution = %s Value=%f \n",tcas_status, intr_id, resolution,resol_value);
      printf("Checksum = %u \n ", crc);
   }  
}


void Airplane::receiveDataList(int sockBROAD, std::vector< Airplane >& real_airspace)
{
   struct sockaddr_storage src_addr;
   

   int i, offset;
   // Limite de Aeronaves
   int list_size = 30; 
   // Vector Buffer
   std::vector< Airplane > airspace(list_size);
   std::vector< Airplane > reducted_airspace;
  
   

   // Formata a mensagem a enviar
   struct iovec io[13*list_size];
   for(i=0;i<list_size;i++)
   {
      offset = i *13;

      io[0 + offset].iov_base = airspace[i].header;
      io[0 + offset].iov_len = sizeof(airspace[i].header);
      io[1 + offset].iov_base = &airspace[i].id;
      io[1 + offset].iov_len = sizeof(airspace[i].id);
      io[2 + offset].iov_base = &airspace[i].x;
      io[2 + offset].iov_len = sizeof(airspace[i].x);
      io[3 + offset].iov_base = &airspace[i].y;
      io[3 + offset].iov_len = sizeof(airspace[i].y);
      io[4 + offset].iov_base = &airspace[i].z;
      io[4 + offset].iov_len = sizeof(airspace[i].z);
      io[5 + offset].iov_base = &airspace[i].vx;
      io[5 + offset].iov_len = sizeof(airspace[i].vx);
      io[6 + offset].iov_base = &airspace[i].vy;
      io[6 + offset].iov_len = sizeof(airspace[i].vy);
      io[7 + offset].iov_base = &airspace[i].vz;
      io[7 + offset].iov_len = sizeof(airspace[i].vz);
      io[8 + offset].iov_base = airspace[i].tcas_status;
      io[8 + offset].iov_len = sizeof(airspace[i].tcas_status);
      io[9 + offset].iov_base = &airspace[i].intr_id;
      io[9 + offset].iov_len = sizeof(airspace[i].intr_id);
      io[10 + offset].iov_base = airspace[i].resolution;
      io[10 + offset].iov_len = sizeof(airspace[i].resolution);
      io[11 + offset].iov_base = &airspace[i].resol_value;
      io[11 + offset].iov_len = sizeof(airspace[i].resol_value);
      io[12 + offset].iov_base = &airspace[i].crc;
      io[12 + offset].iov_len = sizeof(airspace[i].crc);

   }

   struct msghdr message;
   message.msg_name=&src_addr;
   message.msg_namelen=sizeof(src_addr);
   message.msg_iov=io;
   message.msg_iovlen=13*list_size;
   message.msg_control=0;
   message.msg_controllen=0;

   // receção de mensagens           
   ssize_t count=recvmsg(sockBROAD,&message,0);
   if (count==-1) {             
       error("ERROR reading from socket");
   } else if (message.msg_flags&MSG_TRUNC) {
       printf("datagram too large for buffer: truncated\n");
   } else {
      Airplane aux;
      // Determina numero de avioes na lista
      int n_aeroplanes = count / 124;

      printf("Recebi: %d bananos\n", n_aeroplanes);



      for(i=0;i<n_aeroplanes;i++)
      {
         aux = airspace[i];
         
         reducted_airspace.push_back(aux);
         
      }
      
   }  
   real_airspace = reducted_airspace;
   
         
}



void error(const char *msg)
{
    perror(msg);
    exit(0);
}


void recept(int sockBROAD, std::vector< to_display >& real_airspace)
{
   struct sockaddr_storage src_addr;
   

   int i, offset;
   // Limite de Aeronaves
   int list_size = 30; 
  
   // Formata a mensagem a enviar
   struct iovec io[5*list_size];
   for(i=0;i<list_size;i++)
   {
      offset = i *5;

  
     io[0 + offset].iov_base = &airspace[i].d_altitude;
	  io[0 + offset].iov_len = sizeof(airspace[i].d_altitude);
	  io[1 + offset].iov_base = &airspace[i].bearing;
	  io[1 + offset].iov_len = sizeof(airspace[i].bearing);
	  io[2 + offset].iov_base = &airspace[i].distance;
	  io[2 + offset].iov_len = sizeof(airspace[i].distance);
	  io[3 + offset].iov_base = &airspace[i].v_speed;
	  io[3 + offset].iov_len = sizeof(airspace[i].v_speed);
	  io[4 + offset].iov_base = &airspace[i].type;
	  io[4 + offset].iov_len = sizeof(airspace[i].type); 

   }

   struct msghdr message;
   message.msg_name=&src_addr;
   message.msg_namelen=sizeof(src_addr);
   message.msg_iov=io;
   message.msg_iovlen=5*list_size;
   message.msg_control=0;
   message.msg_controllen=0;

   // receção de mensagens           
   ssize_t count=recvmsg(sockBROAD,&message,0);
   if (count==-1) {             
       error("ERROR reading from socket");
   } else if (message.msg_flags&MSG_TRUNC) {
       printf("datagram too large for buffer: truncated\n");
   } else {
      
      int n_aeroplanes = count / 36;

      printf("Recebi: %d \n", n_aeroplanes);



      //for(i=0;i<n_aeroplanes;i++)
      //{
      //   aux = airspace[i];
         
      //   reducted_airspace.push_back(aux);
         
      //}
      
   }  
   //real_airspace = reducted_airspace;
   
         
}


void enviacarago(int sockfd, struct sockaddr_in* sockadd, std::vector< int > airspace,  int list_size )
{
 
   int n;
   int i, offset;
   char buffer[256];

   double e = 0;
   int u =1;
   /* Structured */

   // Formata a mensagem a enviar
   struct iovec io[5*list_size];
   for(i=0;i<list_size;i++)
   {
      offset = i *5;

     io[0 + offset].iov_base = &e;
	  io[0 + offset].iov_len = sizeof(e);
	  io[1 + offset].iov_base = &e;
	  io[1 + offset].iov_len = sizeof(e);
	  io[2 + offset].iov_base = &e;
	  io[2 + offset].iov_len = sizeof(e);
	  io[3 + offset].iov_base = &e;
	  io[3 + offset].iov_len = sizeof(e);
	  io[4 + offset].iov_base = &u;
	  io[4 + offset].iov_len = sizeof(u); 

   }
   


   struct msghdr message;
   message.msg_name=sockadd;
   message.msg_namelen=sizeof(struct sockaddr_in);
   message.msg_iov=io;
   message.msg_iovlen=5*list_size;
   message.msg_control=0;
   message.msg_controllen=0;  


   if (sendmsg(sockfd,&message,0)==-1) 
   {
       printf("ERROR writing to socket\n\n"); 
   }

}



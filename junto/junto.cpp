
/**********/
/* Do auto pilot */
#include "auto_pilot.h"

/**********/
/* Do simulador */
#include "simulator.h"

#define ACCEL_LIMIT 2.4525
#define FASTFORWARD 1
#define PORT2PRE 8128

/**********/
/* Do mux */
#include "mux.h"

#define VUP_MAX 30 // m/s
#define GANHO_VUP 1
#define BROADCAST_PERIOD 1 //seconds

/**********/
/* Do predisp */
#include "predisp.h"

#define VUP_MAX 30 // m/s
#define GANHO_VUP 1


int contador = 0;

int autopilot( Airplane * pombo, double x, double y, double z, float groundspeed, float vertspeed )
{

   //float lat, lon, alt, groundspeed, vertspeed; // para waypoint
   float lat_now, lon_now, alt_now; // só para debug

   // Compute New Velocity
   determine_velocity(pombo, x, y, z, groundspeed, vertspeed);
   
   /*---- DEBUG ----*/         
   // Visualize 
   wgs_to_geo(pombo->x, pombo->y, pombo->z, &lat_now, &lon_now, &alt_now);             
   printf("lon: %.4f :: lat: %.4f :: alt: %.2f \n" ,lon_now /_TORAD, lat_now / _TORAD, alt_now);

}


int simulador( Airplane * pombo, bool first )
{
   double dt;  
   double vx, vy, vz;
   double vn, ve, vu;
   //double vn1, ve1, vu1;
   float lat, lon, alt;
   //double x0, y0, z0;
   double x1, y1, z1;
   
   dt = BROADCAST_PERIOD;

   // Velocidades      
   vx = pombo->vx;
   vy = pombo->vy;
   vz = pombo->vz;
  
   // Latitude e Altitude
   wgs_to_geo(pombo->x, pombo->y, pombo->z, &lat, &lon, &alt);

   // Conversao para ENU         
   wgs_to_enu(pombo->vx, pombo->vy, pombo->vz, &ve, &vn, &vu, lat, lon);

   // Controlo da aceleração vertical   
   if(first)
      first = false;
   else
   {
      if(vu - pombo->vu0 > ACCEL_LIMIT)
         vu = pombo->vu0 + ACCEL_LIMIT * dt;
      if(vu - pombo->vu0 < -ACCEL_LIMIT)
         vu = pombo->vu0 - ACCEL_LIMIT * dt;

   }
      
   // Guarda Anterior
   pombo->vu0 = vu;

   // Conversão para WGS + Atribuição
   enu_to_wgs(ve, vn, vu, &vx, &vy, &vz, lat, lon);    
   
   // Passa para o airplane
   pombo->vx =  vx;
   pombo->vy =  vy;
   pombo->vz =  vz;

   //Compute Position
   x1 = pombo->x0 + dt * vx;
   y1 = pombo->y0 + dt * vy;
   z1 = pombo->z0 + dt * vz;
 
   //Attribution
   pombo->x = x1;
   pombo->y = y1;
   pombo->z = z1;
           
   //Position Renewal
   pombo->x0 = x1;
   pombo->y0 = y1;
   pombo->z0 = z1;
      
    return 0;
}




int mux( Airplane * pombo, Airplane * pombo_tcas)
{      
  
    // Temporizador
   std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   double t_end, t_start;
   t_start= (double) millis_new /1000 ;
   t_end = t_start;

   
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
   bool isReturning; 
   bool isClear;
   bool isAdvisory;
   char clear_str[16] = "CLEAR\0";
   char advisory_str[16] = "ADVISORY\0";  
   char resolving_str[16] = "RESOLVING\0";  
   char returning_str[16] = "RETURNING\0";   

      
   // Variaveis que vêm do TCAS sempre
   pombo->intr_id = pombo_tcas->intr_id;
   pombo->resol_value = pombo_tcas->resol_value;
   for(int i=0; i<16; i++)
   {
      pombo->tcas_status[i] = pombo_tcas->tcas_status[i];
      pombo->resolution[i] = pombo_tcas->resolution[i];
   }
  
   // Check is advisory
   for(int i=0; i<8; i++)
   {
      if(advisory_str[i] == pombo->tcas_status[i])
         isAdvisory=true;
      else
      {
         isAdvisory= false;
      }
      
   }

    // Check is  resolving
   for(int i=0; i<8; i++)
   {
      if(resolving_str[i] == pombo->tcas_status[i])
         isResolving=true;
      else
      {
         isResolving= false;
      }
      
   }

   // Check is clear
   for(int i=0; i<3; i++)
   {
      if(clear_str[i] == pombo->tcas_status[i])
         isClear=true;
      else
      {
         isClear = false;
      }
      
   }

    // Check is returning
   for(int i=0; i<6; i++)
   {
      if(returning_str[i] == pombo->tcas_status[i])
      {
         isReturning=true;
      }
      else
      {
         isReturning= false;
      }      
   }


   // Caso em RA colocar resolução
   if(pombo_tcas->resol_value != 0 && isResolving)
   {
      
      contador = 0;
      //Determina se sobe ou desce
      if(pombo_tcas->resolution[0]=='C')
      {
         signal = 1;
      }
      else
      {
         signal = -1;
      }

      // Latitude e Altitude
      wgs_to_geo(pombo->x, pombo->y, pombo->z, &lat, &lon, &alt);

      // Conversao para ENU         
      wgs_to_enu(pombo->vx, pombo->vy, pombo->vz, &vE_apilot, &vN_apilot, &vU_apilot, lat, lon);

      // Velocidade Direcional do Auto Pilot e vertical do TCAS
      vE_due = vE_apilot;
      vN_due = vN_apilot;
      vU_due = pombo_tcas->resol_value * signal;

      // Conversão para WGS + Atribuição
      enu_to_wgs(vE_due, vN_due, vU_due, &pombo->vx, &pombo->vy, &pombo->vz, lat, lon);    

   }      

   if(isReturning)
	contador++;

   if (isClear || isAdvisory)
   {
      if(contador > 0 && contador < 60)
         contador++;
      else
         contador = 0;
   }

     // Return
   if(contador > 0 )
   {
 
      // Latitude e Altitude
      wgs_to_geo(pombo->x, pombo->y, pombo->z, &lat, &lon, &alt);

      // Conversao para ENU         
      wgs_to_enu(pombo->vx, pombo->vy, pombo->vz, &vE_apilot, &vN_apilot, &vU_apilot, lat, lon);

      if(vU_apilot < 0)
         signal = -1;
      else
         signal = 1;

      // Velocidade Direcional do Auto Pilot e vertical do TCAS
      vE_due = vE_apilot;
      vN_due = vN_apilot;

	  if(contador < 5){
	      	vU_due = -signal*1200/_MSTOFTMIN;
	      }else if(contador < 10){
		vU_due = -signal*(1000/_MSTOFTMIN);
	      } else if(contador < 30){
		vU_due = -signal*500/_MSTOFTMIN;
	      } else{
		vU_due = 0;
	      }

	std::cout << contador << " : vUDUE ---------------- "<<vU_due << std::endl; 

      // Conversão para WGS + Atribuição
      enu_to_wgs(vE_due, vN_due, vU_due, &pombo->vx, &pombo->vy, &pombo->vz, lat, lon);    

   }

          

   return 0;
}


void enviaDisplay(int sockfd, struct sockaddr_in* sockadd, double vU , int climb);

int predisp( Airplane pombo , int sockDISP, struct sockaddr_in* serv_addr_disp)
{      
   int climb;

   // AIRPLANE VEL
   double vE;
   double vN;
   double vU;
  
   // GEO Position
   float lat;
   float lon;
   float alt;
   float groundspeed;

   // Convert to desired
   // Latitude e Altitude
   wgs_to_geo(pombo.x, pombo.y, pombo.z, &lat, &lon, &alt);

   // Conversao para ENU         
   wgs_to_enu(pombo.vx, pombo.vy, pombo.vz, &vE, &vN, &vU, lat, lon);
   groundspeed = std::sqrt(vE*vE + vN*vN)*1.944;

   // Print na Consola
   //std::system("clear");
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


   //Send to Display
   enviaDisplay(sockDISP, serv_addr_disp, vU * _MSTOFTMIN, climb);    
      
  

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





int main( int argc, char *argv[] )
{

   // Recebe Ports
   int  port_tcas, port_disp;
   if(argc == 3)
   {
      port_disp = atoi(argv[1]);
      port_tcas = atoi(argv[2]);
   }
   else
   {
      port_disp = DISP_PORT;
      port_tcas = TCAS_PORT;
   }


   // Nosso Avião
   Airplane pombo;
   // DO TCAS
   Airplane pombo_tcas;

   // Posicao inicial
   double lat, lon, alt;

   // Waypoints
   double xw, yw, zw, groundspeed, vertspeed;

   // Temporizador
   std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

   // Tempos
   double dt;
   double t_start, t_end;
   double t_broad_start, t_broad_end;
   bool first = true;

   //Socket Send Predisp
   int sockDISP;
   struct sockaddr_in serv_addr_disp;      
   pombo.newSocks(&sockDISP, &serv_addr_disp , port_disp, true);

   //Socket Receive TCAS
   int sockTCAS;
   struct sockaddr_in serv_addr_tcas;
   pombo.newSocks(&sockTCAS, &serv_addr_tcas, port_tcas, false );

   //Socket Send Broadcast
   int sockBROAD;
   struct sockaddr_in serv_addr_broad;  
   pombo.newSocks(&sockBROAD, &serv_addr_broad , PORT2BROAD, true, ADDRESS_NET);

   
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
      geo_to_wgs(lat, lon, alt, &pombo.x0, &pombo.y0, &pombo.z0);
      
   }

   // Ficheiro dos Waypoints
   const std::string fileName_W = PATH_FILE;
   // Variavel para usar ficheiro e abertura
   std::ifstream inf_w(fileName_W.c_str());

   // If we couldn't open the output file stream for reading
   if (!inf_w)
   {
      std::cerr << "Ups, parece que o ficheiro ("<< fileName.c_str() <<") nao pode ser lido!" << std::endl;
      exit(1);
   }
   // Waypoint Inicial
   if(inf_w.is_open())
   {
      inf_w >> lat;
      inf_w >> lon;
      inf_w >> alt;
      inf_w >> groundspeed;
      inf_w >> vertspeed;

      // Get Target in WGS
      geo_to_wgs(lat, lon, alt, &xw, &yw, &zw);
   }

   
   // Simulation Timer INIT
   now = std::chrono::system_clock::now();
   duration = now.time_since_epoch();
   millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   t_start= (double) millis_new /1000 ;
   dt = SIMULATION_PERIOD;

   // Inicializaçao do timer de broadcast
   now = std::chrono::system_clock::now();
   duration = now.time_since_epoch();
   millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   t_broad_end = (double) millis_new /1000 ;   
   t_broad_start = t_broad_end;


   // Posição inicial
   pombo.x=pombo.x0;
   pombo.y=pombo.y0;
   pombo.z=pombo.z0;
   


   
   //Iniciar Ciclo Aqui
   while(true)
   {

   
      /**  Mensagem de Broadcast + TCAS (1 Hz)  **/

      // Time for Broadcast
      now = std::chrono::system_clock::now();
      duration = now.time_since_epoch();
      millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      t_broad_end= (double) millis_new /1000 ;
      //Send Broadcast ( Broadcast Frequency )
      if(t_broad_end - t_broad_start > BROADCAST_PERIOD * FASTFORWARD)      
      {
         
         pombo.sendMsg(sockBROAD, &serv_addr_broad );
         std::cout << "Waiting for message\n";
         pombo_tcas.receiveData(sockTCAS);
         t_broad_start = t_broad_end; 
      } 

      /************************/



      /**Leitura de Waypoints**/

       // Check Waypoint
      if(pombo.change_target && inf_w.is_open())
      {
         pombo.change_target = false;
         if(!inf_w.eof())
         {
            inf_w >> lat;
            inf_w >> lon;
            inf_w >> alt;
            inf_w >> groundspeed;
            inf_w >> vertspeed;

            // Get Target in WGS
            geo_to_wgs(lat, lon, alt, &xw, &yw, &zw);
         }
         else
         {
            break;
         }
   }
   

   /************************/


   // Piloto
   autopilot( &pombo, xw, yw, zw,  groundspeed, vertspeed);
   // Mux
   mux( &pombo, &pombo_tcas);


   // Espera Tempo Real 
   do{
         now = std::chrono::system_clock::now();
         duration = now.time_since_epoch();
         millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
         t_end = (double) millis_new /1000 ;
                   
   }while(t_end - t_start < SIMULATION_PERIOD*FASTFORWARD);

   // Reset Timer
      now = std::chrono::system_clock::now();
      duration = now.time_since_epoch();
      millis_new = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      t_start= (double) millis_new /1000 ;

   // Simulador
   simulador( &pombo, first);
   // Envia para display
   predisp(  pombo ,sockDISP, &serv_addr_disp);
      
   first = false;

   }
   //end cycle

   // Close sockets   

   //Mensagem de despedida
   printf("\n AIRPLANE FINISHED FLIGHT\n");
   printf("\n THANK YOU FOR FLYING WITH US!\n\n");

   return 0;
}







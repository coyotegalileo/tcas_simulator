#include "TCAS.h"
#include <math.h>


using namespace std;

to_display::to_display()
{
	d_altitude = 0;
	bearing = 0;
	distance = 0;
	v_speed = 0;
	type = 0;
}

to_display::to_display(double d_alt, double bea, double dist,double v_sp,int t)
{
	d_altitude = d_alt;
	bearing = bea;
	distance = dist;
	v_speed = v_sp;
	type = t;
}

/* LOOk ing for answers*/
void enviacarago(int sockfd, struct sockaddr_in* sockadd, double* d_alt, double* bea, double* dist, double* v_sp, int* t,  int list_size )
{
 
   int n;
   int i, offset;
   char buffer[256];

   /* Structured */

   // Formata a mensagem a enviar
   struct iovec io[5*list_size];
   for(i=0;i<list_size;i++)
   {
      offset = i *5;

     io[0 + offset].iov_base = &d_alt[i];
	  io[0 + offset].iov_len = sizeof(d_alt[i]);
	  io[1 + offset].iov_base = &bea[i];
	  io[1 + offset].iov_len = sizeof(bea[i]);
	  io[2 + offset].iov_base = &dist[i];
	  io[2 + offset].iov_len = sizeof(dist[i]);
	  io[3 + offset].iov_base = &v_sp[i];
	  io[3 + offset].iov_len = sizeof(v_sp[i]);
	  io[4 + offset].iov_base = &t[i];
	  io[4 + offset].iov_len = sizeof(t[i]); 

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


void to_display::newSocks(int* sockback, struct sockaddr_in* sockadd_back, int portno, bool send, const char * specific_addr )
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
         serv_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
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

void to_display::sendMsgList(int sockfd, struct sockaddr_in* sockadd, std::vector< to_display > display_l)
{
	int offset;

   int list_size = (int) display_l.size();
	/* Structured */

	// Formata a mensagem a enviar
	struct iovec io[5*list_size];
	for(int i=0; i < list_size; i++)
	{
	  offset = i *5;

	  io[0 + offset].iov_base = &display_l[i].type;
	  io[0 + offset].iov_len = sizeof(display_l[i].type);
	  io[1 + offset].iov_base = &display_l[i].bearing;
	  io[1 + offset].iov_len = sizeof(display_l[i].bearing);
	  io[2 + offset].iov_base = &display_l[i].distance;
	  io[2 + offset].iov_len = sizeof(display_l[i].distance);
	  io[3 + offset].iov_base = &display_l[i].d_altitude;
	  io[3 + offset].iov_len = sizeof(display_l[i].d_altitude);
	  io[4 + offset].iov_base = &display_l[i].v_speed;
	  io[4 + offset].iov_len = sizeof(display_l[i].v_speed);	   
	
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

void to_display::receiveDataList(int sockBROAD, std::vector< to_display >& display_l)
{
   struct sockaddr_storage src_addr;
   

   int i, offset;
   // Limite de Aeronaves
   int list_size = 15; 
   // Vector Buffer
   std::vector< to_display > airspace(list_size);
   std::vector< to_display > reducted_airspace;   

   // Formata a mensagem a enviar
   struct iovec io[5*list_size];
   for(i=0;i<list_size;i++)
   {
      offset = i *5;

	  io[0 + offset].iov_base = &display_l[i].type;
	  io[0 + offset].iov_len = sizeof(display_l[i].type);
	  io[1 + offset].iov_base = &display_l[i].bearing;
	  io[1 + offset].iov_len = sizeof(display_l[i].bearing);
	  io[2 + offset].iov_base = &display_l[i].distance;
	  io[2 + offset].iov_len = sizeof(display_l[i].distance);
	  io[3 + offset].iov_base = &display_l[i].d_altitude;
	  io[3 + offset].iov_len = sizeof(display_l[i].d_altitude);
	  io[4 + offset].iov_base = &display_l[i].v_speed;
	  io[4 + offset].iov_len = sizeof(display_l[i].v_speed); 

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
      to_display aux;
      // Determina numero de avioes na lista
      int n_aeroplanes = count / 36;

      for(i=0;i<n_aeroplanes;i++)
      {
         aux = airspace[i];
         
         reducted_airspace.push_back(aux);
         
      }
      
   }  
   display_l = reducted_airspace; 
         
}

// calculates distance between 2 points
double Distance(double X0, double Y0, double Z0, double X1, double Y1, double Z1)
{
    return sqrt(pow(X1 - X0, 2) + pow(Y1 - Y0, 2) + pow(Z1 - Z0, 2));
}

// calculates range tau: time to CPA
double calculate_tau_r(double vx,double  vy,double vz,double vx_i,double vy_i,double vz_i,double distance)
{

	float closure_rate = sqrt(pow(vx-vx_i,2) + pow(vy-vy_i,2) + pow(vz-vz_i,2));

	return (distance/closure_rate);

}

// returns vertical tau: time to CPA on vertical plane
double calculate_tau_vert(double vx,double  vy,double vz,double vx_i,double vy_i,double vz_i,double lat,double lon,double lat_i,double lon_i,double d_alt,double* bearing,double* v_speed)
{
	double vx_enu, vy_enu, vz_enu,vx_enu_i, vy_enu_i, vz_enu_i;
	double mod, bearing_o, bearing_i;
	
	wgs_to_enu(vx, vy, vz,&vx_enu, &vy_enu, &vz_enu,lat, lon);

	wgs_to_enu(vx_i, vy_i, vz_i,&vx_enu_i, &vy_enu_i, &vz_enu_i,lat_i, lon_i);

	// printf("%f,%f,%f\n",vx_enu_i,vy_enu_i,vz_enu_i);

	bearing_o = atan(vy_enu/vx_enu)*180/PI;
	bearing_i = atan(vy_enu_i/vx_enu_i)*180/PI;
	

	*bearing = bearing_i - bearing_o;
	*v_speed = vz_enu_i;

	if(abs(-vz_enu+vz_enu_i)< th_d_alt)
	{
		return 0.0;
	}	

	return abs(d_alt/(-vz_enu+vz_enu_i));
}

// decides threshold according to own aircraft altitude
void get_thresold(int* th_tau_TA, int* th_tau_RA, int* th_alt_TA, int* th_alt_RA, int alt)
{
	if (alt < 1000 )
	{
		*th_tau_TA = 20;
		*th_tau_RA = 0; // N/A
		*th_alt_TA = 850;
		*th_alt_RA = 0; // N/A
	}

	else if(alt < 2350)
	{
		*th_tau_TA = 25;
		*th_tau_RA = 15;
		*th_alt_TA = 850;
		*th_alt_RA = 600;
	}

	else if(alt < 5000)
	{
		*th_tau_TA = 30;
		*th_tau_RA = 20;
		*th_alt_TA = 850;
		*th_alt_RA = 600;
	}

	else if(alt < 10000)
	{
		*th_tau_TA = 40;
		*th_tau_RA = 25;
		*th_alt_TA = 850;
		*th_alt_RA = 600;
	}

	else if(alt < 20000)
	{
		*th_tau_TA = 45;
		*th_tau_RA = 30;
		*th_alt_TA = 850;
		*th_alt_RA = 600;
	}

	else if(alt < 42000)
	{
		*th_tau_TA = 48;
		*th_tau_RA = 35;
		*th_alt_TA = 850;
		*th_alt_RA = 700;
	}

	else
	{
		*th_tau_TA = 48;
		*th_tau_RA = 35;
		*th_alt_TA = 1200;
		*th_alt_RA = 800;
	}

}

// compute the distance at CPA for two tracks: position + speed
double cpa_distance(double x0,double y0,double z0,double vx,double vy,double vz,double x0_i,double y0_i,double z0_i,double vx_i,double vy_i,double vz_i,double tau_range)
{    
	double x,y,z,x_i,y_i,z_i,distance;
    x = x0 + vx*tau_range;
    y = y0 + vy*tau_range;
    z = z0 + vz*tau_range;
    x_i = x0_i + vx_i*tau_range;
    y_i = y0_i + vy_i*tau_range;
    z_i = z0_i + vz_i*tau_range;

    distance = Distance(x, y, z, x_i, y_i, z_i);
    return distance;
}

// decides resolution for non obvious situations
void decide_resolution(char* resolution, double x0,double y0,double z0,double vx,double vy,double vz,double x0_i,double y0_i,double z0_i,double vx_i,double vy_i,double vz_i)
{
	float lat,lon,alt;
	double vx_enu, vy_enu, vz_enu;
	double vx_c, vy_c, vz_c, vx_d, vy_d, vz_d;
	double distance, d_up,d_down, tau_range;

	distance = Distance(x0, y0, z0, x0_i, y0_i, z0_i);
	
	wgs_to_geo(x0, y0, z0, &lat, &lon, &alt);
	wgs_to_enu(vx, vy, vz, &vx_enu, &vy_enu, &vz_enu, lat, lon);

	// in case we climb
	enu_to_wgs(vx_enu, vy_enu,1750*ftm_to_ms, &vx_c, &vy_c, &vz_c, lat, lon);
	tau_range = calculate_tau_r(vx_c,vy_c,vz_c,vx_i,vy_i,vz_i,distance);	
	d_up = cpa_distance(x0,y0,z0,vx_c,vy_c,vz_c,x0_i,y0_i,z0_i,vx_i,vy_i,vz_i,tau_range);

	// in case we descend
	enu_to_wgs(vx_enu, vy_enu,-1750*ftm_to_ms, &vx_d, &vy_d, &vz_d, lat, lon);
	tau_range = calculate_tau_r(vx_d,vy_d,vz_d,vx_i,vy_i,vz_i,distance);	
	d_down = cpa_distance(x0,y0,z0,vx_d,vy_d,vz_d,x0_i,y0_i,z0_i,vx_i,vy_i,vz_i,tau_range);

	// printf("%f,%f\n",d_up,d_down );

	if (d_up>d_down)
	{
		strcpy(resolution,"CLIMB");		
	}
	else
	{
		strcpy(resolution,"DESCEND");
	}
}

// TCAS SIM: reads airplanes from broacast and decides TCAS status
void TCAS_sim(std::vector< Airplane > airspace,std::vector< Airplane > &warning_area,std::vector< to_display > &display_l)
{	
	char TCAS_status[16] = "";
	double x,y,z,x_i,y_i,z_i;
	double vx,vy,vz,vx_i,vy_i,vz_i;
	Airplane my_airplane;
	double distance,tau_range, d_alt,tau_vertical,bearing,v_speed, bearing2;
	float lat, lon, alt, lat_i, lon_i, alt_i;
	int th_tau_TA,th_tau_RA,th_alt_TA,th_alt_RA;

	double x_enu, y_enu, z_enu;

	warning_area.clear();
	display_l.clear();   
	// std::vector< Airplane > warning_area;
   
	// My aircraft
	for(int i=0;i<airspace.size();i++)
	{
		if(airspace[i].id == AIRPLANE_ID)
        {
        	// char *pre_status = airspace[i].tcas_status;
        	// printf("my airplane %s\n",pre_status);
        	my_airplane = airspace[i];
        	x = my_airplane.x;
        	y = my_airplane.y;
        	z = my_airplane.z;
        	vx = my_airplane.vx;
        	vy = my_airplane.vy;
        	vz = my_airplane.vz;
        	airspace.erase(airspace.begin() + i);

        	wgs_to_geo(x, y, z, &lat, &lon, &alt);
        	wgs_to_enu( x, y, z, &x_enu, &y_enu, &z_enu, lat, lon);       	
        	printf("%f, %f, %f\n",x_enu,y_enu,z_enu );

        	// printf("%f, %f, %f\n",x,y,z );
        	printf("%fº, %fº, %f m\n",lat*180/PI,lon*180/PI,alt);
        }
	}

	// gets thresholds values given the aircraft altitude
	get_thresold(&th_tau_TA, &th_tau_RA, &th_alt_TA, &th_alt_RA, alt*m_to_ft); 
	printf("Tau: %i,%i alt: %i,%i\n",th_tau_TA,th_tau_RA,th_alt_TA,th_alt_RA );

	// Other aircraft
	for(int i=0;i<airspace.size();i++)
	{
		printf("--------------- ID %ld---\n",airspace[i].id);
		x_i = airspace[i].x;
		y_i = airspace[i].y;
		z_i = airspace[i].z;
		vx_i = airspace[i].vx;
		vy_i = airspace[i].vy;
		vz_i = airspace[i].vz;

		distance = Distance(x, y, z, x_i, y_i, z_i);
		printf("Distance: %f nm\n",distance * m_to_nm);

		tau_range = calculate_tau_r(vx,vy,vz,vx_i,vy_i,vz_i,distance);
		printf("Tau range: %f\n",tau_range);
		
		wgs_to_geo(x_i, y_i, z_i, &lat_i, &lon_i, &alt_i);

		// printf("Other altitude %f ft\n",alt_i*m_to_ft );

		/// bearing 2
		double dx_enu, dy_enu, dz_enu;
        	wgs_to_enu( (x_i-x), (y_i-y), (z_i-z), &dx_enu, &dy_enu, &dz_enu, lat, lon); 
		int normD = sqrt(pow(dx_enu, 2) + pow(dy_enu, 2));

		double vx_enu, vy_enu, vz_enu;
		wgs_to_enu(vx, vy, vz,&vx_enu, &vy_enu, &vz_enu,lat, lon);
		int normV = sqrt(pow(vx_enu, 2) + pow(vy_enu, 2));

		std::cout << dx_enu << " <- dx_enu; " << dy_enu << " <- dy_enu " << std::endl; 	

		std::cout << ((dx_enu*vx_enu+dy_enu*vy_enu)/(normD*normV)) << " modulo 1?" << std::endl; 
		bearing2 = acos((dx_enu*vx_enu+dy_enu*vy_enu)/(normD*normV)) * 180.0 / PI;
      
		int side = (vx_enu*(y_i-y)-vy_enu*(x_i-x));
		if(side < 0)
			bearing2 = -bearing2;

		///

		d_alt = alt_i-alt;
		printf("delta alt %f ft\n", d_alt*m_to_ft );

		if (abs(d_alt) >  th_d_alt){
			tau_vertical = calculate_tau_vert(vx,vy,vz,vx_i,vy_i,vz_i,lat,lon,lat_i,lon_i,d_alt,&bearing,&v_speed);
		}
		else{
			tau_vertical = 0;
		}
		printf("Tau vertical: %f\n",tau_vertical);
		// printf("%f º\n", bearing);

		if ((abs(d_alt*m_to_ft)<th_alt_RA) && (tau_vertical<th_tau_RA) && (tau_range<th_tau_RA))
		{
			to_display item(d_alt*m_to_ft, bearing2, distance,v_speed*m_to_ft*60,0);			
			display_l.push_back(item);
			strncpy(TCAS_status, "RESOLVING", sizeof(TCAS_status));
			// strncpy(my_airplane.tcas_status, "RESOLVING", sizeof(TCAS_status));
			warning_area.push_back(airspace[i]);
			printf("RA\n");
		}

		else if((abs(d_alt)*m_to_ft<th_alt_TA) && (tau_vertical<th_tau_TA) && (tau_range<th_tau_TA))
		{
			to_display item(d_alt*m_to_ft, bearing2, distance,v_speed*m_to_ft*60,1);
			display_l.push_back(item);
			printf("TA\n");
		}

		else if((abs(d_alt)*m_to_ft<1200) || distance*m_to_nm<6)
		{
			to_display item(d_alt*m_to_ft, bearing2, distance,v_speed*m_to_ft*60,2);
			display_l.push_back(item);
			printf("PT\n");
		}

		else
		{
			to_display item(d_alt*m_to_ft, bearing2, distance,v_speed*m_to_ft*60,3);
			display_l.push_back(item);
			printf("OT\n");
		}

	}

	// sorts list by priority - RA,TA,OT,PT
	sort(display_l.begin(), display_l.end(), sortByPriority);

	if (strcmp(TCAS_status,"") == 0)
	{
		strncpy(my_airplane.resolution, "\0", 16);
		// my_airplane.resolution = "\0";
		my_airplane.resol_value = 0;
		my_airplane.intr_id = 0;
		if (strcmp(my_airplane.tcas_status,"RESOLVING")==0)
		{
			strncpy(TCAS_status, "RETURNING", sizeof(TCAS_status));
		}
		else{		

			for (int i = 0; i < display_l.size(); ++i)
			{
				if (display_l[i].type == 1)
				{	
					if(strcmp(my_airplane.tcas_status,"RETURNING")==0)
					{
						strncpy(TCAS_status, "RETURNING", sizeof(TCAS_status));
					}
					else{
						strncpy(TCAS_status, "ADVISORY", sizeof(TCAS_status));
					}
					break;		
				}

				strncpy(TCAS_status, "CLEAR", sizeof(TCAS_status));
			}
		}
	}

	strncpy(my_airplane.tcas_status, TCAS_status, sizeof(TCAS_status));
	warning_area.push_back(my_airplane);

   //std::cout << warning_area.size() << " Warning area size " <<std::endl;
	printf("----------------------\nTCAS status: %s\n", TCAS_status);
}


// TCAS CTRL module: determines resolution actions (climb/descend)
Airplane TCAS_CTRL(std::vector< Airplane > &warning_area)
{
	double x,y,z,x_i,y_i,z_i;
	double vx,vy,vz,vx_i,vy_i,vz_i;
	float lat, lon, alt, lat_i, lon_i, alt_i;
	double vx_enu, vy_enu, vz_enu,vx_enu_i, vy_enu_i, vz_enu_i;
	uint64_t id,id_i;
	char resolution[16],resolution_i[16];
	char tcas_status_i[16];
	Airplane TCAS2mux;

   //for (int i = 0; i < warning_area.size(); ++i)
	//{
	//   std::cout << warning_area[i].id << " : "	<< warning_area[i].tcas_status << std::endl;
   //}


	// defining own and intruder data
	for (int i = 0; i < warning_area.size(); ++i)
	{
		if(warning_area[i].id == AIRPLANE_ID)
		{
			TCAS2mux = warning_area[i];	
         std::cout << warning_area[i].tcas_status <<  " : " << strcmp(warning_area[i].tcas_status,"RESOLVING") << std::endl;
			if (strcmp(warning_area[i].tcas_status,"RESOLVING") != 0)
			{            
				return TCAS2mux;
			}
			else
			{
				x = warning_area[i].x;
		    	y = warning_area[i].y;
		    	z = warning_area[i].z;
		    	vx = warning_area[i].vx;
		    	vy = warning_area[i].vy;
		    	vz = warning_area[i].vz;
		    	id = warning_area[i].id;
		    	strcpy(resolution, warning_area[i].resolution);
		    	wgs_to_geo(x, y, z, &lat, &lon, &alt);	
		    	wgs_to_enu(vx, vy, vz,&vx_enu, &vy_enu, &vz_enu,lat, lon);	
		    }  		
		}
		else
		{
			x_i = warning_area[i].x;
			y_i = warning_area[i].y;
			z_i = warning_area[i].z;
			vx_i = warning_area[i].vx;
			vy_i = warning_area[i].vy;
			vz_i = warning_area[i].vz;	
			id_i = warning_area[i].id;			
			strcpy(tcas_status_i, warning_area[i].tcas_status);
			strcpy(resolution_i, warning_area[i].resolution);
			wgs_to_geo(x_i, y_i, z_i, &lat_i, &lon_i, &alt_i);	
		    wgs_to_enu(vx_i, vy_i, vz_i,&vx_enu_i, &vy_enu_i, &vz_enu_i,lat_i, lon_i);	
		}
	}
	printf("v speed: %f,%f\n",vz_enu,vz_enu_i);
	printf("altitude: %f,%f\n",alt,alt_i);

	
	if (strcmp(tcas_status_i,"RESOLVING") == 0 ) 
	{
		// both made a decision - check conflict and see priorities
		if (strcmp(resolution,"CLIMB")==0 || strcmp(resolution,"DESCEND")==0 )
		{ 
			printf("both made a decision - check conflict and see priorities\n");
			if(strcmp(resolution_i,resolution)!=0) // all ok - resolutions match
			{
				strncpy(TCAS2mux.resolution,resolution,sizeof(TCAS2mux.resolution));	
				printf("all ok! Resolutions match\n");
			}
			else
			{
				if (id < id_i) // I have priority: maintain resolution
				{
					printf("I have priority, will maintain resolution\n");
					strncpy(TCAS2mux.resolution,resolution,sizeof(TCAS2mux.resolution));	
				} 
				else // He has priority: need to change resolution
				{
					printf("He has priority: need to change resolution\n");
					if (strcmp(resolution,"CLIMB")==0)
					{					
						strncpy(TCAS2mux.resolution,"DESCEND",sizeof(TCAS2mux.resolution));	
					}
					else
					{
						strncpy(TCAS2mux.resolution,"CLIMB",sizeof(TCAS2mux.resolution));	
					}
				}
			}			
		}
		// if intruder made a decision and we didn't: we complement its resolution
		else
		{
			if (strcmp(resolution_i,"CLIMB") == 0)
			{
				strncpy(TCAS2mux.resolution, "DESCEND",sizeof(TCAS2mux.resolution));	
			}
			else
			{
				strncpy(TCAS2mux.resolution, "CLIMB",sizeof(TCAS2mux.resolution));	
			}			
		} 
		
	}
	else // the intruder didn't decide, so we decide
	{
		if(abs(vz_enu_i)<th_stab && abs(vz_enu)<th_stab) // both stable
		{
			if (alt>alt_i)
			{
				strncpy(TCAS2mux.resolution, "CLIMB",sizeof(TCAS2mux.resolution));
			}
			else
			{
				strncpy(TCAS2mux.resolution, "DESCEND",sizeof(TCAS2mux.resolution));	
			}
		}
		else if(abs(vz_enu_i)<th_stab && abs(vz_enu)>th_stab) // int stable and own not
		{
			if (vz_enu>0 && alt>alt_i) // own climbing and is higher
			{
				strncpy(TCAS2mux.resolution, "CLIMB",sizeof(TCAS2mux.resolution));
			}
			else if (vz_enu<0 && alt<alt_i) // own descending and is lower
			{
				strncpy(TCAS2mux.resolution, "DESCEND",sizeof(TCAS2mux.resolution));
			}
		}
		else if(abs(vz_enu)<th_stab && abs(vz_enu_i)>th_stab) // own stable and intruder not
		{
			if(vz_enu_i>0 && alt_i>alt)
			{
				strncpy(TCAS2mux.resolution, "DESCEND",sizeof(TCAS2mux.resolution));
			}
			else if(vz_enu_i<0 && alt_i<alt)
			{
				strncpy(TCAS2mux.resolution, "CLIMB",sizeof(TCAS2mux.resolution));
			}
		}
		else if(abs(vz_enu)>th_stab && abs(vz_enu_i)>th_stab) // both not stable
		{
			if(vz_enu>0 && vz_enu_i<0 && alt>alt_i)
			{
				strncpy(TCAS2mux.resolution, "CLIMB",sizeof(TCAS2mux.resolution));
			}
			else if(vz_enu_i>0 && vz_enu<0 && alt_i>alt)
			{
				strncpy(TCAS2mux.resolution, "DESCEND",sizeof(TCAS2mux.resolution));
			}			
		}
		if (strcmp(TCAS2mux.resolution,"")==0)
		{			
			printf("Nothing decided yet. Needed conflict resolution.\n");
			decide_resolution(resolution, x, y, z, vx, vy, vz, x_i, y_i, z_i, vx_i, vy_i, vz_i);				
			strncpy(TCAS2mux.resolution, resolution,16);					
		}
	}

	TCAS2mux.resol_value = res_v_speed*ftm_to_ms;
	TCAS2mux.intr_id = id_i;

	printf("Resolution: %s, %f m/s\n",TCAS2mux.resolution, TCAS2mux.resol_value);

	return TCAS2mux;
}

int main( int argc, char *argv[] )
{
   //Recebe ports da consola
   int port_radar, port_mux,  port_disp;
   if(argc == 4)
   {
      port_radar = atoi(argv[1]);
      port_mux = atoi(argv[2]);
      port_disp = atoi(argv[3]);
   }
   else
   {
      port_radar = PORT_RADAR;
      port_mux = PORT_MUX;
      port_disp = PORT_DISPLAY;
   }



	std::vector< Airplane > warning_area; // list to send from TCAS sim to TCAS CTRL
	std::vector< to_display > display_l; // list to send from TCAS sim to display
	Airplane to_mux; // airplane to send from TCAS CTRL to MUX
    std::vector< Airplane > airspace;

    // TIRAR DEPOIS-----------------------------------------------------------------

	// Airplane pombo;
	// pombo.x = 4725711.493418; 
	// pombo.y = -714958.573418;
	// pombo.z = 4213921.006865;
	// pombo.vx = 83.841366;
	// pombo.vy = -50.673805;
	// pombo.vz = -105.600935;
	// pombo.id = 216412359;
	// strncpy(pombo.tcas_status, "RESOLVING", 16);
	// strncpy(pombo.resolution, "CLIMB", 16);
	// airspace.push_back(pombo); // ours

	// Airplane andorinha;
	// andorinha.x = 4719200;
	// andorinha.y = -719000;
	// andorinha.z = +4220360;
	// andorinha.vx = -85;
	// andorinha.vy = +40;
	// andorinha.vz = +80;
	// andorinha.id = 2207253491;
	// airspace.push_back(andorinha); 

	// Airplane tucano;
	// tucano.x = 4725500;
	// tucano.y = -715000;
	// tucano.z = +4214400;
	// tucano.vx = -85;
	// tucano.vy = +40;
	// tucano.vz = -80;
	// tucano.id = 2219999;
	// strncpy(tucano.tcas_status, "RESOLVING", 16);
	// strncpy(tucano.resolution, "CLIMB", 16);
	// airspace.push_back(tucano); 

	// Airplane cegonha;
	// cegonha.x = 4725500;
	// cegonha.y = -716000;
	// cegonha.z = +4215000;
	// cegonha.vx = -85;
	// cegonha.vy = +40;
	// cegonha.vz = -80;
	// cegonha.id = 1119999111;
	// airspace.push_back(cegonha);

	// Airplane corvo;
	// corvo.x = 4725500;
	// corvo.y = -715000;
	// corvo.z = +4213800;
	// corvo.vx = -85;
	// corvo.vy = +40;
	// corvo.vz = -80;
	// corvo.id = 5099999900;
	// airspace.push_back(corvo);

	// Airplane rola;
	// rola.x = 4735500;
	// rola.y = -719000;
	// rola.z = +4225000;
	// rola.vx = -85;
	// rola.vy = +40;
	// rola.vz = -80;
	// rola.id = 8889999988;
	// airspace.push_back(rola);

	// TIRAR DEPOIS-----------------------------------------------------------------
    
	// socket 1: RADAR -> TCAS SIM
	int sockfd_radar;
	Airplane RADAR2SIM;
    struct sockaddr_in serv_addr_radar;
    RADAR2SIM.newSocks( &sockfd_radar, &serv_addr_radar, port_radar, false);
    

	// socket 2: TCAS SIM -> display
	int sockfd_disp;
    struct sockaddr_in serv_addr_disp;
    to_display SIM2DISP;
    SIM2DISP.newSocks( &sockfd_disp, &serv_addr_disp, port_disp, true);
   

    // socket 3: TCAS CTRL -> MUX
   int sockfd_mux;
	Airplane CTRL2MUX; 
   struct sockaddr_in serv_addr_mux;
    CTRL2MUX.newSocks( &sockfd_mux, &serv_addr_mux, port_mux, true); 
   

    double bea[30], d_alt[30], dist[30], v_sp[30]; 
    int t[30];

    while(true)
    {

		// receive airspace from RADAR - 8228
		RADAR2SIM.receiveDataList(sockfd_radar, airspace);

		TCAS_sim(airspace,warning_area,display_l);

		// send display_l to display - 8678
		//SIM2DISP.sendMsgList(sockfd_disp, &serv_addr_disp, display_l);

      std::cout << "Display Size " << display_l.size() << std::endl ;
      for(int i = 0; i< display_l.size(); i++)         
      {
         bea[i] = display_l[i].bearing;
         d_alt[i] = display_l[i].d_altitude;
         dist[i] = display_l[i].distance;  
         v_sp[i] = display_l[i].v_speed;
         t[i] = display_l[i].type;          
      }
      enviacarago(sockfd_disp, &serv_addr_disp, d_alt, bea, dist, v_sp,  t, (int) display_l.size() );

		to_mux = TCAS_CTRL(warning_area);

      // atribuição
      CTRL2MUX = to_mux;
		// send airplane to_mux to MUX - 8668
		CTRL2MUX.sendMsg(sockfd_mux, &serv_addr_mux);
	}
	
	close(sockfd_radar);
	close(sockfd_disp);
	close(sockfd_mux);

}

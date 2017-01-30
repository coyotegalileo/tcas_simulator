#ifndef _TCAS_H
#define _TCAS_H

#include "../converts/converts.h"
#include "../airplane/airplane.h"
#include <cmath>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>

// Conversions
#define m_to_nm 0.000539956803
#define m_to_ft 3.2808399
#define ftm_to_ms 0.00508

// Sockets Ports
#define PORT_RADAR 8228 // The port to receive data for TCAS SIM
#define PORT_DISPLAY 8678 // The port to send data from TCAS SIM to display
#define PORT_MUX 8668 // The port to send data from TCAS CTRL to MUX

// Constants
#define th_stab 0.5 // m/s aprox 100 ft /min
#define th_d_alt 0.5 // m
#define res_v_speed 1500 // vertical speed for resolution (ft/min)

// Classes
class to_display
{
	public:
		to_display();
		to_display(double,double,double,double,int);
		double bearing; // º	
		double distance; // nm
		double d_altitude; // ft		
		double v_speed; // ft/min		
		int type; // 0 - RA, 1 - TA, 2 - PT, 3 - OT
		void newSocks(int*, struct sockaddr_in*,int, bool, const char* = "Nope");
		void sendMsgList(int, struct sockaddr_in*, std::vector< to_display >);    
		void receiveDataList(int, std::vector< to_display>&);
		
	private:	

};

bool sortByPriority(const to_display &lhs, const to_display &rhs) { return lhs.type < rhs.type; }

// functions
double Distance(double X0, double Y0, double Z0, double X1, double Y1, double Z1);
double calculate_tau_r(double vx,double  vy,double vz,double vx_i,double vy_i,double vz_i,double distance);
double calculate_tau_vert(double vx,double  vy,double vz,double vx_i,double vy_i,double vz_i,double lat,double lon,double lat_i,double lon_i,double d_alt,double* bearing);
void get_thresold(int* th_tau_TA, int* th_tau_RA, int* th_alt_TA, int* th_alt_RA, int alt);
void TCAS_sim(std::vector< Airplane > airspace,std::vector< Airplane > &warning_area,std::vector< to_display > &display_l);
void decide_resolution(char* resolution, double x0,double y0,double z0,double vx,double vy,double vz,double x0_i,double y0_i,double z0_i,double vx_i,double vy_i,double vz_i);
double cpa_distance(char* resolution, double x0,double y0,double z0,double vx,double vy,double vz,double x0_i,double y0_i,double z0_i,double vx_i,double vy_i,double vz_i);
Airplane TCAS_CTRL(std::vector< Airplane > &warning_area);



#endif

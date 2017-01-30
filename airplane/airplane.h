

#ifndef AIRPLANE_H
#define AIRPLANE_H

#define AIRPLANE_ID 9999 //216412359 // se tiver 0 no inicio é octal 999//

// #define AIRPLANE_ID 2796771980 // se tiver 0 no inicio é octal 999//

#define ADDRESS_NET "192.168.1.94"//"194.210.157.92"  //"192.168.1.94"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <vector>
#include <chrono>
#include <inttypes.h>


//Linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <sys/uio.h>
#include <iomanip>

uint32_t checksum(unsigned char * message);
bool validate_checksum(unsigned char* message);
uint32_t crc32(unsigned char * message);
uint32_t GetCrc32(unsigned char * message);
void error(const char *msg);

class Airplane
{


    public:
        Airplane();
        Airplane(double, double, double ,double ,double, double);
        virtual ~Airplane();

        void sendMsg(int, struct sockaddr_in*);
        void sendMsgList(int, struct sockaddr_in*, std::vector< Airplane >, int);
        void newSocks(int*, struct sockaddr_in*,int, bool, const char* = "Nope");
        //void newSocks(int*, struct sockaddr_in*,int, bool);          
        void receiveData(int);
        void receiveDataList(int, std::vector< Airplane >&);
        
      
      
        // void *point[13]; para ja nao
        // Variaveis de cada aviao conforme acordado
        char header [16];
        uint64_t id;
        double x, y, z;
        double vx, vy, vz;
        char tcas_status [16];
        uint64_t intr_id;
        char resolution [16];
        double resol_value;
        uint32_t crc;

        // Variaveis para auto pilot
        bool change_target;

    
    protected:

    private:
};

#endif // AIRPLANE_H



#ifndef AIRPLANE_H
#define AIRPLANE_H

#define AIRPLANE_ID 216412359 // se tiver 0 no inicio é octal

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <cmath>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <vector>

//Linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>
#include <sys/uio.h>
#include <iomanip>

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
        void createSockSend(int*, struct sockaddr_in*,int);
        void createSockReceive(int*, struct sockaddr_in*,int);        
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

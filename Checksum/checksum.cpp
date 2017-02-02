#include <cmath>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h> 
#include <boost/crc.hpp>  // for boost::crc_32_type


// Generates random ascii char array
// void gen_random(char *s, const int len) {
//     static const char alphanum[] =
//         "0123456789"
//         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//         "abcdefghijklmnopqrstuvwxyz";

//     for (int i = 0; i < len; ++i) {
//         s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
//     }

//     s[len] = 0;
// }

// Converts uint32 to a ascii char array
void uint32ToBuff(unsigned char *out, uint32_t in){
    uint32_t aux=in;
    for(int i=0; i<4; i++){

        out[3-i]= (aux & 0xFF);
        aux=(aux >> 8);
    }
}

//  Calculates checksum of a message  - returns int
// uint32_t checksum(unsigned char * message)
// {
//     uint32_t crc32= 0;
//     int shift = 0;
//     for (int i = 0; i < 120; ++i){
//         crc32 += (message[i] << shift);
//         shift += 8; // assuming bytes instead of bits
//         if (shift == 32){
//             shift = 0;
//         }
//     }    
//     return crc32;
// }

uint32_t GetCrc32(unsigned char * message) {
    boost::crc_32_type result;
    result.process_bytes(message, 120);
    return result.checksum();
}

// Check whether the checksum is valid
bool validate_checksum(unsigned char* message)
{
    unsigned char crc_buff[4];
    unsigned char first_part[120];
    unsigned char last_part[4];
    bool answer;

    for (int i = 0; i < 120; ++i)
    {
        first_part[i] = message[i];
    }

    for (int i = 0; i < 4; ++i)
    {
        last_part[i] = message[120+i];
    }

    // uint32_t crc32 = checksum(first_part);
    uint32_t crc32 = GetCrc32(first_part);

    unsigned char const* crc_chars;
    crc_chars = reinterpret_cast<unsigned char const *>(&crc32);

  
   // Print para debug
   //for (int i = 0; i < 4; ++i)
   //{
   //   printf("1. %x\n",crc_chars[i]);
   //   printf("2. %x\n",last_part[i]);
   //   printf("CRC. %u\n",crc32); 
   //} 
  
   
   answer = 0;
   for(int i = 0; i<4; i++)    
   {
      if(last_part[i]!=crc_chars[i])   
      {
         answer = 1;
         break;
      }
   }

    if (answer == 0)
      return true;
    else
      return false; // true if 0
}


/*
int strain()
{
    char message[120];
    char message_final[124];
    char crc_buff[4];
    bool validation;
    char strout;

    // message definition
    gen_random(message, sizeof(message));
    printf("message = %s\nsize = %zu\n",message,sizeof(message));

    // checksum calculation
    uint32_t crc32 = checksum(message);
    printf("checksum = %d\nsize = %zu\n",crc32,sizeof(crc32));

    // checksum to buffer
    uint32ToBuff(crc_buff, crc32);
    printf("%s\n", crc_buff);

    //final message
    for (int i = 0; i < 120; ++i)
    {
        message_final[i] = message[i];
    }

    for (int i = 0; i < 4; ++i)
    {
        message_final[i+120] = crc_buff[i];
    }
    printf("final message = %s\nsize = %zu\n",message_final,sizeof(message_final));

    validation = validate_checksum(message_final);

    if (validation == 0)
    {
        printf("Valid checksum\n");
    }
    else
    {
        printf("Invalid checksum - discard message \n");
    }
}
*/

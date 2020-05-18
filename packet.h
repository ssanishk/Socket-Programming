// Name: Anishkumar SS
// ID No: 2017A7PS0069P
// Question 1

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PACKET_SIZE 100 // No of bytes of data to be transferred in one packet 
#define PDR 10          // Packet Drop Rate = 10%
#define TIMEOUT 2       // Timeout is set to be 2 Seconds.

typedef struct packet{
    int size;
    int sq_no;
    int lastflg;    // 1 for last byte and 0 otherwise
    int ackdataflg; // 0 for data and 1 for ACK
    char data[PACKET_SIZE];
} PKT;

void end_conn(int a, int b, FILE* fp)
{
    close(a);
    close(b);
    fclose(fp);
    printf("File has been transferred \nConnection terminated\n");
    exit(1);
}

void die(char *s)
{
    perror(s);
    exit(1);
}

typedef struct buffer_system
{

    PKT packet;
    struct buffer_system *next;
    int size; // Expresses the size of Linked List

} Buffer_store;

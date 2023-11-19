#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MACROS
#define SERVER_IP "127.0.0.1"
#define LOCAL_HOST "127.0.0.1"
#define SERVER_PORT_TO 5002
#define CLIENT_PORT 6001
#define SERVER_PORT 6002
#define CLIENT_PORT_TO 5001
#define PAYLOAD_SIZE 1024
#define MAX_SEQUENCE 1024

// able to change these
#define WINDOW_SIZE 5 
#define TIMEOUT 2 


// Packet Layout
// You may change this if you want to
struct packet {
    unsigned short seqnum;
    unsigned short acknum;

    //flags
    char ack;// true means this is an ack
    char last;// this is the last pkt

    unsigned int length;// length of payload (?)
    char payload[PAYLOAD_SIZE];// 1024 payload
};

// Utility function to build a packet
void build_packet(struct packet* pkt, unsigned short seqnum, unsigned short acknum, char last, char ack,unsigned int length, const char* payload) {
    pkt->seqnum = seqnum;
    pkt->acknum = acknum;
    pkt->ack = ack;
    pkt->last = last;
    pkt->length = length;
    memcpy(pkt->payload, payload, length);
}

// Utility function to print a packet
void printRecv(struct packet* pkt) {
    printf("RECV %d %d%s%s\n", pkt->seqnum, pkt->acknum, pkt->last ? " LAST": "", (pkt->ack) ? " ACK": "");
}

void printSend(struct packet* pkt, int resend) {
    if (resend)
        printf("RESEND %d %d%s%s\n", pkt->seqnum, pkt->acknum, pkt->last ? " LAST": "", pkt->ack ? " ACK": "");
    else
        printf("SEND %d %d%s%s\n", pkt->seqnum, pkt->acknum, pkt->last ? " LAST": "", pkt->ack ? " ACK": "");
}

struct file_info{
    int size;// Size of the file in bytes
    int sections; // Number of 1024 byte sections
    int trail; // Trailing bytes (file is non-multiple of 1024)
};

void getFileInfo( struct file_info* file_info, FILE * fp) {
    long fileSize;

    fseek(fp, 0, SEEK_END);// Seek to the end of the file
    fileSize = ftell(fp);
    rewind(fp);// Go back to beginning
    
    int remainingBytes = fileSize % 1024;
    int totalSections = (fileSize / 1024) + (remainingBytes > 0 ? 1 : 0);

    file_info->size = fileSize;
    file_info->sections = totalSections;
    file_info->trail = remainingBytes;
}

void printBuffer(char* buffer, int size) {
    /* Function for testing */
    for (int i = 0; i < size-2; ++i) {
        printf("%c", buffer[i]);
    }
    printf("\n");
}

void printFileInfo(file_info * fi){
    printf("File Info: size = %d, sections = %d, trail = %d \n",fi->size, fi->sections, fi->trail);
}

void readFileSection(char* buffer, FILE * fp, int seq_num, int bytes){
    // TODO: function that retrieves seq_num'th section of 1024 bytes of the file pointed to by fp and store it in buffer
    int offset = seq_num * PAYLOAD_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking to the desired section\n");
        return;
    }
    size_t bytesRead = fread(buffer, 1, PAYLOAD_SIZE, fp);
    if (bytesRead != bytes) {
        fprintf(stderr, "Error reading the file section\n");
        return;
    }
}

#endif
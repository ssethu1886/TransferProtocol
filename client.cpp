#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>

#include "utils.h"


int main(int argc, char *argv[]) {
    int listen_sockfd, send_sockfd;
    struct sockaddr_in client_addr, server_addr_to, server_addr_from;
    socklen_t addr_size = sizeof(server_addr_to);
    struct timeval tv;
    struct packet pkt;
    struct packet ack_pkt;
    char buffer[PAYLOAD_SIZE];
    unsigned short seq_num = 0;
    unsigned short ack_num = 0;
    char last = 0;
    char ack = 0;

    // read filename from command line argument
    if (argc != 2) {
        printf("Usage: ./client <filename>\n");
        return 1;
    }
    char *filename = argv[1];

    // Create a UDP socket for listening
    listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_sockfd < 0) {
        perror("Could not create listen socket");
        return 1;
    }

    // Create a UDP socket for sending
    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sockfd < 0) {
        perror("Could not create send socket");
        return 1;
    }

    // Configure the server address structure to which we will send data
    memset(&server_addr_to, 0, sizeof(server_addr_to));
    server_addr_to.sin_family = AF_INET;
    server_addr_to.sin_port = htons(SERVER_PORT_TO);
    server_addr_to.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Configure the client address structure
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the listen socket to the client address
    if ( bind(listen_sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        perror("Bind failed");
        close(listen_sockfd);
        return 1;
    }

    // Connect to send_fd proxy server - where we send packets to be forwarded to server
    if( connect(send_sockfd, (struct sockaddr *)&server_addr_to, sizeof(server_addr_to)) < 0 ){
        perror("Connection failed");
        close(send_sockfd);
        return 1;
    }

    // Open file for reading
    FILE *fp = fopen(filename, "rb");
    file_info file_info;
    if (fp == NULL) {
        perror("Error opening file");
        close(listen_sockfd);
        close(send_sockfd);
        return 1;
    } else {
        getFileInfo(&file_info,fp);
        printFileInfo(&file_info);
    }

    // stop and wait (still need timeouts)
    char pkt_buffer[PKT_SIZE];// buffer to hold pkt
    for( seq_num = 0; seq_num < file_info.sections; seq_num++){
        
        // Read file section and store in a packet's payload
        readFileSection(buffer,fp,seq_num,PAYLOAD_SIZE);// read 1024, put in buffer
        build_packet(&pkt, seq_num,ack_num,0,0,MAX_SEQUENCE,buffer);//build pkt (use buffer)
        memcpy(pkt_buffer,&pkt,sizeof(pkt));// copy pkt into buffer (for sending)
        // Send 
        sendto(send_sockfd,pkt_buffer,PKT_SIZE,0,(const sockaddr*)&server_addr_to,sizeof(server_addr_to));

        // Wait for ACK
        packet recieved_pkt
        usleep(TIMEOUT * 100000);// wait for 0.2 seconds (RTT?) 
        while( read(listen_fd, pkt_buffer, PKT_SIZE) < 0 )// Attempt to read an ACK 
        {
            sendto(send_sockfd,pkt_buffer,PKT_SIZE,0,(const sockaddr*)&server_addr_to,sizeof(server_addr_to));
        }
        printRecv(recieved_pkt);
        
        //seq_num++;
        usleep(TIMEOUT * 1000);
        //ack_num += rec_pkt ack (cumulative?)
    }
    // send last packet
    readFileSection(buffer,fp,seq_num,file_info.trail);// read trail bytes, put in buffer
    build_packet(&pkt, seq_num,ack_num,1,0,file_info.trail,buffer);//build pkt (use buffer)
    memcpy(pkt_buffer,&pkt,sizeof(pkt));// copy pkt into buffer
    ssize_t bytes_sent = sendto(send_sockfd,pkt_buffer,PKT_SIZE,0,(const sockaddr*)&server_addr_to,sizeof(server_addr_to));
    printSend(&pkt,0);
    printBuffer(pkt.payload,pkt.length);
    
    fclose(fp);
    close(listen_sockfd);
    close(send_sockfd);
    return 0;
}

    


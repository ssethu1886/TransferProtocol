#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "utils.h"

int main() {
    int listen_sockfd, send_sockfd;
    struct sockaddr_in server_addr, client_addr_from, client_addr_to;
    struct packet buffer;
    socklen_t addr_size = sizeof(client_addr_from);
    int expected_seq_num = 0;
    int recv_len;
    struct packet ack_pkt;

    // Create a UDP socket for sending
    send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sockfd < 0) {
        perror("Could not create send socket");
        return 1;
    }

    // Create a UDP socket for listening
    listen_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_sockfd < 0) {
        perror("Could not create listen socket");
        return 1;
    }

    // Configure the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the listen socket to the server address
    if (bind(listen_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listen_sockfd);
        return 1;
    }

    // Configure the client address structure to which we will send ACKs
    memset(&client_addr_to, 0, sizeof(client_addr_to));
    client_addr_to.sin_family = AF_INET;
    client_addr_to.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    client_addr_to.sin_port = htons(CLIENT_PORT_TO);

    // Connect to send_fd proxy server - where we send acks to be forwarded to client
    if( connect(send_sockfd, (struct sockaddr *)&client_addr_to, sizeof(client_addr_to)) < 0 ){
        perror("Connection failed");
        close(send_sockfd);
        return 1;
    }
    
    // Open the target file for writing (always write to output.txt)
    FILE *fp = fopen("output.txt", "wb");
    
    // error checking 
    if (!fp) {
        perror("File open failed");
        close(listen_sockfd);
        close(send_sockfd);
        return 1;
    }

    // set timeout length
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;

    // TODO: Receive file from the client and save it as output.txt
    int total_byte = 0;
    while(true){ // recieve until we find a pkt with last = true
        int val_read = 0;
        char pkt_buff[PKT_SIZE];
        packet rec_pkt;

        // set timeout for sending ack
    //int j = setsockopt(send_sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        val_read = recv(listen_sockfd, pkt_buff, PKT_SIZE,0);
       
        memcpy(&rec_pkt, pkt_buff, sizeof(packet));
        printRecv(&rec_pkt);

        // write payload to output file
        total_byte += writePkt(rec_pkt.seqnum, rec_pkt.payload, fp, rec_pkt.length);
	//if (rec_pkt.last) {
    //     writeFileSection(pkt_buff,fp,rec_pkt.seqnum,rec_pkt.length);
	//} else {
    //     writeFileSection(pkt_buff,fp,rec_pkt.seqnum,PAYLOAD_SIZE);
        //}
        packet ack_pkt;
        build_packet(&ack_pkt, 0, rec_pkt.seqnum, 0, 1, 0, NULL); //build ack pkt
        sendto(send_sockfd, &ack_pkt, sizeof(ack_pkt), 0, (const sockaddr*)&client_addr_to, sizeof(client_addr_to));
        printSend(&ack_pkt,0);
        
        if(rec_pkt.last){
            printf("last\n");
            break;
        }  //break if last packet
        
    }
    fclose(fp);
    close(listen_sockfd);
    close(send_sockfd);
    return 0;
}

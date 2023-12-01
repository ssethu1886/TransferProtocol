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
    //fcntl(listen_sockfd, F_SETFL, O_NONBLOCK);// shouldnt have to move on after hang ?
    // rcv -> send -> rcv .... -> send , if nothing to read, dont send anythin ?
    if (listen_sockfd < 0) {
        perror("s: Could not create listen socket");
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

    
    ///////////////////////////////////////////////////////////////////////////////////////////

    int total_byte = 0;

    char listen_for_file = true;
    while(listen_for_file){ // recieve until we read all data in
        char pkt_buff[PKT_SIZE];
        packet rec_pkt;

        // Recieve pkt from proxy 
        if ( recv(listen_sockfd, pkt_buff, PKT_SIZE,0) < PKT_SIZE ){
            printf("Failed to read full packet (s)\n");
            continue;
        } 
        memcpy(&rec_pkt, pkt_buff, sizeof(packet));//store in rec_pkt
        printRecv(&rec_pkt);
        // TODO, handle dup packets - track seq nums already recieved ?

        // Save the pkt's payload to output.txt
        total_byte += writePkt(rec_pkt.seqnum, rec_pkt.payload, fp, rec_pkt.length);

        /* TEST ACK FOR S&G TESTING */
                    // ackpkt  seqnum of recv'd pkt  next expected  T id recv'd is last else false                no payload 
        char test_buf[PKT_SIZE];
        build_packet(&ack_pkt, rec_pkt.seqnum, rec_pkt.seqnum + 1 , rec_pkt.last ? 1 : 0, 1 /*IMPORTANT 1=true*/, 0 , NULL );
        memcpy(test_buf,&ack_pkt,PKT_SIZE);
        sendto(send_sockfd,test_buf,PKT_SIZE,0,(const sockaddr*)&client_addr_to,sizeof(client_addr_to));
        printSend(&ack_pkt,0);
        /* ------------------------ */


        // Check if done ( only S&G can end at pkt.last  )
        if(rec_pkt.last){
            printf("last ack");
            listen_for_file = false;// stop listening
            break;//break when we recieve last packet (only for stop and go)
            // later, might be when we send ack for last pkt ( dont care if client gets the ack, server is done (grader ends at server end ) )
            // or maybe when total_byte = PAYLOAD_SIZE * ( rec_pkt.seqnum - 1 ) + rec_pkt.length (tail) (on to something....) 
        }

        // send(); // TODO send ack to send_sockfd - 5001
    }
    fclose(fp);
    close(listen_sockfd);
    close(send_sockfd);
    return 0;
}

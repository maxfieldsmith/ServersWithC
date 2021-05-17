#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define OPCODE_POST 0x01
#define OPCODE_POSTING 0x02
#define OPCODE_RECEIVE 0x03
#define OPCODE_RECEIVING 0x04

#define MAGIC_1 0x4D
#define MAGIC_2 0x53

int main() {

    int ret;
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char recv_buffer[1024];
    int recv_len;
    socklen_t len;
    char last_message[400];
    FILE *ptr;
    ptr = fopen("udp_log.txt", "a+");
    // This is a memory buffer to hold the message for retrieval.
    // I just declare a static character array for simplicity. You can
    // use a linked list or std::vector<...>. My array is only big enough
    // to hold one message, which is the most recent one.
    // Note that one more character is needed to hold the null-terminator
    // of a C string with a length of 200, i.e., strlen(msg) == 200).
    char recent_msg[201];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);                                                if (sockfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // The servaddr is the address and port number that the server will
    // keep receiving from.
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(32000);

    bind(sockfd,
         (struct sockaddr *) &servaddr,
         sizeof(servaddr));

    while (1) {
        len = sizeof(cliaddr);
        recv_len = recvfrom(sockfd, // socket file descriptor
                 recv_buffer,       // receive buffer
                 sizeof(recv_buffer),  // max number of bytes to be received
                 0,
                 (struct sockaddr *) &cliaddr,  // client address
                 &len);             // length of client address structure

        if (recv_len <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }

          // TODO: check whether the recv_buffer contains a proper header
        // at the beginning. If it does not have a proper header, just
        // ignore the datagram. Note that a datagram can be up to 64 KB,
        // and hence, the input parameter of recvfrom() on the max number
        // of bytes to be received should be the maximum message size in
        // the protocol we designed, i.e., size of the header (4 bytes) +
        // the size of the maximum payload (200 bytes). For the same reason,
        // the receiver buffer should be at least this big.

        // One more thing about datagram is that one datagram is a one
        // unit of transport in UDP, and hence, you either receive the whole
        // datagram or nothing. If you provide a size parameter less than
        // the actual size of the received datagram to recvfrom(), the OS
        // kernel will just truncates the datagram, fills your buffer with
        // the beginning part of the datagram and silently throws away the
        // remaining part. As a result, you can not call recvfrom() twice
        // to expect to receive the header first and then the message
        // payload next in UDP. That can only work with TCP.

        // If the received message has the correct format, send back an ack
        // of the proper type.

        if (recv_buffer[0] != MAGIC_1 || recv_buffer[1] != MAGIC_2) {

            // Bad message!!! Skip this iteration
            printf("Something wrong with magic!");
            continue;

        } else {

            if (recv_buffer[2] == OPCODE_POST) {
                // Note that you need to erase the memory to store the most
                // recent message first. C string is always terminated by
                // a '\0', but when we send the line, we did not send
                // this null-terminator.
                char ack[1024];
                memset(recent_msg, 0, sizeof(recent_msg));

                // Note that recv_buffer[3] contains the length of the text
                // line, see the protocol description.
                // Be careful, you may need to do some sanity checks on
                // recv_buffer[3], i.e., whether it is non-zero, etc.
                memcpy(recent_msg, recv_buffer + 4, recv_buffer[3]);

                fputs(recent_msg, ptr);
                fclose(ptr);
                // TODO: Now you need to assemble an ack, like what you did
                // in the client code.
                recv_buffer[2] = OPCODE_POSTING;
                strcpy(ack, "post_ack#successful");
                memcpy(recv_buffer + 4, ack, strlen(ack));
                recv_buffer[3] = strlen(ack);
            } else if (recv_buffer[2] == OPCODE_RECEIVE) {

                // TODO: Handle the retrieve message.
                char ack[1024];
                recv_buffer[2] = OPCODE_RECEIVING;
                strcpy(ack, "retreive_ack#");
                strcat(ack, recent_msg);
                memcpy(recv_buffer + 4, ack, strlen(ack));
                recv_buffer[3] = strlen(ack);
            } else {
                printf("something wrong with post");
                // Wrong message format. Skip this iteration.
                continue;
            }

        }

        // You are supposed to call the sendto() function here to send back
        // the echoed message, using "cliaddr" as the destination address.
        sendto(sockfd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

    }
    fclose(ptr);
    return 0;
}
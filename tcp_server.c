#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// This line must be included if you want to use multithreading.
// Besides, use "gcc ./tcp_receive.c -lpthread -o tcp_receive" to compile
// your code. "-lpthread" means link against the pthread library.
#include <pthread.h>

// This the "main" function of each worker thread. All worker thread runs
// the same function. This function must take only one argument of type
// "void *" and return a value of type "void *".

#define MAGIC_1 0x4D
#define MAGIC_2 0x53
#define OPCODE_UPLOAD 0x80
#define OPCODE_UPLOADING 0x81
#define OPCODE_DOWNLOAD 0x82
#define OPCODE_DOWNLOADING 0x83

void *worker_thread(void *arg) {

    int ret;
    int connfd = (int) (long)arg;
    char recv_buffer[1024];

    printf("[%d] worker thread started.\n", connfd);

    while (1) {
        ret = recv(connfd,
                    recv_buffer,
                    sizeof(recv_buffer),
                    0);

        if (ret < 0) {
            // Input / output error.
                printf("[%d] recv() error: %s.\n", connfd, strerror(errno));
                return NULL;
        } else if (ret == 0) {
            // The connection is terminated by the other end.
                printf("[%d] connection lost\n", connfd);
                break;
        }

        // TODO: Process your message, receive chunks of the byte stream,
        // write the chunks to a file. You also need an inner loop to
        // write the chunks to a file. You also need an inner loop to
        // receive and write each chunk.
        // This is the main loop of the worker thread
        while (1) {

            // Receive the fixed size header, i.e., just read out the first 8
            // bytes.
            ret = recv(connfd, recv_buffer, 8, 0);

            // Parse the "recv_buffer", check the header, and extract the
            // length of the file name. Similarly, extract the file size.

            // Call recv() again, receive exactly "file_name_length" number
            // of bytes. Do remember to check the return value and verify that
            // it really receives the desired amount of bytes.
            // Note that TCP is different from UDP. A connection is a stream of
            // bytes. Hence, you can call recv() multiple times to only read out
            // part of the bytes in the stream in sequence. However, in UDP, you
            // can only call recvfrom() once to get a single datagram. A datagram
            // is read out as a whole piece or truncated if the buffer you provide
            // is not big enough. But in TCP, you can call recv() many times and
            // each time you read out as many bytes as specified in the second
            // argument of recv().

            // TODO: Now you need to check whether it is an upload message or a
            // download message by inspecting the received header.

            if (recv_buffer[2] == OPCODE_UPLOAD) {

                // Now we know it is an upload message.

                // Open a new file to writing, using the file name you received.
                FILE *ptr;
                char file_name[64];
                memcpy(file_name, recv_buffer + 8, recv_buffer[3]);
                ptr = fopen(file_name, "wb");
                long file_size = 8;
                // This is the inner loop, like that on the client side.
                int bytes_received = 0;
                while (bytes_received < file_size) {

                    // receive a chunk to the recv_buffer, e.g., 1024 bytes.
                    // Similarly, you can declare a static buffer of 1024 bytes,
                    // or you can malloc() dynamically.
                    bytes_received += recv(connfd,recv_buffer, strlen(recv_buffer), 0);

                    // write this chunk to the file
                    fwrite("hello", 1, sizeof("hello"), ptr);
                }

                // Once this loop finishes, every byte of the file is received.
                // Now you send an acknowledgment.

                send(connfd, recv_buffer, strlen(recv_buffer), 0);

            //} else if (recv_buffer[2] == OPCODE_DOWNLOAD) {


            //} else {

            // Unknown opcode!! Probably your client code is buggy!!
            // Just print out a line and abort.
            //exit(-1);
            }//}


    }




    printf("[%d] worker thread terminated.\n", connfd);
}}


// The main thread, which only accepts new connections. Connection socket
// is handled by the worker thread.
int main(int argc, char *argv[]) {

    int ret;
    socklen_t len;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
          printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(31000);

    ret = bind(listenfd, (struct sockaddr*)
               &serv_addr, sizeof(serv_addr));
    if (ret < 0) {
          printf("bind() error: %s.\n", strerror(errno));
        return -1;
    }


    if (listen(listenfd, 10) < 0) {
          printf("listen() error: %s.\n", strerror(errno));
        return -1;
    }

    while (1) {
          printf("waiting for connection...\n");
        connfd = accept(listenfd,
                 (struct sockaddr*) &client_addr,
                 &len);

        if(connfd < 0) {
                printf("accept() error: %s.\n", strerror(errno));
                return -1;
        }
          printf("conn accept - %s.\n", inet_ntoa(client_addr.sin_addr));

          pthread_t tid;
          pthread_create(&tid, NULL, worker_thread, (void *)(long)connfd);

    }
    return 0;
}
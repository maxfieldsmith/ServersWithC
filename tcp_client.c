#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAGIC_1 0x4D
#define MAGIC_2 0x53
#define OPCODE_UPLOAD 0x80
#define OPCODE_UPLOADING 0x81
#define OPCODE_DOWNLOAD 0x82
#define OPCODE_DOWNLOADING 0x83

int main(int argc, char *argv[]) {
    int ret;
    int sockfd = 0;
    char send_buffer[1024];
    struct sockaddr_in serv_addr;
    char file_name[64];
    FILE *ptr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
          printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // Note that this is the server address that the client will connect to.
    // We do not care about the source IP address and port number.

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(31000);

    ret = connect(sockfd,
                  (struct sockaddr *) &serv_addr,
                  sizeof(serv_addr));
    if (ret < 0) {
          printf("connect() error: %s.\n", strerror(errno));
        return -1;
    }

    while (1) {
        fgets(send_buffer,
              sizeof(send_buffer),
              stdin);

        // These two lines allow the client to "gracefully exit" if the
        // user type "exit".
        int FILE_NAMELENGTH = 0;
        if (strncmp(send_buffer, "exit", strlen("exit")) == 0)
                break;

          // TODO: You need to parse the string you read from the keyboard,
          // check the format, extract the file name, open the file,
          // read each chunk into the buffer and send the chunk.
          // You need to write an inner loop to read and send each chunk.
          //
          // Note that you need to send the header before sending the actual
        // file data
        // Connection set up code ...

        // This the outer loop of the client.
        while (1) {

            // The client read a line using fgets()

            // Check this line, whether it contains "upload$" or "download$"
            // at the start. Assume we are dealing with an upload message.
            if (strncmp(send_buffer, "upload$", strlen("upload$")) == 0) {
                FILE_NAMELENGTH = strlen(send_buffer) - 7;
                memcpy(file_name, send_buffer + strlen("upload$"), FILE_NAMELENGTH);
                printf("%s", file_name);
                printf("%d", FILE_NAMELENGTH);
                send_buffer[0] = MAGIC_1;
                send_buffer[1] = MAGIC_2;
                send_buffer[2] = OPCODE_UPLOAD;
                send_buffer[3] = FILE_NAMELENGTH;


            } else if (strncmp(send_buffer, "download$", strlen("download$")) == 0) {
                FILE_NAMELENGTH = strlen(send_buffer) - strlen("download$");
                memcpy(send_buffer + strlen("download$"), file_name, FILE_NAMELENGTH);
                send_buffer[0] = MAGIC_1;
                send_buffer[1] = MAGIC_2;
                send_buffer[2] = OPCODE_DOWNLOAD;
                send_buffer[3] = FILE_NAMELENGTH;

            } else {
                printf("Error: Unrecognized command format.\n");
                continue;
            }
            // Open the file using fopen(). If it is not successful,
            // print a line about the error and break.
            ptr = fopen(file_name, "rb");
            printf("%s", file_name);
            // If the file open is successful, obtain the file name and file size.
            // You can seek to the end and get the offset using fseek()
            // and ftell(), which tells the file size.
            // Note that after this you need to call fseek() again to go back to
            // the beginning of the file. Otherwise your fread() only returns an
            // end-of-file error.
            long file_size = 0;
            if (ptr) {
                fseek(ptr, 0, SEEK_END);
                file_size = ftell(ptr);
                fseek(ptr, 0, SEEK_SET);
            } else {
                printf("File not found!\n");
            }

            send_buffer[4] = file_size;

            // Now send the header. If the message has no payload, go to the
            // receive part.

            // Next, use a nested inner loop to send the chunks
            int bytes_send = 0;
            while (bytes_send < file_size) {

                    // Read a chunk to the send_buffer, e.g., read 1024 bytes.
                    // You can declare a static buffer of 1024 bytes,
                    // like the "send_buffer" variable. Or you can allocate the
                    // buffer dynamically using malloc().
                    char buffer[1024];
                    fread(buffer, sizeof(buffer), 1, ptr);

                    memcpy(send_buffer + 7 + FILE_NAMELENGTH, buffer, sizeof(buffer));
                    // Send this chunk ret = send(sockfd, send_buffer, strlen(send_buffer), 0);

                    // Check the return value. if nothing goes wrong, keep counting. bytes_send += ret;
            }

            // Once this loop finishes, every breyte of the file is sent.
            // Now the client waits for an acknowledgment.

            recv(sockfd, send_buffer, strlen(send_buffer) + 1, 0);

            // Check whether the received message is OK, and print a line.
            // Note that this simple code does not handle all kinds of errors.
            // It should work if nothing goes wrong.
            // We don't expect you to design a protocol that is robust against
            // all kinds of errors. As long as it works in this simple case,
            // it is OK.
        }


    }

    close(sockfd);

    return 0;
}
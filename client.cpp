#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include "applicationprotocol.h"

static int32_t query (int fd, const char* text);

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        printf("socket()");
        exit(1);
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; // ipv4
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1 speaking to self
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));

    if (rv) {
        std::cerr << "connect" << std::endl;
        exit(1);
    }

    // char msg[] = {};
    // std::cout << "Send Message: ";
    // std::cin >> msg;
    // write(fd, msg, strlen(msg));
    
    // char rbuf[64] = {};
    // ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);

    // if (n < 0){
    //     std::cerr << "Read Error" << std::endl;
    //     exit(1);
    // }
    // printf("server says: %s\n", rbuf);

    int32_t err = query(fd, "Where is animal pasture");
    if (err) {
        goto L_DONE;
    }

    err = query(fd, "Count the sheeps");
    if (err) {
        goto L_DONE;
    }

    err = query(fd, "Good Night!");
    if (err) {
        goto L_DONE;
    }

L_DONE:
        close(fd);
        return 0;


}

static int32_t query (int fd, const char* text) {
    uint32_t len = (uint32_t)strlen(text);
    if (len > k_max_msg){
        return -1; // too long
    }

    // send request
    char wbuf[4 + k_max_msg]; // initialize string to have 4 + length of max message
    memcpy(wbuf, &len, 4); // set first 4 chars to the length needed
    memcpy(&wbuf[4], text , len); // set text to the space after the length space 
    if (int32_t err = write_all(fd, wbuf, 4 + len)){
        return err;
    }
    // 4 bytes header
    char rbuf[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
        if (errno == 0){
            printf("EOF");
        } else {
            printf("read() error");
        }
        return err;
    }
    memcpy(&len, rbuf, 4); // assume little endian?
    if (len > k_max_msg){
        printf("too long");
        return -1;
    }
    
    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        printf("read() error");
        return err;
    }
    // do something
    rbuf[4 + len] = '\0'; // mark end of string
    printf("server says: %s\n", &rbuf[4]);
    return 0;
}
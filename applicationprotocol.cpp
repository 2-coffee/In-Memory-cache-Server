#include <cstdint>
#include <stddef.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <stdio.h>
#include <string.h>
#include "applicationprotocol.h"
#include <iostream>

int32_t read_full(int fd, char *buf, size_t n) {
    /* Sometimes read could be interrupted*/
    while (n>0) {
        ssize_t rv = read(fd, buf ,n);
        if (rv == 0 && errno == EINTR) {
            continue; // signal interruption
        } else if (rv < 0) {
            return -1; // error, or unexpceted EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

int32_t write_all( int fd, const char *buf, size_t n) {
    /* The reason this loop is needed is because we might write less than n.*/
    while (n>0) { 
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) { // if we wrote nothing
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv; // subtract whatever we could write
        buf += rv; // move pointer up
    }
    return 0;
}

int32_t one_request(int connfd) {
    // 4 bytes header
    char rbuf[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        if (errno == 0){
            printf("EOF\n");
        } else {
            printf("read() error");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4); // assume a little endian
    if (len > k_max_msg) {
        std::cerr << "too long\n" ;
        return -1;
    }
    
    // request body
    err = read_full(connfd, &rbuf[4], len);
    if (err){
        printf("read() error");
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    printf("client says: %s\n", &rbuf[4]);

    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd,wbuf, 4 + len);
}
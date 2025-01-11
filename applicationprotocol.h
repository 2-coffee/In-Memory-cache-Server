

#ifndef APPLICATIONPROTOCOL_H
#define APPLICATIONPROTOCOL_H
#include <cstdint>
#include <stddef.h>

const size_t k_max_msg = 4096; // 12
int32_t write_all( int fd, const char *buf, size_t n);
int32_t read_full(int fd, char *buf, size_t n);
int32_t one_request(int connfd);

#endif
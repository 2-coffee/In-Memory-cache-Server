#include "sys/socket.h"
#include "unistd.h"
#include "stdio.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <stdbool.h>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include "applicationprotocol.h"


// static void do_something(int connfd);


int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET is for IPv4. Use AF_INET6 for IPv6 or dual-stack
    // SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP
    // 3rd is 0 
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    // 2nd and 3rd specifies which option to set
    // the 4th argument is the option value
    // different options use different types, so the size of the option value is also needed.

    /* In this case, SO_REUSEADDR is set to and int value of 1, this option accepts a boolean value of 0 or 1. This is related to the delayed packets and TCP TIME_WAIT.
    The effect of SO_REUSEADDR is important: if it's not set to 1, a server program cannot bind to the same IP:port it was using after a restart. This is generally undesirable TCP behavior. You should enable SO_REUSEADDR for all listening sockets! 
    */

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234); // port
    addr.sin_addr.s_addr = ntohl(0); // wildcard IP 0.0.0.0
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr)); // struct sockaddr_in holds an IPv4:port pair stored as big numbers, converted by ntohs(). For example, 1.2.3.4 is represented by ntohl(0x01020304).
    if (rv) {
        std::cerr << "bind()" << std::endl;
        exit(1);
    }
    /* All of the previous steps are just passing parameters. The socket is actually created after listen().
    The OS will automatically handle TCP handshakes and place established connections in a queue.
    The application can then retrieve them via accept().
    */

   // listen
    rv = listen(fd, SOMAXCONN); // second argument is the size of the queue
    if (rv) {
        std::cerr << "listen()" << std::endl;
        exit(1);
    }

    while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen); // accept the connection
        if (connfd < 0){
            continue; // error
        }

        // only serves one client connection at once
        while(true) {
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        //do_something(connfd);
        close(connfd);
    }
}

// static void do_something(int connfd) {
//     char rbuf[64] = {};
//     ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1); // connection socket size

//     if (n<0){
//         std::cerr << "read() error" << std::endl;
//         return;
//     }
//     printf("client says: %s\n", rbuf);

//     std::cout << "Respond: ";
//     char wbuf[] = {}; // respond 
//     std::cin >> wbuf;
//     write(connfd, wbuf, strlen(wbuf));

//     // can replace read/write with send/recv
// }


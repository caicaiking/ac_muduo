//
// Created by acai on 6/12/22.
//

#ifndef AC_MUDUO_SOCKETS_OPS_H
#define AC_MUDUO_SOCKETS_OPS_H

#include <arpa/inet.h>

namespace ac_muduo::net::sockets {

    int create_nonblocking_or_die(sa_family_t family);

    int connect(int socket_fd, const struct sockaddr *addr);

    void bind_or_die(int socket_fd, const struct sockaddr *addr);

    void listen_or_die(int socket_fd);

    int accept(int socket_fd, struct sockaddr_in6 *addr);

    ssize_t read(int socket_fd, void *buf, size_t count);

    ssize_t readv(int socket_fd, const struct iovec *iov, int iovcnt);

    ssize_t write(int socket_fd, const void *buf, size_t count);

    void close(int socket_fd);

    void shutdown_write(int socket_fd);

    void to_ip_port(char *buf, size_t size, const struct sockaddr *addr);

    void to_ip(char *buf, size_t size, const struct sockaddr *addr);

    void from_ip_port(const char *buf, uint16_t port, struct sockaddr_in *addr);

    void from_ip_port(const  char *buf, uint16_t port, struct sockaddr_in6 *addr);

    int get_socket_error(int socket_fd);

    const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);

    const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);

    struct sockaddr *sockaddr_cast(struct sockaddr_in *addr);

    struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);

    const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *);

    const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *);

    struct sockaddr_in6 get_local_addr(int sockfd);

    struct sockaddr_in6 get_peer_addr(int sockfd);

    bool is_self_connect(int sockfd);
}


#endif //AC_MUDUO_SOCKETS_OPS_H

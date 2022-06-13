//
// Created by acai on 6/12/22.
//

#include "sockets_ops.h"
#include "base/logging.h"
#include "base/types.h"

#include <cerrno>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include "endian.h"

namespace ac_muduo::net::sockets {


    int create_nonblocking_or_die(sa_family_t family) {

        int socket_fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                                 IPPROTO_TCP);
        if (socket_fd < 0) {
            LOG_SYSFATAL << "sockets::create_nonblocking_or_die";
        }
        return socket_fd;
    }

    int connect(int socket_fd, const struct sockaddr *addr) {
        return ::connect(socket_fd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    }

    void bind_or_die(int socket_fd, const struct sockaddr *addr) {
        int ret = ::bind(socket_fd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
        if (ret < 0) {
            LOG_SYSFATAL << "sockets::bind_or_die";
        }
    }

    void listen_or_die(int socket_fd) {
        int ret = ::listen(socket_fd, SOMAXCONN);
        if (ret < 0) {
            LOG_SYSFATAL << "sockets::listen_or_die";
        }
    }

    int accept(int socket_fd, struct sockaddr_in6 *addr) {
        auto addrlen = static_cast<socklen_t>(sizeof(*addr));
        int conn_fd = ::accept4(socket_fd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (conn_fd < 0) {
            int saved_errono = errno;
            LOG_SYSERR << "Socket::accept";
            switch (saved_errono) {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO: // ???
                case EPERM:
                case EMFILE: // per-process lmit of open file desctiptor ???
                    // expected errors
                    errno = saved_errono;
                    break;
                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                    // unexpected errors
                    LOG_FATAL << "unexpected error of ::accept " << saved_errono;
                    break;
                default:
                    LOG_FATAL << "unknown error of ::accept " << saved_errono;
                    break;
            }
        }
        return conn_fd;
    }

    const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr) {
        return static_cast<const struct sockaddr *>(implicit_cat<const void *>(addr));
    }

    const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr) {
        return static_cast<const struct sockaddr *>(implicit_cat<const void *>(addr));
    }

    struct sockaddr *sockaddr_cast(struct sockaddr_in *addr) {
        return static_cast<struct sockaddr *>(implicit_cat<void *>(addr));
    }

    struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr) {
        return static_cast<struct sockaddr *>(implicit_cat<void *>(addr));
    }

    const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr) {
        return static_cast<const struct sockaddr_in *>(implicit_cat<const void *>(addr));
    }

    const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr) {
        return static_cast<const struct sockaddr_in6 *>(implicit_cat<const void *>(addr));
    }

    ssize_t read(int socket_fd, void *buf, size_t count) {
        return ::read(socket_fd, buf, count);
    }

    ssize_t readv(int socket_fd, const struct iovec *iov, int iovcnt) {
        return ::readv(socket_fd, iov, iovcnt);
    }

    ssize_t write(int socket_fd, const void *buf, size_t count) {
        return ::write(socket_fd, buf, count);
    }

    void close(int socket_fd) {
        if (::close(socket_fd) < 0) {
            LOG_SYSERR << "sockets::close";
        }
    }

    void shutdown_write(int socket_fd) {
        if (::shutdown(socket_fd, SHUT_WR) < 0) {
            LOG_SYSERR << "sockets::shutdown_write";
        }
    }

    void to_ip_port(char *buf, size_t size, const struct sockaddr *addr) {

        if (addr->sa_family == AF_INET6) {
            buf[0] = '[';
            to_ip(buf + 1, size - 1, addr);
            size_t end = ::strlen(buf);

            const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
            uint16_t port = network_to_host_16(addr6->sin6_port);
            assert(size > end);
            snprintf(buf + end, size - end, "]:%u", port);
            return;
        }

        to_ip(buf, size, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
        uint16_t port = network_to_host_16(addr4->sin_port);
        assert(size > end);
        snprintf(buf + end, size - end, ":%u", port);
    }

    void to_ip( char *buf, size_t size, const struct sockaddr *addr) {

        if (addr->sa_family == AF_INET) {
            assert(size > INET_ADDRSTRLEN);
            auto addr4 = sockaddr_in_cast(addr);
            ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
        } else if (addr->sa_family == AF_INET6) {
            assert(size > INET6_ADDRSTRLEN);
            auto addr6 = sockaddr_in6_cast(addr);
            ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
        }

    }

    void from_ip_port(const char *ip, uint16_t port, struct sockaddr_in *addr) {
        addr->sin_family = AF_INET;

        addr->sin_port = host_to_network_16(port);
        if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
            LOG_SYSERR << "sockets::from_ip_port";
        }

    }

    void from_ip_port(const char *ip, uint16_t port, struct sockaddr_in6 *addr) {
        addr->sin6_family = AF_INET6;

        addr->sin6_port = host_to_network_16(port);
        if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
            LOG_SYSERR << "sockets::from_ip_port";
        }
    }

    int get_socket_error(int socket_fd) {
        int opt_val;
        socklen_t opt_len = static_cast<socklen_t>(sizeof opt_val);

        if (::getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &opt_val, &opt_len) < 0) {
            return errno;
        } else {
            return opt_val;
        }
    }

    struct sockaddr_in6 get_local_addr(int sockfd) {
        struct sockaddr_in6 local_addr;
        mem_zero(&local_addr, sizeof local_addr);

        socklen_t addr_len = static_cast<socklen_t >(sizeof local_addr);

        if (::getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0) {
            LOG_SYSERR << "sockets::get_local_addr";
        }
        return local_addr;
    }

    struct sockaddr_in6 get_peer_addr(int sockfd) {
        struct sockaddr_in6 peer_addr;
        mem_zero(&peer_addr, sizeof peer_addr);
        socklen_t addr_len = static_cast<socklen_t >(sizeof peer_addr);

        if (::getpeername(sockfd, sockaddr_cast(&peer_addr), &addr_len) < 0) {
            LOG_SYSERR << "sockets::get_peer_addr";
        }
        return peer_addr;
    }

    bool is_self_connect(int sockfd) {
        auto local_addr = get_local_addr(sockfd);
        auto peer_addr = get_peer_addr(sockfd);

        if (local_addr.sin6_family == AF_INET) {
            auto laddr4 = reinterpret_cast<struct sockaddr_in *>(&local_addr);
            auto paddr4 = reinterpret_cast<struct sockaddr_in *>(&peer_addr);

            return laddr4->sin_port == paddr4->sin_port &&
                   laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;

        } else if (local_addr.sin6_family == AF_INET6) {
            return local_addr.sin6_port == peer_addr.sin6_port &&
                   memcmp(&local_addr.sin6_addr, &peer_addr.sin6_addr, sizeof local_addr.sin6_addr) == 0;
        } else {
            return false;
        }

    }


}
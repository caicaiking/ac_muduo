#include <iostream>
#include <thread>
#include "net/event_loop_thread_t.h"
#include "net/event_loop_t.h"
#include "base/logging.h"
#include "net/sockets_ops.h"


int main(int argc, char **argv) {
    sockaddr_in addr;
    ac_muduo::net::sockets::from_ip_port("192.169.1.1", 5000, &addr);

    char buf[255];
    ac_muduo::net::sockets::to_ip_port(buf, sizeof buf, ac_muduo::net::sockets::sockaddr_cast(&addr));

    std::cout << buf << std::endl;
}
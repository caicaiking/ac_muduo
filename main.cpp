#include <iostream>
#include <thread>
#include "net/event_loop_thread_t.h"
#include "net/event_loop_t.h"
#include "base/logging.h"

void print(int value) {
    LOG_TRACE << "value: " << value;
}

void init_fun(ac_muduo::net::event_loop_t * t)
{
    LOG_INFO << "init function";
}

int main(int argc, char **argv) {
    ac_muduo::logger::set_log_level(ac_muduo::logger::TRACE);
    ac_muduo::net::event_loop_thread_t thread(init_fun);
    auto event_loop = thread.start_loop();

    std::thread([&] {

        event_loop->run_every(1, [] { print(1); });
        event_loop->run_after(6, [] { print(6); });
        sleep(20);
        event_loop->quit();

    }).detach();

    sleep(30);
}
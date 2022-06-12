#include <iostream>
#include <thread>
#include "net/event_loop_t.h"
#include "base/logging.h"

void print(int value) {
    LOG_TRACE << "value: " << value;
}

int main(int argc, char **argv) {
    ac_muduo::logger::set_log_level(ac_muduo::logger::TRACE);
    ac_muduo::net::event_loop_t event_loop;

    std::thread([&] {
        event_loop.run_every(1, [] { print(1); });
        event_loop.run_after(6, [] { print(6); });
        sleep(20);
        event_loop.quit();
    }).detach();

    event_loop.loop();
}
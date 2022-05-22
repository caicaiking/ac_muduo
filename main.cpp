#include <iostream>
#include <base/weak_callback.h>
#include <base/thread.h>
#include "base/exception.h"
#include "base/current_thead.h"

struct foo
{
    void print(int a)
    {
        std::cout << a << std::endl;
    }
};

int main() {
    foo f;
    ac_muduo::thread t([&]{f.print(10);});
    t.start();
    std::cout<< t.name() << std::endl;
    t.join();

    ac_muduo::thread t2([&]{f.print(1);});
    t2.start();
    std::cout<< t2.name() << std::endl;
    t2.join();

    return 0;
}

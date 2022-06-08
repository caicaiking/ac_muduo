#include <iostream>
#include "base/logging.h"
#include "base/async_logging.h"

using namespace ac_muduo;

async_logging g_asyc_log("test_demo", 10000*1000, 3);

void print_log(const char* data, int len)
{
    g_asyc_log.append(data,len);
}
void flush()
{
}


int main() {
    char * data = "0123456789abcdefghijklmnopqrstuvwxyz;!@#$%^&";
    g_asyc_log.start();


    logger::set_output_fun(print_log);
    logger::set_flush(flush);
    logger::set_log_level(logger::log_level_t::TRACE);


    char buf[11];
    for(int i = 0; i != 1000*1000; ++i)
    {
        for(int j= 0; j!= sizeof(buf)-2; ++j)
        {
           buf[j] = data [ random() %(strlen(data))];
        }

        buf[sizeof(buf)-2]=0;
        buf[sizeof(buf)-1]=0;

        LOG_INFO << string_piece(buf, strlen(buf)) ;
        usleep(500);

    }



}

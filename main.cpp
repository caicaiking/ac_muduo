#include <iostream>
#include "base/time_zone.h"
#include "base/date_t.h"
#include "base/file_utils.h"
#include "base/process_info.h"
#include "base/process_info.h"
#include "base/log_stream.h"
#include "base/logging.h"

using namespace ac_muduo;

int main() {
    time_zone beijing(8 * 3600, "CST");
    logger::set_time_zone(beijing);
    logger::set_log_level(logger::log_level_t::TRACE);

    LOG_TRACE << "trace NYT";
    LOG_DEBUG << "debug NYT";
    LOG_INFO << "Hello NYT";
    LOG_WARN << "World NYT";
    LOG_ERROR << "Error NYT";
    LOG_FATAL << "FATAL NYT";

}

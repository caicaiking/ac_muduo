//
// Created by acai on 5/22/22.
//

#include "exception.h"

#include "current_thead.h"

namespace ac_muduo {

    exception::exception(string what) :
            message_(std::move(what)),
            stack_(current_thread::stack_trace(false)) {

    }
}

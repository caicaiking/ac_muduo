cmake_minimum_required(VERSION 3.22)
project(ac_muduo)

set(CMAKE_CXX_STANDARD 20)

set (APP_SRC
        src/base/types.h
        src/base/weak_callback.h src/base/noncopyable.h src/base/copyable.h src/base/count_down_latch.cpp src/base/count_down_latch.h src/base/thread.cpp src/base/thread.h src/base/current_thead.cpp src/base/current_thead.h src/base/exception.cpp src/base/exception.h)


add_executable(main main.cpp ${APP_SRC})

target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/src)

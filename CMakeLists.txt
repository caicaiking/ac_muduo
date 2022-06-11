cmake_minimum_required(VERSION 3.22)
project(ac_muduo)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE})
file(GLOB APP_SRC
        src/base/*.h
        src/base/*.cpp
        src/net/*.h
        src/net/*.cpp
        src/net/poller/*.cpp
        src/net/poller/*.h
        )


add_executable(main main.cpp ${APP_SRC})

target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/src)

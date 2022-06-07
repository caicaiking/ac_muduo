cmake_minimum_required(VERSION 3.22)
project(ac_muduo)

set(CMAKE_CXX_STANDARD 20)

file(GLOB APP_SRC
        src/base/*.h
        src/base/*.cpp
        )


add_executable(main main.cpp ${APP_SRC})

target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/src)

//
// Created by acai on 6/13/22.
//

#ifndef AC_MUDUO_ENDIAN_H
#define AC_MUDUO_ENDIAN_H

#include <cinttypes>
#include <endian.h>

inline uint16_t network_to_host_16(uint16_t net16) {
    return be16toh(net16);
}

inline uint16_t host_to_network_16(uint16_t host16) {
    return htobe16(host16);
}

#endif //AC_MUDUO_ENDIAN_H

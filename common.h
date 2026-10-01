#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cerrno>

constexpr size_t k_max_msg = 4096;

inline void die(const char *msg)
{
    std::cerr << std::strerror(errno) << " " << msg << std::endl;
    exit(EXIT_FAILURE);
}


#endif

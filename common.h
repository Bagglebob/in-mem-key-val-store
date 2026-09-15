#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>

// 'inline' prevents duplicate symbol errors if linked across multiple files
inline void die(const char *msg) {
    perror(msg);
    exit(1);
}

inline void msg(const char *msg) {
    printf("%s\n", msg);
}

#endif

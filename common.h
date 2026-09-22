#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>

constexpr size_t k_max_msg = 4096;

// 'inline' prevents duplicate symbol errors if linked across multiple files
inline void die(const char *msg) {
    perror(msg);
    exit(1);
}

inline void msg(const char *msg) {
    printf("%s\n", msg);
}

// recv
// read_full:
// When reading a disk file and it returns less than requested, 
// it means either EOF or an error. But a socket can return less data even under normal conditions
// TODO (DONE): handle this case in read_full
// read can also be interrupted by a signal because it must wait if the buffer is empty. 
// In this case, 0 bytes are read, but the return value is -1 and errno is EINTR
inline int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv == -1 && errno == EINTR) { continue; } // TODO PART DONE
        if (rv <= 0) {
            return -1;  // error, or unexpected EOF
        }
        // to avoid unsigned int underflow, n > rv otherwise you get wrap around
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

// send
// write_all:
// when using write(), the OS copies the data from your apps user-space memory buffer 
// into a socket buffer inside kernel space; however, the buffer size is limited.
// while the buffer waits to free up, the write() syscall may be interrupted.
// write_all() gives us a loop to work with,
// so while there is more to write, continue writing to kernal space
inline int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error
        }
        // to avoid unsigned int underflow
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

#endif

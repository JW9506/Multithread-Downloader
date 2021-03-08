#include "utils/unit_util.h"
#include <time.h>
#include <tinycthread.h>

uint64_t TimeInMillisecond() {
    time_t curr_time;
    time(&curr_time);
    return (uint64_t)curr_time * 1000;
}

void sleepms(unsigned int ms) {
    long seconds = ms / 1000;
    long nanoseconds = ms % 1000 * 1000000L;
    thrd_sleep(&(struct timespec){ .tv_sec = seconds, .tv_nsec = nanoseconds },
               NULL);
}

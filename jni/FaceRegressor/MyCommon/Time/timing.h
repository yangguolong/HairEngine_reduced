#ifndef __TIMING_H
#define __TIMING_H

// determine which OS
#if defined(_WIN32)
  #define OS_WIN
#elif defined(__APPLE__) && defined(__MACH__)
  #define OS_MAC
#else
  #define OS_LNX
#endif


// define internal datatype
#if defined(OS_WIN)
  #include <windows.h>
  typedef LARGE_INTEGER gtime_t;
#elif defined(OS_MAC)
  // http://developer.apple.com/qa/qa2004/qa1398.html
  #include <mach/mach_time.h>
  typedef uint64_t gtime_t;
#elif defined(OS_LNX)
  #include <sys/time.h>
  typedef struct timeval gtime_t;
#endif


// get current time
static inline gtime_t gtime_now(void)
{
#if defined(OS_WIN)
    gtime_t time;
    QueryPerformanceCounter(&time);
    return time;
#elif defined(OS_MAC)
    return mach_absolute_time();
#elif defined(OS_LNX)
    gtime_t time;
    gettimeofday(&time, NULL);
    return time;
#endif
}



// absolute difference between two times (in seconds)
static inline double gtime_seconds(gtime_t start, gtime_t end)
{
#if defined(OS_WIN)
    if (start.QuadPart > end.QuadPart) {
        gtime_t temp = end;
        end = start;
        start = temp;
    }
    gtime_t system_freq;
    QueryPerformanceFrequency(&system_freq);
    return (double)(end.QuadPart - start.QuadPart) / system_freq.QuadPart;
#elif defined(OS_MAC)
    if (start > end) {
        uint64_t temp = start;
        start = end;
        end   = temp;
    }
    // calculate platform timing epoch
    static mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    double nano = (double)info.numer / (double)info.denom;
    return (end - start) * nano * 1e-9;
#elif defined(OS_LNX)
    struct timeval elapsed;
    timersub(&start, &end, &elapsed);
    long sec = elapsed.tv_sec;
    long usec = elapsed.tv_usec;
    double t = sec + usec * 1e-6;
    return t >= 0 ? t : -t;
#endif
}


#endif // __TIMING_H

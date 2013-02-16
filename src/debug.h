#ifndef DEBUG_H_
#define DEBUG_H_

#define VERBOSITY_NONE 0
#define VERBOSITY_ERROR 1
#define VERBOSITY_WARNING 2
#define VERBOSITY_INFO 3
#define VERBOSITY_DEBUG 4
#define VERBOSITY_EVERYTHING 5

char *debug_getVersionInfo();

#if VLNone
#define VERBOSITY_LEVEL VERBOSITY_NONE
#elif VLError
#define VERBOSITY_LEVEL VERBOSITY_ERROR
#define DEBUG
#elif VLWarning
#define VERBOSITY_LEVEL VERBOSITY_WARNING
#define DEBUG
#elif VLInfo
#define VERBOSITY_LEVEL VERBOSITY_INFO
#define DEBUG
#elif VLDebug
#define VERBOSITY_LEVEL VERBOSITY_DEBUG
#elif VLEverything
#define VERBOSITY_LEVEL VERBOSITY_EVERYTHING
#define DEBUG
#else
#endif

#ifdef DEBUG
void TraceCall(int line, const char *file, const char *function, int level, const char *format, ...);
void TraceTimedCall(int line, const char *file, const char *function, int level, const char *format, ...);
#define VTRACE(level,format, ...) TraceCall(__LINE__, __FILE__, __FUNCTION__, level, format, ##__VA_ARGS__)
#define VTRACETIME(level,format, ...) TraceTimedCall(__LINE__, __FILE__, __FUNCTION__,level,  format, ##__VA_ARGS__)
#else
#define VTRACE(...)
#define VTRACETIME(...)
#endif

#if VERBOSITY_LEVEL>=VERBOSITY_ERROR
#define ETRACE(format, ...) TraceCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_ERROR, format, ##__VA_ARGS__)
#define ETRACETIME(format, ...) TraceTimedCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_ERROR,  format, ##__VA_ARGS__)
#else
#define ETRACE(...)
#define ETRACETIME(...)
#endif

#if VERBOSITY_LEVEL>=VERBOSITY_INFO
#define TRACE(format, ...) TraceCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_INFO, format, ##__VA_ARGS__)
#define TRACETIME(format, ...) TraceTimedCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_INFO,  format, ##__VA_ARGS__)
#else
#define TRACE(...)
#define TRACETIME(...)
#endif


#if VERBOSITY_LEVEL>=VERBOSITY_WARNING
#define WTRACE(format, ...) TraceCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_WARNING, format, ##__VA_ARGS__)
#define WTRACETIME(format, ...) TraceTimedCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_WARNING,  format, ##__VA_ARGS__)
#else
#define WTRACE(...)
#define WTRACETIME(...)
#endif

#if VERBOSITY_LEVEL>=VERBOSITY_DEBUG
#define DTRACE(format, ...) TraceCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_DEBUG, format, ##__VA_ARGS__)
#define DTRACETIME(format, ...) TraceTimedCall(__LINE__, __FILE__, __FUNCTION__, VERBOSITY_DEBUG,  format, ##__VA_ARGS__)
#else
#define DTRACE(...)
#define DTRACETIME(...)
#endif
#endif

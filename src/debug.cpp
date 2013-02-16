#include "debug.h"

#ifdef DEBUG
char levels[][16]={"", "Error: ", "Warning: ", "", "Debug: ", ""};

#include <SDL/SDL.h>
#include <stdarg.h>
void TraceCall(int line, const char *file, const char *function, int level, const char *format, ...) {
    if(VERBOSITY_LEVEL<level) return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsprintf(buffer, format, args);
    fprintf(stdout, "%20s @ %d%30s()\t\t\t%s%s\n", file, line, function, levels[level], buffer);
    va_end(args);
}

void TraceTimedCall(int line, const char *file, const char *function, int level, const char *format, ...) {
    if(VERBOSITY_LEVEL<level) return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsprintf(buffer, format, args);
    fprintf(stdout, "%20s @ %d%30s()\t%8dms\t%s%s\n", file, line, function, SDL_GetTicks(), levels[level], buffer);
    va_end(args);
}
#endif

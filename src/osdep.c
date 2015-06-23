#include "osdep.h"

#ifdef _WIN32

#include <windows.h>
#include <shlwapi.h>

// fprintf_w32 is non trivial and defined at src/print_w32.c

int getpagesize_w32(void) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

int getnumcores_ag() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

inline int fnmatch_path_ag(const char *pattern, const char *string) {
    return !PathMatchSpec(pattern, string);
}

#else /* not _WIN32 */

#include <unistd.h>
#include <fnmatch.h>

int getnumcores_ag() {
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

inline int fnmatch_path_ag(const char *pattern, const char *string) {
    return fnmatch(pattern, string, FNM_PATHNAME);
}

#endif

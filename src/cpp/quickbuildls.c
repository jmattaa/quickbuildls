#include "quickbuildls.h"
#include <string.h>
#include <sys/types.h>

#define KEWORDS(_X)                                                            \
    _X("run")                                                                  \
    _X("run_parallel")                                                         \
    _X("depends")                                                              \
    _X("depends_parallel")

uint is_keyword(const char *s)
{
#define X(k)                                                                   \
    if (strcmp(s, k) == 0)                                                     \
        return 1;
    KEWORDS(X)
#undef X
    return 0;
}

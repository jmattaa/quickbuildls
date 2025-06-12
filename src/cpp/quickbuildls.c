#include "quickbuildls.h"
#include <string.h>
#include <sys/types.h>

#define KEYWORDS(_X)                                                           \
    _X("run", "## Run\n---\nThe command this task should run")                 \
    _X("run_parallel", "## Run Parallel\n---\nA boolean that specifies if "    \
                       "this task should run in parallel")                     \
    _X("depends", "## Depends\n---\nThe list of tasks that should be run "     \
                  "before the current task")                                   \
    _X("depends_parallel",                                                     \
       "## Depends Parallel\n---\nA boolean that specifies if the "            \
       "dependencies should run in parallel")

uint is_keyword(const char *s)
{
#define X(k, _)                                                                \
    if (strcmp(s, k) == 0)                                                     \
        return 1;
    KEYWORDS(X)
#undef X
    return 0;
}

const char *get_keyword_desc(const char *K)
{
#define X(k, d)                                                                \
    if (strcmp(K, k) == 0)                                                     \
        return d;
    KEYWORDS(X)
#undef X
    return "";
}

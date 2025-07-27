#include "quickbuildls.h"
#include <string.h>
#include <sys/types.h>

#define X(k, _) k,
const char *task_keyword_names[KEYWORDS_LEN] = {KEYWORDS(X)};
#undef X

uint8_t is_keyword(const char *s)
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

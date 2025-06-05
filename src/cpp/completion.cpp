// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "completion.h"
#include "quickbuildls.hpp"
#include <cstddef>
#include <cstdlib>

extern "C" quickbuildls_completion *get_completions(const char *csrc, int l,
                                                    int c, size_t *count)
{
    quickbuildls_completion *q =
        (quickbuildls_completion *)malloc(sizeof(quickbuildls_completion));
    if (q == NULL)
        return NULL;

    *count = 1;

    q->label = mkstr("test");
    q->detail = mkstr("this is a test and this is the label for the test");
    q->kind = qls_compkind_Variable;

    q->documentation =
        (decltype(q->documentation))malloc(sizeof(*q->documentation));
    if (q->documentation == NULL)
    {
        if (q->label != NULL)
            free((void *)q->label);
        if (q->detail != NULL)
            free((void *)q->detail);
        free(q);
        return NULL;
    }

    q->documentation->kind = mkstr("markdown");
    q->documentation->value =
        mkstr("## this is a markdown test\n---\ntesting markdown");

    return q;
}

extern "C" void free_completion(quickbuildls_completion *q, size_t c)
{
    if (q == NULL)
        return;
    for (size_t i = 0; i < c; i++)
    {
        if (q[i].label != NULL)
            free((void *)q[i].label);
        if (q[i].detail != NULL)
            free((void *)q[i].detail);
        if (q[i].documentation->kind != NULL)
            free((void *)q[i].documentation->kind);
        if (q[i].documentation->value != NULL)
            free((void *)q[i].documentation->value);
        if (q[i].documentation != NULL)
            free(q[i].documentation);
    }
    free(q);
}

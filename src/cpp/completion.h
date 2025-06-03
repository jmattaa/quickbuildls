// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#ifndef QUICKBUILDLS_COMPLETION_H
#define QUICKBUILDLS_COMPLETION_H

#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stddef.h>
#endif

    typedef struct completion
    {
        const char *label;
        const char *detail;
        struct
        {
            const char *kind;
            const char *value;
        } *documentation;
    } quickbuildls_completion;

    quickbuildls_completion *get_completions(const char *csrc, int l, int c,
                                             size_t *count);
    
    void free_completion(quickbuildls_completion *q, size_t c);

#ifdef __cplusplus
}
#endif

#endif

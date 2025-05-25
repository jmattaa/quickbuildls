// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#ifndef QUICKBUILDLS_HOVER_H
#define QUICKBUILDLS_HOVER_H

#ifdef __cplusplus
extern "C"
{
#endif

    const char *get_hover_md(const char *csrc, int l, int c);
    void hover_md_free(const char *md);

#ifdef __cplusplus
}
#endif

#endif

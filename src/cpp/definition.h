// this code does NOT come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#ifndef QUICKBUILDLS_DEFINITION_H
#define QUICKBUILDLS_DEFINITION_H

#ifdef __cplusplus
extern "C"
{
#endif

    void get_definition(const char *csrc, int l, int c, int *tol, int *toc);

#ifdef __cplusplus
}
#endif

#endif

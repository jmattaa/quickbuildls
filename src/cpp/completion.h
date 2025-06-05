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

    enum completion_kind
    {
        qls_compkind_Text = 1,
        qls_compkind_Method = 2,
        qls_compkind_Function = 3,
        qls_compkind_Constructor = 4,
        qls_compkind_Field = 5,
        qls_compkind_Variable = 6,
        qls_compkind_Class = 7,
        qls_compkind_Interface = 8,
        qls_compkind_Module = 9,
        qls_compkind_Property = 10,
        qls_compkind_Unit = 11,
        qls_compkind_Value = 12,
        qls_compkind_Enum = 13,
        qls_compkind_Keyword = 14,
        qls_compkind_Snippet = 15,
        qls_compkind_Color = 16,
        qls_compkind_File = 17,
        qls_compkind_Reference = 18,
        qls_compkind_Folder = 19,
        qls_compkind_EnumMember = 20,
        qls_compkind_Constant = 21,
        qls_compkind_Struct = 22,
        qls_compkind_Event = 23,
        qls_compkind_Operator = 24,
        qls_compkind_TypeParameter = 25,
    };

    typedef struct completion
    {
        const char *label;
        enum completion_kind kind; 
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

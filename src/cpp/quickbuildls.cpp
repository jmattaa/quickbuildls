// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "quickbuildls.hpp"

#include <string>

const char *mkstr(const std::string &s)
{
    char *cstr = (char *)std::malloc(s.size() + 1);
    if (!cstr)
        return nullptr;
    std::memcpy(cstr, s.c_str(), s.size() + 1);
    return cstr;
}

size_t line_char_to_offset(const std::string &source, int line, int character)
{
    size_t offset = 0;
    int current_line = 0;
    while (offset < source.size())
    {
        if (current_line == line)
            break;
        if (source[offset] == '\n')
            current_line++;
        offset++;
    }
    return offset + character;
}

size_t get_origin_index(const Origin &origin)
{
    if (std::holds_alternative<InputStreamPos>(origin))
        return std::get<InputStreamPos>(origin).index;
    if (std::holds_alternative<ObjectReference>(origin))
        return std::get<ObjectReference>(origin).size();

    return SIZE_MAX;
}

// cuz the offset is at the end of the decl for a task or a feild so the range
// should be (offset - size <= x < offset)
// note the <= in the begining and < in the end!
bool in_range(size_t x, size_t offset, std::string content)
{
    if (offset < content.size() || offset == SIZE_MAX)
        return false;

    return offset - content.size() <= x && x < offset;
}

void get_origin_line_char(const Origin &origin, const std::string &src,
                          int *line, int *character)
{
    const size_t idx = get_origin_index(origin);
    *line = 0;
    *character = 0;
    for (size_t i = 0; i < idx; i++)
    {
        if (src[i] == '\n')
        {
            (*line)++;
            *character = 0;
        }
        else
            (*character)++;
    }
}

#define KEWORDS(_X)                                                            \
    _X("run")                                                                  \
    _X("run_parallel")                                                         \
    _X("depends")                                                              \
    _X("depends_parallel")

bool is_keyword(const std::string &s)
{
#define X(k)                                                                   \
    if (k == s)                                                                \
        return true;
    KEWORDS(X)
#undef X
    return false;
}

// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "hover.h"
#include "lexer.hpp"
#include "parser.hpp"

#include <string>

static size_t get_origin_index(const Origin &origin)
{
    return std::visit(
        [](auto const &val) -> size_t
        {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, InputStreamPos>)
                return val.index;
            else
                return SIZE_MAX;
        },
        origin);
}

// cuz the offset is at the end of the decl for a task or a feild so the range
// should be (offset - size <= x < offset)
// note the <= in the begining and < in the end!
static bool in_range(size_t x, const Origin &origin, std::string content)
{
    size_t offset = get_origin_index(origin);
    if (offset < content.size())
        return false; // ain't sure if we'll ever reach here

    return offset != SIZE_MAX && offset - content.size() <= x && x < offset;
}

static size_t line_char_to_offset(const std::string &source, int line,
                                  int character)
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

// this should prolly be optimized cuz idk it parses the whole file each time
// we want to do a hover????
extern "C" const char *get_hover_md(const char *csrc, int l, int c)
{
    std::string source(csrc);
    size_t boffset = line_char_to_offset(source, l, c);

    std::vector<unsigned char> vecsrc(source.begin(), source.end());
    Lexer lexer(vecsrc);

    std::vector<Token> token_stream = lexer.get_token_stream();
    Parser parser(token_stream);

    AST ast(parser.parse_tokens());

    auto mkstr = [](const std::string &s)
    {
        char *cstr = (char *)std::malloc(s.size() + 1);
        std::memcpy(cstr, s.c_str(), s.size() + 1);
        return cstr;
    };

    for (const Field &f : ast.fields)
        if (in_range(boffset, f.origin, f.identifier.content))
            return mkstr("**Variable:** `" + f.identifier.content + "`");

    for (const Task &t : ast.tasks)
        if (in_range(boffset, t.origin, t.iterator.content))
            return mkstr("**Task:** `" + t.iterator.content + "`");

    return NULL;
}

extern "C" void hover_md_free(const char *md)
{
    if (md)
        std::free((void *)md);
}

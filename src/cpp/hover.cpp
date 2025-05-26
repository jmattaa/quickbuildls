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
    if (std::holds_alternative<InputStreamPos>(origin))
        return std::get<InputStreamPos>(origin).index;
    if (std::holds_alternative<ObjectReference>(origin))
        return std::get<ObjectReference>(origin).size();

    return SIZE_MAX;
}

// cuz the offset is at the end of the decl for a task or a feild so the range
// should be (offset - size <= x < offset)
// note the <= in the begining and < in the end!
static bool in_range(size_t x, const Origin &origin, std::string content)
{
    size_t offset = get_origin_index(origin);
    if (offset < content.size() || offset == SIZE_MAX)
        return false;

    return offset - content.size() <= x && x < offset;
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

struct ASTVisitContent
{
    std::string operator()(Identifier const &id) { return id.content; }
    std::string operator()(Literal const &li) { return li.content; }
    std::string operator()(FormattedLiteral const &fl)
    {
        if (fl.contents.empty())
            return "";
        return std::visit(ASTVisitContent{}, fl.contents.front());
    }

    std::string operator()(List const &l)
    {
        if (l.contents.empty())
            return "";
        std::string res;
        for (auto &item : l.contents)
            res +=
                (res.empty() ? "" : ", ") + std::visit(ASTVisitContent{}, item);
        return res;
    }

    std::string operator()(Boolean const &b)
    {
        return b.content ? "true" : "false";
    }

    // idk what this is tho
    std::string operator()(Replace const &r)
    {
        if (!r.replacement)
            return "";
        return std::visit(ASTVisitContent{}, *r.replacement);
    }
};

struct ASTVisitOrigin
{
    Origin operator()(Identifier const &i) { return i.origin; }
    Origin operator()(Literal const &literal) { return literal.origin; }
    Origin operator()(FormattedLiteral const &fl) { return fl.origin; }
    Origin operator()(List const &l) { return l.origin; }
    Origin operator()(Boolean const &b) { return b.origin; }
    Origin operator()(Replace const &r) { return r.origin; }
};

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
            return mkstr("### Variable: `" + f.identifier.content + "`");

    for (const Task &t : ast.tasks)
    {
        std::string tdependencies;

        for (const Field &f : t.fields)
        {
            if (f.identifier.content == "depends")
            {
                tdependencies = std::visit(ASTVisitContent{}, f.expression);
                if (in_range(boffset, f.origin, f.identifier.content))
                    return mkstr("### Dependencies: `" + tdependencies + "`" +
                                 "\n---" + "\n" +
                                 "the dependency list is the lists of tasks "
                                 "that should be run before this task.");
            }
            else if (f.identifier.content == "run")
            {
                if (in_range(boffset, f.origin, f.identifier.content))
                    return mkstr(
                        "### Run \n---\nThe command this task will run");
            }

            if (in_range(boffset, f.origin, f.identifier.content))
                return mkstr("### Field: `" + f.identifier.content + "`");
        }

        const std::string &tname = std::visit(ASTVisitContent{}, t.identifier);
        if (in_range(boffset, t.origin, tname))
        {
            return mkstr("### Task: `" + tname + "`\n---" +
                         (tdependencies.empty()
                              ? ""
                              : ("\ndepends on: `" + tdependencies + "`")));
        }
    }

    return NULL;
}

extern "C" void hover_md_free(const char *md)
{
    if (md)
        std::free((void *)md);
}

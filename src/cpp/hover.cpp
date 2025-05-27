// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "hover.h"
#include "lexer.hpp"
#include "parser.hpp"
#include "quickbuildls.hpp"

#include <string>

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

    size_t offset;
    for (const Field &f : ast.fields)
    {
        offset = get_origin_index(f.origin);
        if (in_range(boffset, offset, f.identifier.content))
            return mkstr("### Variable: `" + f.identifier.content + "`");
    }

    for (const Task &t : ast.tasks)
    {
        std::string tdependencies;

        for (const Field &f : t.fields)
        {
            if (f.identifier.content == "depends")
            {
                tdependencies = std::visit(ASTVisitContent{}, f.expression);
                offset = get_origin_index(f.origin);
                if (in_range(boffset, offset, f.identifier.content))
                    return mkstr("### Dependencies: `" + tdependencies + "`" +
                                 "\n---" + "\n" +
                                 "the dependency list is the lists of tasks "
                                 "that should be run before this task.");
            }
            else if (f.identifier.content == "run")
            {
                offset = get_origin_index(f.origin);
                if (in_range(boffset, offset, f.identifier.content))
                    return mkstr(
                        "### Run \n---\nThe command this task will run");
            }

            offset = get_origin_index(f.origin);
            if (in_range(boffset, offset, f.identifier.content))
                return mkstr("### Field: `" + f.identifier.content + "`");
        }

        const std::string &tname = std::visit(ASTVisitContent{}, t.identifier);
        offset = get_origin_index(t.origin);

        if (source[offset - tname.size() - 1] != '\n')
            offset -= 1; // to fix the offset thingy for tasks that start with "

        if (in_range(boffset, offset, tname))
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

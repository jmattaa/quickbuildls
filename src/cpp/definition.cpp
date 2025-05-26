// this code does not come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "definition.h"
#include "lexer.hpp"
#include "parser.hpp"
#include "quickbuildls.hpp"

extern "C" void get_definition(const char *csrc, int l, int c, int *tol,
                               int *toc)
{
    // -1 not found
    // this is so we can do early returns
    *tol = -1;
    *toc = -1;

    std::string source(csrc);
    size_t boffset = line_char_to_offset(source, l, c);

    // is the text under the cursor
    std::string ident;

    if (boffset >= source.size())
        return;

    // get the identifier under the cursor, not using the lexer because
    // i couldn't get it to work like i wanted because i need to get stuff from
    // inside of formatted strings
    // using the is_alphabetic function that the lexer uses though
    size_t start = boffset;
    while (start > 0 && is_alphabetic(source[start - 1]))
        --start;
    size_t end = boffset;
    while (end < source.size() && is_alphabetic(source[end]))
        ++end;
    if (start < end)
        ident = source.substr(start, end - start);

    if (ident.empty())
        return;

    // check that ident isn't a keyword
    if (is_keyword(ident))
        return;

    std::vector<unsigned char> vecsrc(source.begin(), source.end());
    Lexer lexer(vecsrc);

    std::vector<Token> token_stream = lexer.get_token_stream();
    Parser parser(token_stream);

    AST ast(parser.parse_tokens());

    // find the definition of the identifier
    for (const Field &f : ast.fields)
        if (f.identifier.content == ident)
        {
            int origintol, origintoc;
            get_origin_line_char(f.origin, source, &origintol, &origintoc);

            *tol = origintol;
            *toc = origintoc;

            return;
        }

    // check if is a task or in a task
    for (const Task &t : ast.tasks)
    {
        const std::string &tname = std::visit(ASTVisitContent{}, t.identifier);
        if (tname == ident)
        {
            int origintol, origintoc;
            get_origin_line_char(t.origin, source, &origintol, &origintoc);

            *tol = origintol;
            *toc = origintoc;

            return;
        }

        // check the fields
        for (const Field &f : t.fields)
            if (f.identifier.content == ident)
            {
                int origintol, origintoc;
                get_origin_line_char(f.origin, source, &origintol, &origintoc);

                *tol = origintol;
                *toc = origintoc;

                return;
            }
    }
}

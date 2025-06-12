// this code does NOT come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#ifndef QUICKBUILDLS_H
#define QUICKBUILDLS_H

#include "lexer.hpp"
#include "parser.hpp"
#include <string>

size_t get_origin_index(const Origin &origin);

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

#endif

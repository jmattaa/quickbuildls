// this code does NOT come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#ifndef QUICKBUILDLS_H
#define QUICKBUILDLS_H

#include "quickbuild/parser.hpp"
#include <string>
#include <variant>

struct ASTVisitContent
{
    std::string operator()(Identifier const &id) { return id.content; }
    std::string operator()(Literal const &li) { return li.content; }
    std::string operator()(FormattedLiteral const &fl)
    {
        if (fl.contents.empty())
            return "";
        std::string res;
        for (auto &item : fl.contents)
        {
            if (std::holds_alternative<Identifier>(item))
                res += "[" + std::visit(ASTVisitContent{}, item) + "]";
            else
                res += std::visit(ASTVisitContent{}, item);
        }
        return res;
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

    std::string operator()(Replace const &r)
    {
        std::string res = "";
        if (r.input)
        {
            res += std::visit(ASTVisitContent{}, *(r.input));
            res += ": ";
        }
        if (r.filter)
        {
            res += std::visit(ASTVisitContent{}, *(r.filter));
            res += " -> ";
        }
        if (r.product)
            res += std::visit(ASTVisitContent{}, *(r.product));

        return res;
    }
};

#endif

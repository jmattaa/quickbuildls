// MIT License
//
// Copyright (c) 2024 Jake
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// this code is from https://github.com/nordtechtiger/quickbuild

#ifndef PARSER_H
#define PARSER_H

#include "lexer.hpp"
#include "tracking.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>

struct Identifier;
struct Literal;
struct FormattedLiteral;
struct List;
struct Boolean;
struct Replace;
using ASTObject =
    std::variant<Identifier, Literal, FormattedLiteral, List, Boolean, Replace>;

// Logic: Expressions
struct Identifier
{
    std::string content;
    StreamReference reference;
    bool operator==(Identifier const &other) const;
    // Identifier() = delete;
};
struct Literal
{
    std::string content;
    StreamReference reference;
    bool operator==(Literal const &other) const;
    // Literal() = delete;
};
struct FormattedLiteral
{
    std::vector<ASTObject> contents;
    StreamReference reference;
    bool operator==(FormattedLiteral const &other) const;
    // FormattedLiteral() = delete;
};
struct Boolean
{
    bool content;
    StreamReference reference;
    bool operator==(Boolean const &other) const;
    // Boolean() = delete;
};
struct List
{
    std::vector<ASTObject> contents;
    StreamReference reference;
    bool operator==(List const &other) const;
    // List() = delete;
};
struct Replace
{
    std::shared_ptr<ASTObject> input;
    std::shared_ptr<ASTObject> filter;
    std::shared_ptr<ASTObject> product;
    StreamReference reference;
    bool operator==(Replace const &other) const;
    // Replace() = delete;
};

// visitor that simply returns the origin of an AST object.
struct ASTVisitReference
{
    StreamReference operator()(Identifier const &identifier)
    {
        return identifier.reference;
    }
    StreamReference operator()(Literal const &literal)
    {
        return literal.reference;
    }
    StreamReference operator()(FormattedLiteral const &formatted_literal)
    {
        return formatted_literal.reference;
    }
    StreamReference operator()(List const &list) { return list.reference; }
    StreamReference operator()(Boolean const &boolean)
    {
        return boolean.reference;
    }
    StreamReference operator()(Replace const &replace)
    {
        return replace.reference;
    }
};

// Config: Fields, tasks, AST
struct Field
{
    Identifier identifier;
    ASTObject expression;
    StreamReference reference;
    bool operator==(Field const &other) const;
    // Field() = delete;
};
struct Task
{
    ASTObject identifier;
    Identifier iterator;
    std::vector<Field> fields;
    StreamReference reference;
    bool operator==(Task const &other) const;
    // Task() = delete;
};
struct AST
{
    std::vector<Field> fields;
    std::vector<Task> tasks;
    // make the copy constructor explicit to emphasize performance.
    explicit AST(AST const &) = default;
    AST() = default;
};

// Work class
class Parser
{
  private:
    std::vector<Token> m_token_stream;
    AST m_ast;

    size_t m_index;
    std::optional<Token> m_current;
    std::optional<Token> m_next;

    std::optional<Token> consume_token();
    std::optional<Token> consume_token(int n);
    std::optional<Token> consume_if(TokenType token_type);
    bool check_current(TokenType token_type);
    bool check_next(TokenType token_type);

    std::optional<ASTObject> parse_ast_object();
    std::optional<ASTObject> parse_list();
    std::optional<ASTObject> parse_replace();
    std::optional<ASTObject> parse_primary();
    std::optional<Field> parse_field();
    std::optional<Task> parse_task();

  public:
    Parser(std::vector<Token> token_stream);
    AST parse_tokens();
};

#endif

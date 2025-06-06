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
    Origin origin;

    bool operator==(Identifier const &other) const;
};
struct Literal
{
    std::string content;
    Origin origin;

    bool operator==(Literal const &other) const;
};
struct FormattedLiteral
{
    std::vector<ASTObject> contents;
    Origin origin;

    bool operator==(FormattedLiteral const &other) const;
};
struct Boolean
{
    bool content;
    Origin origin;

    bool operator==(Boolean const &other) const;
};
struct List
{
    std::vector<ASTObject> contents;
    Origin origin;

    bool operator==(List const &other) const;
};
struct Replace
{
    std::shared_ptr<ASTObject> identifier;
    std::shared_ptr<ASTObject> original;
    std::shared_ptr<ASTObject> replacement;
    Origin origin;

    bool operator==(Replace const &other) const;
};

// Config: Fields, tasks, AST
struct Field
{
    Identifier identifier;
    ASTObject expression;
    Origin origin;

    auto operator==(Field const &other) const
    {
        return this->identifier == other.identifier &&
               this->expression == other.expression;
    }
};
struct Task
{
    ASTObject identifier;
    Identifier iterator;
    std::vector<Field> fields;
    Origin origin;

    auto operator==(Task const &other) const
    {
        return this->identifier == other.identifier &&
               this->iterator == other.iterator && this->fields == other.fields;
    }
};
struct AST
{
    std::vector<Field> fields;
    std::vector<Task> tasks;
    // delete the copy constructor to emphasize performance.
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
    Token m_previous;
    Token m_current;
    Token m_next;

    Token consume_token();
    Token consume_token(int n);
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

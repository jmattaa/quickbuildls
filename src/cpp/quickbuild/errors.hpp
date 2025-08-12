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

#ifndef ERRORS_H
#define ERRORS_H

class BuildError;

#include "lexer.hpp"
#include "parser.hpp"
#include "tracking.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

class BuildError
{
  public:
    virtual char const *get_exception_msg() = 0;
    virtual ~BuildError() = default;
};

// class ENoFieldNorDefault : public BuildError {
// private:
//   std::string field_name;
//
// public:
//   std::string render_error(std::vector<unsigned char> config) override;
//   char const *get_exception_msg() override;
//   ENoFieldNorDefault() = delete;
//   ENoFieldNorDefault(std::string);
// };

class EWithStreamReference
{
  public:
    virtual const StreamReference &get_reference() const = 0;
    virtual ~EWithStreamReference() = default;
};

class EInvalidSymbol : public BuildError, public EWithStreamReference
{
  private:
    std::string symbol;

  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidSymbol() = delete;
    EInvalidSymbol(StreamReference, std::string);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class EInvalidGrammar : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidGrammar() = delete;
    EInvalidGrammar(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class EInvalidLiteral : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidLiteral() = delete;
    EInvalidLiteral(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoValue : public BuildError
{
  private:
    Identifier identifier;

  public:
    char const *get_exception_msg() override;
    ENoValue() = delete;
    ENoValue(Identifier);
};

class ENoLinestop : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoLinestop() = delete;
    ENoLinestop(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoIterator : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoIterator() = delete;
    ENoIterator(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoTaskOpen : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoTaskOpen() = delete;
    ENoTaskOpen(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoTaskClose : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoTaskClose() = delete;
    ENoTaskClose(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

// todo: revisit this.
class EInvalidListEnd : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidListEnd() = delete;
    EInvalidListEnd(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoReplacementIdentifier : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoReplacementIdentifier() = delete;
    ENoReplacementIdentifier(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoReplacementOriginal : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoReplacementOriginal() = delete;
    ENoReplacementOriginal(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoReplacementArrow : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoReplacementArrow() = delete;
    ENoReplacementArrow(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoReplacementReplacement : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoReplacementReplacement() = delete;
    ENoReplacementReplacement(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class EInvalidEscapedExpression : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidEscapedExpression() = delete;
    EInvalidEscapedExpression(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ENoExpressionClose : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENoExpressionClose() = delete;
    ENoExpressionClose(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class EEmptyExpression : public BuildError, public EWithStreamReference
{
  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EEmptyExpression() = delete;
    EEmptyExpression(StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class EInvalidEscapeCode : public BuildError, public EWithStreamReference
{
  private:
    unsigned char code;

  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidEscapeCode() = delete;
    EInvalidEscapeCode(unsigned char, StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

// api-facing error handler.
class ErrorHandler
{
  private:
    // we have multiple threads running, and thus more than one
    // error may be reported for a single build pass.
    static std::unordered_map<size_t, std::shared_ptr<BuildError>> error_state;
    static std::mutex error_lock;

  public:
    template <typename B> static void halt [[noreturn]] (B build_error);
    template <typename B> static void soft_report(B build_error);
    static std::unordered_map<size_t, std::shared_ptr<BuildError>> get_errors();
};

// internal exception.
class BuildException : public std::exception
{
  private:
    const char *details;

  public:
    BuildException(const char *details) : details(details) {}
    const char *what() const noexcept override { return details; };
};

#endif

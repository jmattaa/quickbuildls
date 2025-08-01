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

struct ReferenceView;
class BuildError;
class Frame;

#include "lexer.hpp"
#include "parser.hpp"
#include "tracking.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

struct ReferenceView
{
    std::string line_prefix;
    std::string line_ref;
    std::string line_suffix;
    size_t line_num;
};

class BuildError
{
  public:
    virtual char const *get_exception_msg() = 0;
    virtual ~BuildError() = default;
};

class ENoMatchingIdentifier : public BuildError
{
  private:
    Identifier identifier;

  public:
    char const *get_exception_msg() override;
    ENoMatchingIdentifier() = delete;
    ENoMatchingIdentifier(Identifier);
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

class ENonZeroProcess : public BuildError, public EWithStreamReference
{
  private:
    std::string cmdline;

  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    ENonZeroProcess() = delete;
    ENonZeroProcess(std::string, StreamReference);

    virtual const StreamReference &get_reference() const override
    {
        return reference;
    };
};

class ETaskNotFound : public BuildError
{
  private:
    std::string task_name;

  public:
    char const *get_exception_msg() override;
    ETaskNotFound() = delete;
    ETaskNotFound(std::string);
};

class ENoTasks : public BuildError
{
  public:
    char const *get_exception_msg() override;
};

class EAmbiguousTask : public BuildError
{
  private:
    Task task;

  public:
    char const *get_exception_msg() override;
    EAmbiguousTask() = delete;
    EAmbiguousTask(Task);
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

class EInvalidInputFile : public BuildError
{
  private:
    std::string path;

  public:
    char const *get_exception_msg() override;
    EInvalidInputFile() = delete;
    EInvalidInputFile(std::string);
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

class ERecursiveVariable : public BuildError
{
  private:
    Identifier identifier;

  public:
    char const *get_exception_msg() override;
    ERecursiveVariable() = delete;
    ERecursiveVariable(Identifier);
};

class ERecursiveTask : public BuildError
{
  private:
    Task task;
    std::string dependency_value;

  public:
    char const *get_exception_msg() override;
    ERecursiveTask() = delete;
    ERecursiveTask(Task, std::string);
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
    static void trigger_report [[noreturn]] ();
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

// api-facing context stack getter.
class ContextStack
{
    friend class FrameGuard;

  private:
    // thread hash, frames
    static std::unordered_map<size_t, std::vector<std::shared_ptr<Frame>>>
        stack;
    static std::mutex stack_lock;
    static bool frozen;

  public:
    static std::unordered_map<size_t, std::vector<std::shared_ptr<Frame>>>
    dump_stack();
    static std::vector<std::shared_ptr<Frame>> export_local_stack();
    static void import_local_stack(std::vector<std::shared_ptr<Frame>>);

    static void freeze();
    static bool is_frozen();
};

#endif

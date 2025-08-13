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

#include "parser.hpp"
#include "tracking.hpp"
#include <memory>
#include <mutex>
#include <string>
#include "../errors.h"

class BuildError
{
  public:
    virtual char const *get_exception_msg() = 0;
    virtual ~BuildError() = default;
    ErrorCode get_code() const;
};

class EWithStreamReference
{
  public:
    virtual const StreamReference &get_reference() const = 0;
    virtual ~EWithStreamReference() = default;
};

#define _X(name, ...)                                                          \
    class name : public BuildError, public EWithStreamReference                \
    {                                                                          \
      public:                                                                  \
        StreamReference reference;                                             \
        char const *get_exception_msg() override;                              \
        name() = delete;                                                       \
        name(StreamReference);                                                 \
        ErrorCode get_code() const;                                            \
                                                                               \
        virtual const StreamReference &get_reference() const override          \
        {                                                                      \
            return reference;                                                  \
        };                                                                     \
    };
E_STANDARD_ITER(_X)
#undef _X

class EInvalidSymbol : public BuildError, public EWithStreamReference
{
  private:
    std::string symbol;

  public:
    StreamReference reference;
    char const *get_exception_msg() override;
    EInvalidSymbol() = delete;
    EInvalidSymbol(StreamReference, std::string);
    ErrorCode get_code() const;

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
    ErrorCode get_code() const;
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
    ErrorCode get_code() const;

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

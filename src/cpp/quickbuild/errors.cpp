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
//
// "It may not always match the exact code on the upstream repo, and some files
// will be modified"
// - Jonathan Matta
// What a quote

#include "errors.hpp"
#include "tracking.hpp"
#include <cassert>
#include <memory>
#include <thread>

#define MAKE_CONSTRUCTOR(name, ...)                                            \
    name::name(StreamReference reference) { this->reference = reference; }
E_STANDARD_ITER(MAKE_CONSTRUCTOR)
#undef MAKE_CONSTRUCTOR

#define GET_EXPECTION_MSG(name, msg, ...)                                      \
    char const *name::get_exception_msg() { return msg; }
E_STANDARD_ITER(GET_EXPECTION_MSG)
#undef GET_EXPECTION_MSG

EInvalidSymbol::EInvalidSymbol(StreamReference reference, std::string symbol)
{
    this->reference = reference;
    this->symbol = symbol;
}
char const *EInvalidSymbol::get_exception_msg() { return "Invalid symbol"; }

ENoValue::ENoValue(Identifier identifier) { this->identifier = identifier; }
char const *ENoValue::get_exception_msg() { return "Invalid value"; }

EInvalidEscapeCode::EInvalidEscapeCode(unsigned char code,
                                       StreamReference reference)
{
    this->code = code;
    this->reference = reference;
}
char const *EInvalidEscapeCode::get_exception_msg()
{
    return "Invalid escape code";
}

std::unordered_map<size_t, std::shared_ptr<BuildError>>
    ErrorHandler::error_state = {};
std::mutex ErrorHandler::error_lock;

std::unordered_map<size_t, std::shared_ptr<BuildError>>
ErrorHandler::get_errors()
{
    return error_state;
};

template <typename B> void ErrorHandler::halt [[noreturn]] (B build_error)
{
    std::thread::id thread_id = std::this_thread::get_id();
    size_t thread_hash = std::hash<std::thread::id>{}(thread_id);

    std::unique_lock<std::mutex> guard(ErrorHandler::error_lock);
    ErrorHandler::error_state[thread_hash] = std::make_unique<B>(build_error);
    throw BuildException(build_error.get_exception_msg());
}

template <typename B> void ErrorHandler::soft_report(B build_error)
{
    std::thread::id thread_id = std::this_thread::get_id();
    size_t thread_hash = std::hash<std::thread::id>{}(thread_id);

    std::unique_lock<std::mutex> guard(ErrorHandler::error_lock);
    ErrorHandler::error_state[thread_hash] = std::make_unique<B>(build_error);
}

#define _X(name, ...)                                                          \
    template void ErrorHandler::halt<name>(name);                              \
    template void ErrorHandler::soft_report<name>(name);
E_STANDARD_ITER(_X)
#undef _X

template void ErrorHandler::halt<EInvalidSymbol>(EInvalidSymbol);
template void ErrorHandler::halt<ENoValue>(ENoValue);
template void ErrorHandler::halt<EInvalidEscapeCode>(EInvalidEscapeCode);
template void ErrorHandler::soft_report<EInvalidSymbol>(EInvalidSymbol);
template void ErrorHandler::soft_report<ENoValue>(ENoValue);
template void ErrorHandler::soft_report<EInvalidEscapeCode>(EInvalidEscapeCode);

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

#include "errors.hpp"

std::vector<ErrorInfo> ErrorHandler::error_stack = {};

// push an error onto the stack so that it can be traced later.
void ErrorHandler::push_error(ErrorContext context, ErrorCode error_code)
{
    std::string message = _ERROR_LOOKUP_TABLE.at(error_code);
    ErrorInfo error_info = {
        context,
        error_code,
        message,
    };
    error_lock.lock();
    error_stack.push_back(error_info);
    error_lock.unlock();
}

// push an error and abandon ship.
void ErrorHandler::push_error_throw(ErrorContext context, ErrorCode error_code)
{
    std::string message = _ERROR_LOOKUP_TABLE.at(error_code);
    ErrorInfo error_info = {
        context,
        error_code,
        message,
    };
    error_lock.lock();
    error_stack.push_back(error_info);
    error_lock.unlock();
    throw BuildException(message.c_str());
}

// get the latest error pushed to the stack.
std::optional<ErrorInfo> ErrorHandler::pop_error()
{
    if (error_stack.empty())
        return std::nullopt;
    ErrorInfo error_info = error_stack.back();
    error_stack.pop_back();
    return error_info;
}

std::optional<ErrorInfo> ErrorHandler::peek_error()
{
    if (error_stack.empty())
        return std::nullopt;
    return error_stack.back();
}

ErrorContext::ErrorContext(Origin const &origin)
{
    if (std::holds_alternative<InputStreamPos>(origin))
    {
        this->stream_pos = std::get<InputStreamPos>(origin);
        this->ref = std::nullopt;
    }
    else if (std::holds_alternative<ObjectReference>(origin))
    {
        this->stream_pos = std::nullopt;
        this->ref = std::get<ObjectReference>(origin);
    }
    else
    {
        this->stream_pos = std::nullopt;
        this->ref = std::nullopt;
    }
}

ErrorContext::ErrorContext(InternalNode const &)
{
    this->stream_pos = std::nullopt;
    this->ref = std::nullopt;
}

ErrorContext::ErrorContext(ObjectReference const &ref)
{
    this->stream_pos = std::nullopt;
    this->ref = ref;
}

ErrorContext::ErrorContext(Origin const &origin, ObjectReference const &ref)
{
    if (std::holds_alternative<InputStreamPos>(origin))
    {
        this->stream_pos = std::get<InputStreamPos>(origin);
        this->ref = std::nullopt;
    }
    else if (std::holds_alternative<ObjectReference>(origin))
    {
        this->stream_pos = std::nullopt;
        this->ref = std::get<ObjectReference>(origin);
    }
    else
    {
        this->stream_pos = std::nullopt;
        this->ref = std::nullopt;
    }
    this->ref = ref;
}

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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <format>
#include <iostream>
#include <memory>
#include <thread>
#include <variant>

EInvalidSymbol::EInvalidSymbol(StreamReference reference, std::string symbol)
{
    this->reference = reference;
    this->symbol = symbol;
}

char const *EInvalidSymbol::get_exception_msg()
{
    return std::format("Invalid symbol: {}", this->symbol).c_str();
}

EInvalidLiteral::EInvalidLiteral(StreamReference reference)
{
    this->reference = reference;
}

char const *EInvalidLiteral::get_exception_msg() { return "Invalid literal"; }

EInvalidGrammar::EInvalidGrammar(StreamReference reference)
{
    this->reference = reference;
}

char const *EInvalidGrammar::get_exception_msg() { return "Invalid grammar"; }

ENoValue::ENoValue(Identifier identifier) { this->identifier = identifier; }

char const *ENoValue::get_exception_msg() { return "No valid value"; }

ENoLinestop::ENoLinestop(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoLinestop::get_exception_msg()
{
    return "Missing semicolon at end of line";
}

ENoIterator::ENoIterator(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoIterator::get_exception_msg() { return "No task iterator"; }

ENoTaskOpen::ENoTaskOpen(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoTaskOpen::get_exception_msg()
{
    return "Missing opening curly bracket";
}

ENoTaskClose::ENoTaskClose(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoTaskClose::get_exception_msg()
{
    return "Missing closing curly bracket";
}

EInvalidListEnd::EInvalidListEnd(StreamReference reference)
{
    this->reference = reference;
}

char const *EInvalidListEnd::get_exception_msg()
{
    return "Incorrectly formatted list";
}

ENoReplacementIdentifier::ENoReplacementIdentifier(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementIdentifier::get_exception_msg()
{
    return "Missing variable name for replacement";
}

ENoReplacementOriginal::ENoReplacementOriginal(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementOriginal::get_exception_msg()
{
    return "Missing original value for replacement";
}

ENoReplacementArrow::ENoReplacementArrow(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementArrow::get_exception_msg()
{
    return "Missing the `->`";
}

ENoReplacementReplacement::ENoReplacementReplacement(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementReplacement::get_exception_msg()
{
    return "Missing a replacement value";
}

EInvalidEscapedExpression::EInvalidEscapedExpression(StreamReference reference)
{
    this->reference = reference;
}

char const *EInvalidEscapedExpression::get_exception_msg()
{
    return "Invalid escaped expression";
}

ENoExpressionClose::ENoExpressionClose(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoExpressionClose::get_exception_msg()
{
    return "Expression not closed";
}

EEmptyExpression::EEmptyExpression(StreamReference reference)
{
    this->reference = reference;
}

char const *EEmptyExpression::get_exception_msg() { return "Empty expression"; }

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

template void ErrorHandler::halt<EInvalidSymbol>(EInvalidSymbol);
template void ErrorHandler::halt<EInvalidLiteral>(EInvalidLiteral);
template void ErrorHandler::halt<EInvalidGrammar>(EInvalidGrammar);
template void ErrorHandler::halt<ENoValue>(ENoValue);
template void ErrorHandler::halt<ENoLinestop>(ENoLinestop);
template void ErrorHandler::halt<ENoIterator>(ENoIterator);
template void ErrorHandler::halt<ENoTaskOpen>(ENoTaskOpen);
template void ErrorHandler::halt<ENoTaskClose>(ENoTaskClose);
template void ErrorHandler::halt<EInvalidListEnd>(EInvalidListEnd);
template void
    ErrorHandler::halt<ENoReplacementIdentifier>(ENoReplacementIdentifier);
template void
    ErrorHandler::halt<ENoReplacementOriginal>(ENoReplacementOriginal);
template void ErrorHandler::halt<ENoReplacementArrow>(ENoReplacementArrow);
template void
    ErrorHandler::halt<ENoReplacementReplacement>(ENoReplacementReplacement);
template void
    ErrorHandler::halt<EInvalidEscapedExpression>(EInvalidEscapedExpression);
template void ErrorHandler::halt<ENoExpressionClose>(ENoExpressionClose);
template void ErrorHandler::halt<EEmptyExpression>(EEmptyExpression);
template void ErrorHandler::halt<EInvalidEscapeCode>(EInvalidEscapeCode);
template void ErrorHandler::soft_report<EInvalidSymbol>(EInvalidSymbol);
template void ErrorHandler::soft_report<EInvalidLiteral>(EInvalidLiteral);
template void ErrorHandler::soft_report<EInvalidGrammar>(EInvalidGrammar);
template void ErrorHandler::soft_report<ENoValue>(ENoValue);
template void ErrorHandler::soft_report<ENoLinestop>(ENoLinestop);
template void ErrorHandler::soft_report<ENoIterator>(ENoIterator);
template void ErrorHandler::soft_report<ENoTaskOpen>(ENoTaskOpen);
template void ErrorHandler::soft_report<ENoTaskClose>(ENoTaskClose);
template void ErrorHandler::soft_report<EInvalidListEnd>(EInvalidListEnd);
template void ErrorHandler::soft_report<ENoReplacementIdentifier>(
    ENoReplacementIdentifier);
template void
    ErrorHandler::soft_report<ENoReplacementOriginal>(ENoReplacementOriginal);
template void
    ErrorHandler::soft_report<ENoReplacementArrow>(ENoReplacementArrow);
template void ErrorHandler::soft_report<ENoReplacementReplacement>(
    ENoReplacementReplacement);
template void ErrorHandler::soft_report<EInvalidEscapedExpression>(
    EInvalidEscapedExpression);
template void ErrorHandler::soft_report<ENoExpressionClose>(ENoExpressionClose);
template void ErrorHandler::soft_report<EEmptyExpression>(EEmptyExpression);
template void ErrorHandler::soft_report<EInvalidEscapeCode>(EInvalidEscapeCode);

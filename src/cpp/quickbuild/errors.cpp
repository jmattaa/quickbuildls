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

std::unordered_map<size_t, std::vector<std::shared_ptr<Frame>>>
    ContextStack::stack = {};
std::mutex ContextStack::stack_lock;
bool ContextStack::frozen = false;

void ContextStack::freeze() { ContextStack::frozen = true; }
bool ContextStack::is_frozen() { return ContextStack::frozen; }
std::unordered_map<size_t, std::vector<std::shared_ptr<Frame>>>
ContextStack::dump_stack()
{
    return stack;
}
std::vector<std::shared_ptr<Frame>> ContextStack::export_local_stack()
{
    std::thread::id thread_id = std::this_thread::get_id();
    size_t thread_hash = std::hash<std::thread::id>{}(thread_id);
    std::unique_lock<std::mutex> guard(ContextStack::stack_lock);
    return ContextStack::stack[thread_hash];
}
void ContextStack::import_local_stack(
    std::vector<std::shared_ptr<Frame>> local_stack)
{
    std::thread::id thread_id = std::this_thread::get_id();
    size_t thread_hash = std::hash<std::thread::id>{}(thread_id);
    std::unique_lock<std::mutex> guard(ContextStack::stack_lock);
    ContextStack::stack[thread_hash] = local_stack;
}

// specific errors.
ENoMatchingIdentifier::ENoMatchingIdentifier(Identifier identifier)
{
    this->identifier = identifier;
}

char const *ENoMatchingIdentifier::get_exception_msg()
{
    return "No matching identifier";
}

ENonZeroProcess::ENonZeroProcess(std::string cmdline, StreamReference reference)
{
    this->cmdline = cmdline;
    this->reference = reference;
}

char const *ENonZeroProcess::get_exception_msg() { return "Command failed"; }

ETaskNotFound::ETaskNotFound(std::string task_name)
{
    this->task_name = task_name;
}

char const *ETaskNotFound::get_exception_msg() { return "Task not found"; }

char const *ENoTasks::get_exception_msg() { return "No tasks are defined"; }

EAmbiguousTask::EAmbiguousTask(Task task) { this->task = task; }

char const *EAmbiguousTask::get_exception_msg()
{
    return "Ambiguous topmost task";
}

EInvalidSymbol::EInvalidSymbol(StreamReference reference, std::string symbol)
{
    this->reference = reference;
    this->symbol = symbol;
}

char const *EInvalidSymbol::get_exception_msg() { return "Invalid symbol"; }

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

char const *ENoLinestop::get_exception_msg() { return "No linestop"; }

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
    return "No task open curly bracket";
}

ENoTaskClose::ENoTaskClose(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoTaskClose::get_exception_msg()
{
    return "No task close curly bracket";
}

EInvalidListEnd::EInvalidListEnd(StreamReference reference)
{
    this->reference = reference;
}

char const *EInvalidListEnd::get_exception_msg() { return "Invalid list end"; }

ENoReplacementIdentifier::ENoReplacementIdentifier(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementIdentifier::get_exception_msg()
{
    return "No replacement identifier";
}

ENoReplacementOriginal::ENoReplacementOriginal(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementOriginal::get_exception_msg()
{
    return "No replacement original";
}

ENoReplacementArrow::ENoReplacementArrow(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementArrow::get_exception_msg()
{
    return "No replacement arrow";
}

ENoReplacementReplacement::ENoReplacementReplacement(StreamReference reference)
{
    this->reference = reference;
}

char const *ENoReplacementReplacement::get_exception_msg()
{
    return "No replacement replacement";
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
    return "No expression close";
}

EEmptyExpression::EEmptyExpression(StreamReference reference)
{
    this->reference = reference;
}

char const *EEmptyExpression::get_exception_msg() { return "Empty expression"; }

EInvalidInputFile::EInvalidInputFile(std::string path) { this->path = path; }

char const *EInvalidInputFile::get_exception_msg()
{
    return "Invalid input file";
}

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

ERecursiveVariable::ERecursiveVariable(Identifier identifier)
{
    this->identifier = identifier;
}

char const *ERecursiveVariable::get_exception_msg()
{
    return "Recursive variable initialized";
}

ERecursiveTask::ERecursiveTask(Task task, std::string dependency_value)
{
    this->task = task;
    this->dependency_value = dependency_value;
}

char const *ERecursiveTask::get_exception_msg()
{
    return "Recursive task built";
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
    ContextStack::freeze();
    std::thread::id thread_id = std::this_thread::get_id();
    size_t thread_hash = std::hash<std::thread::id>{}(thread_id);

    std::unique_lock<std::mutex> guard(ErrorHandler::error_lock);
    ErrorHandler::error_state[thread_hash] = std::make_unique<B>(build_error);
    throw BuildException(build_error.get_exception_msg());
}

template <typename B> void ErrorHandler::soft_report(B build_error)
{
    ContextStack::freeze();
    std::thread::id thread_id = std::this_thread::get_id();
    size_t thread_hash = std::hash<std::thread::id>{}(thread_id);

    std::unique_lock<std::mutex> guard(ErrorHandler::error_lock);
    ErrorHandler::error_state[thread_hash] = std::make_unique<B>(build_error);
}

void ErrorHandler::trigger_report [[noreturn]] ()
{
    std::unique_lock<std::mutex> guard(ErrorHandler::error_lock);
    assert(ErrorHandler::error_state.size() > 0 &&
           "attempt to trigger a report on an empty error state");
    std::shared_ptr<BuildError> build_error =
        ErrorHandler::error_state.begin()->second;
    throw BuildException(build_error->get_exception_msg());
}

template void ErrorHandler::halt<ENoMatchingIdentifier>(ENoMatchingIdentifier);
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
template void ErrorHandler::halt<ENoTasks>(ENoTasks);
template void ErrorHandler::halt<ETaskNotFound>(ETaskNotFound);
template void ErrorHandler::halt<EAmbiguousTask>(EAmbiguousTask);
template void ErrorHandler::halt<ENonZeroProcess>(ENonZeroProcess);
template void ErrorHandler::halt<EInvalidInputFile>(EInvalidInputFile);
template void ErrorHandler::halt<EInvalidEscapeCode>(EInvalidEscapeCode);
template void ErrorHandler::halt<ERecursiveVariable>(ERecursiveVariable);
template void ErrorHandler::halt<ERecursiveTask>(ERecursiveTask);
template void
    ErrorHandler::soft_report<ENoMatchingIdentifier>(ENoMatchingIdentifier);
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
template void ErrorHandler::soft_report<ENoTasks>(ENoTasks);
template void ErrorHandler::soft_report<ETaskNotFound>(ETaskNotFound);
template void ErrorHandler::soft_report<EAmbiguousTask>(EAmbiguousTask);
template void ErrorHandler::soft_report<ENonZeroProcess>(ENonZeroProcess);
template void ErrorHandler::soft_report<EInvalidInputFile>(EInvalidInputFile);
template void ErrorHandler::soft_report<EInvalidEscapeCode>(EInvalidEscapeCode);
template void ErrorHandler::soft_report<ERecursiveVariable>(ERecursiveVariable);
template void ErrorHandler::soft_report<ERecursiveTask>(ERecursiveTask);

//
// ErrorContext::ErrorContext(Origin const &origin) {
//   if (std::holds_alternative<InputStreamPos>(origin)) {
//     this->stream_pos = std::get<InputStreamPos>(origin);
//     this->ref = std::nullopt;
//   } else if (std::holds_alternative<ObjectReference>(origin)) {
//     this->stream_pos = std::nullopt;
//     this->ref = std::get<ObjectReference>(origin);
//   } else {
//     this->stream_pos = std::nullopt;
//     this->ref = std::nullopt;
//   }
// }
//
// ErrorContext::ErrorContext(InternalNode const &) {
//   this->stream_pos = std::nullopt;
//   this->ref = std::nullopt;
// }
//
// ErrorContext::ErrorContext(ObjectReference const &ref) {
//   this->stream_pos = std::nullopt;
//   this->ref = ref;
// }
//
// ErrorContext::ErrorContext(Origin const &origin, ObjectReference const
// &ref)
// {
//   if (std::holds_alternative<InputStreamPos>(origin)) {
//     this->stream_pos = std::get<InputStreamPos>(origin);
//     this->ref = std::nullopt;
//   } else if (std::holds_alternative<ObjectReference>(origin)) {
//     this->stream_pos = std::nullopt;
//     this->ref = std::get<ObjectReference>(origin);
//   } else {
//     this->stream_pos = std::nullopt;
//     this->ref = std::nullopt;
//   }
//   this->ref = ref;
// }

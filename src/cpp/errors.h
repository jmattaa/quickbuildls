#ifndef _QUICKBUILDLS_ERRORS_H
#define _QUICKBUILDLS_ERRORS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define E_STANDARD_ITER(_X)                                                    \
    _X(EInvalidGrammar, "Invalid grammar")                                     \
    _X(EInvalidLiteral, "Invalid literal")                                     \
    _X(ENoLinestop, "Missing semicolon at end of line")                        \
    _X(ENoIterator, "No task iterator")                                        \
    _X(ENoTaskOpen, "Missing opening curly bracket")                           \
    _X(ENoTaskClose, "Missing closing curly bracket")                          \
    _X(EInvalidListEnd, "Incorrectly formatted list")                          \
    _X(ENoReplacementIdentifier, "Missing variable name for replacement",      \
       halt)                                                                   \
    _X(ENoReplacementOriginal, "Missing original value for replacement")       \
    _X(ENoReplacementArrow, "Missing the `->`")                                \
    _X(ENoReplacementReplacement, "Missing a replacement value")               \
    _X(EInvalidEscapedExpression, "Invalid escaped expression")                \
    _X(ENoExpressionClose, "Expression not closed")                            \
    _X(EEmptyExpression, "Empty expression")

    // non standard!!
    //     _X(EInvalidSymbol, "Invalid symbol", halt)
    //     _X(ENoValue, "No valid value", soft)
    //     _X(EInvalidEscapeCode, "Invalid escape code", halt)

    typedef enum ErrorCode
    {
#define _X(name, ...) _##name,
        E_STANDARD_ITER(_X)
#undef _X

            _EInvalidSymbol,
        _ENoValue,
        _EInvalidEscapeCode
    } ErrorCode;

#ifdef __cplusplus
}
#endif

#endif

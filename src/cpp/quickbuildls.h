#ifndef QUICKBUILDLS_C_H
#define QUICKBUILDLS_C_H

#include <sys/types.h>

// defined in lexer.cpp
uint is_alphabetic(char x);

uint is_keyword(const char *s);
const char *get_keyword_desc(const char *k);

#endif

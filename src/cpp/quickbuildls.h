#ifndef QUICKBUILDLS_C_H
#define QUICKBUILDLS_C_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define KEYWORDS(_X)                                                           \
    _X("run", "## Run\n---\nThe command this task should run")                 \
    _X("run_parallel", "## Run Parallel\n---\nA boolean that specifies if "    \
                       "this task should run in parallel")                     \
    _X("depends", "## Depends\n---\nThe list of tasks that should be run "     \
                  "before the current task")                                   \
    _X("depends_parallel",                                                     \
       "## Depends Parallel\n---\nA boolean that specifies if the "            \
       "dependencies should run in parallel")

#define KEYWORDS_LEN 4

// defined in lexer.cpp
bool is_alphanumeric(char x);

// quickbuildls.c
uint8_t is_keyword(const char *s);
const char *get_keyword_desc(const char *k);

extern const char *task_keyword_names[KEYWORDS_LEN];

#endif

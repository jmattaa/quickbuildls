// this code does NOT come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "state.h"
#include "errors.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "quickbuildls.hpp"
#include <cstdlib>
#include <string>
#include <vector>

static bool qls_state_set(qls_state *s, const char *csrc);
static void qls_free_state_but_not_state_ptr(qls_state *s);

extern "C" qls_state *qls_state_init(const char *csrc)
{
    qls_state *s = (qls_state *)malloc(sizeof(qls_state));
    qls_state_set(s, csrc);

    return s;
}

extern "C" void qls_state_update(qls_state *s, const char *csrc)
{
    qls_state *tmp = (qls_state *)malloc(sizeof(qls_state));
    if (qls_state_set(tmp, csrc))
    {
        qls_free_state_but_not_state_ptr(s);
        *s = *tmp;
    }
    free(tmp);
}

extern "C" void qls_state_free(qls_state *s)
{
    if (!s)
        return;

    qls_free_state_but_not_state_ptr(s);
    free(s);
}

static bool qls_state_set(qls_state *s, const char *csrc)
{
    if (!s || csrc == NULL)
        return false;

    std::string src(csrc);
    std::vector<unsigned char> vecsrc(src.begin(), src.end());

    AST ast;

    try
    {
        Lexer lexer(vecsrc);
        Parser parser(lexer.get_token_stream());
        ast = parser.parse_tokens();
    }
    catch (BuildException &e)
    {
        s->err = e.what();
        return false; // early return should kepp state as is
    }

    s->nfields = ast.fields.size();
    s->fields = (qls_obj *)malloc(sizeof(qls_obj) * s->nfields);
    for (int i = 0; i < s->nfields; i++)
    {
        qls_obj *f = &s->fields[i];
        f->type = QLS_FIELD;
        f->offset = get_origin_index(ast.fields[i].origin);

        f->name = strdup(ast.fields[i].identifier.content.c_str());
        f->value = strdup(
            std::visit(ASTVisitContent{}, ast.fields[i].expression).c_str());
    }

    s->ntasks = ast.tasks.size();
    s->tasks = (qls_obj *)malloc(sizeof(qls_obj) * s->ntasks);
    for (int i = 0; i < s->ntasks; i++)
    {
        qls_obj *t = &s->tasks[i];
        t->type = QLS_TASK;
        t->offset = get_origin_index(ast.tasks[i].origin);

        std::string tname =
            std::visit(ASTVisitContent{}, ast.tasks[i].identifier);
        t->name = strdup(tname.c_str());

        // for some reason the quoted strings offset is always wrong by one 😭
        if(src[t->offset - tname.size() - 1] != '\n')
            t->offset -= 1;

        t->nfields = ast.tasks[i].fields.size();
        t->fields = (qls_obj *)malloc(sizeof(qls_obj) * t->nfields);
        for (int j = 0; j < t->nfields; j++)
        {
            qls_obj *f = &t->fields[j];
            f->type = QLS_FIELD;
            f->offset = get_origin_index(ast.tasks[i].fields[j].origin);
            f->name = strdup(ast.tasks[i].fields[j].identifier.content.c_str());
            f->value = strdup(
                std::visit(ASTVisitContent{}, ast.tasks[i].fields[j].expression)
                    .c_str());
        }
    }

    return true;
}

static void qls_free_state_but_not_state_ptr(qls_state *s)
{
    if (!s)
        return;
    for (int i = 0; i < s->nfields; i++)
    {
        qls_obj *f = &s->fields[i];

        f->name ? free(f->name) : (void)0;
        f->value ? free(f->value) : (void)0;
    }
    s->fields ? free(s->fields) : (void)0;

    for (int i = 0; i < s->ntasks; i++)
    {
        qls_obj *t = &s->tasks[i];
        t->name ? free(t->name) : (void)0;

        for (int j = 0; j < t->nfields; j++)
        {
            qls_obj *f = &t->fields[j];
            f->name ? free(f->name) : (void)0;
            f->value ? free(f->value) : (void)0;
        }
        t->fields ? free(t->fields) : (void)0;
    }
}

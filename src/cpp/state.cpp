// this code does NOT come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#include "state.h"
#include "quickbuild/errors.hpp"
#include "quickbuild/lexer.hpp"
#include "quickbuild/parser.hpp"
#include "quickbuild/tracking.hpp"
#include "quickbuildls.hpp"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define noop (void)0

static bool qls_state_set(qls_state *s, const char *csrc);
static void qls_free_state_but_not_state_ptr(qls_state *s);
static void qls_free_s_err(qls_err *e);

extern "C" qls_state *qls_state_init(const char *csrc)
{
    qls_state *s = (qls_state *)calloc(1, sizeof(qls_state));
    qls_state_set(s, csrc);

    return s;
}

extern "C" void qls_state_update(qls_state *s, const char *csrc)
{
    qls_state *tmp = (qls_state *)calloc(1, sizeof(qls_state));
    if (qls_state_set(tmp, csrc))
    {
        qls_free_state_but_not_state_ptr(s);
        *s = *tmp;

        return;
    }

    // we've got an error
    if (tmp->errs)
    {
        free(s->errs);
        s->errs = (qls_err **)calloc(tmp->nerrs, sizeof(qls_err *));
        for (size_t i = 0; i < tmp->nerrs; i++)
        {
            qls_free_s_err(s->errs[i]);
            s->errs[i] = (qls_err *)calloc(1, sizeof(qls_err));
            s->errs[i]->msg = strdup(tmp->errs[i]->msg);
            s->errs[i]->offset = tmp->errs[i]->offset;
            s->errs[i]->endoffset = tmp->errs[i]->endoffset;
            s->errs[i]->code = tmp->errs[i]->code;
        }
    }
    else
    {
        free(s->errs);
        s->errs = NULL; // explicitly clear error if none set
    }

    qls_free_state_but_not_state_ptr(tmp);
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
    catch (...)
    {
        const auto errs = ErrorHandler::get_errors();
        s->nerrs = errs.size();
        s->errs = (qls_err **)calloc(s->nerrs, sizeof(qls_err *));

        int i = 0;
        for (const auto &[thread_hash, err] : errs)
        {
            s->errs[i] = (qls_err *)calloc(1, sizeof(qls_err));
            s->errs[i]->msg = strdup(err->get_exception_msg());

            if (auto *e_withRef =
                    dynamic_cast<EWithStreamReference *>(err.get()))
            {
                const StreamReference &ref = e_withRef->get_reference();

                s->errs[i]->offset = ref.index;
                s->errs[i]->endoffset = ref.index + ref.length;
            }
            s->errs[i]->code = err->get_code();

            i++;
        }
    }

    s->nfields = ast.fields.size();
    s->fields = (qls_obj *)calloc(s->nfields, sizeof(qls_obj));
    for (int i = 0; i < s->nfields; i++)
    {
        qls_obj *f = &s->fields[i];
        f->type = QLS_FIELD;
        f->offset = ast.fields[i].reference.index;

        f->name = strdup(ast.fields[i].identifier.content.c_str());
        f->quotedname =
            strdup(("\"" + ast.fields[i].identifier.content + "\"").c_str());
        f->value = strdup(
            std::visit(ASTVisitContent{}, ast.fields[i].expression).c_str());
    }

    s->ntasks = ast.tasks.size();
    s->tasks = (qls_obj *)calloc(s->ntasks, sizeof(qls_obj));
    for (int i = 0; i < s->ntasks; i++)
    {
        qls_obj *t = &s->tasks[i];
        t->type = QLS_TASK;
        t->offset = ast.tasks[i].reference.index;

        bool iter_has_value = ast.tasks[i].iterator.content != "__task__";
        if (iter_has_value)
            t->value = strdup(ast.tasks[i].iterator.content.c_str());

        std::string tname =
            std::visit(ASTVisitContent{}, ast.tasks[i].identifier);

        // value is set -> iterator -> name not quoted
        // value is not set -> iterator -> name quoted
        if (!iter_has_value)
            t->quotedname = strdup(("\"" + tname + "\"").c_str());

        t->name = strdup(tname.c_str());

        // for some reason the quoted strings offset is always wrong by
        // one 😭
        if (src[t->offset - tname.size() - 1] != '\n')
            t->offset -= 1;

        t->nfields = ast.tasks[i].fields.size();
        t->fields = (qls_obj *)calloc(t->nfields, sizeof(qls_obj));
        for (int j = 0; j < t->nfields; j++)
        {
            qls_obj *f = &t->fields[j];
            f->type = QLS_FIELD;
            f->offset = ast.tasks[i].fields[j].reference.index;
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

        f->name ? free(f->name) : noop;
        f->value ? free(f->value) : noop;
    }
    s->fields ? free(s->fields) : noop;

    for (int i = 0; i < s->ntasks; i++)
    {
        qls_obj *t = &s->tasks[i];
        t->name ? free(t->name) : noop;
        t->value ? free(t->value) : noop;
        t->quotedname ? free(t->quotedname) : noop;

        for (int j = 0; j < t->nfields; j++)
        {
            qls_obj *f = &t->fields[j];
            f->name ? free(f->name) : noop;
            f->value ? free(f->value) : noop;
        }
        t->fields ? free(t->fields) : noop;
    }

    for (int i = 0; i < s->nerrs; i++)
    {
        qls_free_s_err(s->errs[i]);
    }
    s->errs ? free(s->errs) : noop;
}

static void qls_free_s_err(qls_err *e)
{
    if (!e)
        return;
    e->msg ? free(e->msg) : noop;
    free(e);
}

// this code does NOT come from https://github.com/nordtechtiger/quickbuild
// rather it's a part of the quickbuildls project to bridge the c++ code to zig
// (I suck att c++ so if there is problems then blame me, i'll also be trying to
// do as much c as possible) :)

#ifndef QUICKBUILDLS_STATE_H
#define QUICKBUILDLS_STATE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum qls_objtype
    {
        QLS_TASK,
        QLS_FIELD,
    };

    typedef struct qls_obj
    {
        char *name;
        enum qls_objtype type;
        int offset;

        // `QLS_FIELD`: it will be the value of the field
        //
        // `QLS_TASK`: `if` the task contains an iterator it will be the iterator
        char *value;
        char *quotedname; 
        struct qls_obj *fields; // only if type is QLS_TASK
                                //
                                // dependencies are as a string in the depends
                                // field
        size_t nfields;
    } qls_obj;

    typedef struct qls_err
    {
        char *msg;
        size_t offset;
        size_t endoffset;
    } qls_err;

    // resembles the AST object in quickbuild
    typedef struct qls_state
    {
        qls_obj *tasks;
        qls_obj *fields;

        size_t ntasks;
        size_t nfields;

        qls_err *err;
        // TODO: implement something like this
        // size_t nerr;
        // qls_err **errs;
    } qls_state;

    qls_state *qls_state_init(const char *csrc);
    void qls_state_update(qls_state *s, const char *csrc);
    void qls_state_free(qls_state *s);

#ifdef __cplusplus
}
#endif

#endif

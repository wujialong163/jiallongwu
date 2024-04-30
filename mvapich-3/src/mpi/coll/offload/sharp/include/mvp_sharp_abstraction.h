/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 *
 */

#ifndef SHARP_ABSTRACTION_H
#define SHARP_ABSTRACTION_H

#include <infiniband/verbs.h>
#include "mvp_abstraction_util.h"

#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"

extern int MVP_ENABLE_SHARP;
extern const char *MVP_LIBSHARP_PATH;
#endif /* defined(_SHARP_SUPPORT_) */

#if defined(_SHARP_SUPPORT_)
typedef struct _sharp_ops_t {
    int (*coll_init)(struct sharp_coll_init_spec *sharp_coll_spec,
                     struct sharp_coll_context **sharp_coll_context);
    int (*coll_finalize)(struct sharp_coll_context *context);
    int (*coll_caps_query)(struct sharp_coll_context *context,
                           struct sharp_coll_caps *sharp_caps);
    int (*coll_progress)(struct sharp_coll_context *context);
    int (*coll_comm_init)(struct sharp_coll_context *context,
                          struct sharp_coll_comm_init_spec *spec,
                          struct sharp_coll_comm **sharp_coll_comm);
    int (*coll_comm_destroy)(struct sharp_coll_comm *comm);
    int (*coll_do_barrier)(struct sharp_coll_comm *comm);
    int (*coll_do_barrier_nb)(struct sharp_coll_comm *comm, void **handle);
    int (*coll_do_allreduce)(struct sharp_coll_comm *comm,
                             struct sharp_coll_reduce_spec *spec);
    int (*coll_do_allreduce_nb)(struct sharp_coll_comm *comm,
                                struct sharp_coll_reduce_spec *spec,
                                void **handle);
    int (*coll_do_reduce)(struct sharp_coll_comm *comm,
                          struct sharp_coll_reduce_spec *spec);
    int (*coll_do_reduce_nb)(struct sharp_coll_comm *comm,
                             struct sharp_coll_reduce_spec *spec,
                             void **handle);
    int (*coll_do_bcast)(struct sharp_coll_comm *comm,
                         struct sharp_coll_bcast_spec *spec);
    int (*coll_do_bcast_nb)(struct sharp_coll_comm *comm,
                            struct sharp_coll_bcast_spec *spec, void **handle);
    int (*coll_req_test)(void *handle);
    int (*coll_req_wait)(void *handle);
    int (*coll_req_free)(void *handle);
    int (*coll_reg_mr)(struct sharp_coll_context *context, void *buf,
                       size_t size, void **mr);
    int (*coll_dereg_mr)(struct sharp_coll_context *context, void *mr);
    int (*coll_print_config)(FILE *stream, enum config_print_flags print_flags);
    const char *(*coll_strerror)(int error);
    int (*coll_dump_stats)(struct sharp_coll_context *context);
    int (*coll_log_early_init)();
} sharp_ops_t;

extern sharp_ops_t sharp_ops;
extern void *sharp_dl_handle;
#endif /* defined(_SHARP_SUPPORT_) */

#ifdef _SHARP_SUPPORT_
#define MVP_STRIGIFY(v_) #v_

#define MVP_DLSYM(_struct_, _handle_, _prefix_, _function_)                    \
    do {                                                                       \
        _struct_._function_ =                                                  \
            dlsym((_handle_), MVP_STRIGIFY(_prefix_##_##_function_));          \
        if (_struct_._function_ == NULL) {                                     \
            fprintf(stderr, "Error opening %s: %s. Falling back.\n",           \
                    MVP_STRIGIFY(_prefix_##_##_function_), dlerror());         \
        }                                                                      \
    } while (0)
#else
#define MVP_DLSYM(_struct_, _handle_, _prefix_, _function_)                    \
    do {                                                                       \
        _struct_._function_ = _prefix_##_##_function_;                         \
    } while (0)
#endif

#if defined(_SHARP_SUPPORT_)
static inline int mvp_sharp_dlopen_init()
{
    char *path;

    sharp_dl_handle = dlopen("libsharp_coll.so", RTLD_NOW);
    if (!sharp_dl_handle) {
        if (MVP_LIBSHARP_PATH) {
            sharp_dl_handle = dlopen(MVP_LIBSHARP_PATH, RTLD_NOW);
        }
    }
    if (!sharp_dl_handle) {
        fprintf(stderr, "Error opening libsharp_coll.so: %s.\n", dlerror());
        if (!path) {
            fprintf(stderr, "Please retry with MVP_LIBSHARP_PATH="
                            "<path/to/libsharp_coll.so>\n");
            fprintf(stderr, "Or please try with MVP_ENABLE_SHARP=0\n");
        } else {
            fprintf(stderr, "Please try with MVP_ENABLE_SHARP=0\n");
        }
        return ERROR_DLOPEN;
    }

    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_init);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_finalize);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_caps_query);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_progress);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_comm_init);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_comm_destroy);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_barrier);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_barrier_nb);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_allreduce);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_allreduce_nb);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_reduce);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_reduce_nb);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_bcast);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_do_bcast_nb);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_req_test);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_req_wait);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_req_free);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_reg_mr);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_dereg_mr);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_print_config);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_strerror);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_dump_stats);
    MVP_DLSYM(sharp_ops, sharp_dl_handle, sharp, coll_log_early_init);

    return SUCCESS_DLOPEN;
}
#endif /* defined(_SHARP_SUPPORT_) */

static inline int mvp_dlopen_init()
{
    int err = SUCCESS_DLOPEN;

#if defined(_SHARP_SUPPORT_)
    if (MVP_ENABLE_SHARP) {
        err = mvp_sharp_dlopen_init();
        if (err != SUCCESS_DLOPEN) {
            fprintf(stderr, "mvp_sharp_dlopen_init returned %d\n", err);
        }
    }
#endif /*defined(_SHARP_SUPPORT_)*/

    return err;
}

#if defined(_SHARP_SUPPORT_)
static inline void mvp_sharp_dlopen_finalize()
{
    if (sharp_dl_handle) {
        dlclose(sharp_dl_handle);
    }
}
#endif /*defined(_SHARP_SUPPORT)*/

static inline int mvp_dlopen_finalize()
{
#if defined(_SHARP_SUPPORT_)
    mvp_sharp_dlopen_finalize();
#endif /*defined(_SHARP_SUPPORT_)*/
    return SUCCESS_DLOPEN;
}
#endif /*SHARP_ABSTRACTION_H*/

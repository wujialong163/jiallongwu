/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

/* Types and interfaces in this file are internally used by MPIR_T itself.
 * Other modules should call higher level interfaces in mpit.h instead.
 */
#ifndef MPITIMPL_H_INCLUDED
#define MPITIMPL_H_INCLUDED

#include "mpi.h"
#include "mpir_strerror.h"
#include "mpir_type_defs.h"
#include "mpir_assert.h"
#include "mpir_pointers.h"
#include "utarray.h"
#include "uthash.h"
#include "mpir_objects.h"

#ifdef HAVE_ERROR_CHECKING
typedef enum {
    MPIR_T_OBJECT_INVALID = 0,
    MPIR_T_ENUM_HANDLE,
    MPIR_T_CVAR_HANDLE,
    MPIR_T_PVAR_HANDLE,
    MPIR_T_PVAR_SESSION
} MPIR_T_object_kind;
#endif

/* MPI_T enum
 */
typedef struct enum_item_s {
    const char *name;
    int value;
} enum_item_t;

typedef struct MPIR_T_enum_s {
#ifdef HAVE_ERROR_CHECKING
    MPIR_T_object_kind kind;
#endif
    const char *name;
    UT_array *items;
} MPIR_T_enum_t;

/* MPI_T category (cat)
 */
typedef struct {
    const char *name;
    UT_array *cvar_indices;
    UT_array *pvar_indices;
    UT_array *subcat_indices;
    const char *desc;
} cat_table_entry_t;

/* Hash names to indices in a table */
typedef struct {
    const char *name;
    unsigned idx;
    UT_hash_handle hh;          /* Makes this structure hashable */
} name2index_hash_t;

/* MPI_T control variable (cvar)
 */
typedef struct MPIR_T_cvar_range_value_s {
    int low;
    int high;
} MPIR_T_cvar_range_value_t;

/* Type used to represent cvar default values */
typedef union MPIR_T_cvar_value_s {
    int d;
    unsigned u;
    unsigned ul;
    unsigned ull;
    MPI_Count c;
    const char *str;
    double f;
    MPIR_T_cvar_range_value_t range;
} MPIR_T_cvar_value_t;

typedef int MPIR_T_cvar_get_addr_cb(void *obj_handle, void **addr);
typedef int MPIR_T_cvar_get_count_cb(void *obj_handle, int *count);

typedef struct cvar_table_entry_s {
    /* Is the cvar currently in use? False if the cvar is unregistered */
    int active;

    /* cvar name */
    const char *name;

    /* Address of the cvar. May be NULL when get_addr != NULL */
    void *addr;

    /* cvar data type */
    MPI_Datatype datatype;

    /* Num. of elements of the cvar. May be ignored when get_count != NULL */
    int count;

    /* Properties of the cvar */
    MPI_T_enum enumtype;
    MPIR_T_verbosity_t verbosity;
    MPIR_T_bind_t bind;
    MPIR_T_scope_t scope;

    /* Default value */
    MPIR_T_cvar_value_t defaultval;

    /* If not NULL, components provide this callback to get addr of the cvar */
    MPIR_T_cvar_get_addr_cb *get_addr;

    /* If not NULL, components provide this callback to get count */
    MPIR_T_cvar_get_count_cb *get_count;

    /* Description of the cvar */
    const char *desc;
} cvar_table_entry_t;

typedef struct MPIR_T_cvar_handle_s {
#ifdef HAVE_ERROR_CHECKING
    MPIR_T_object_kind kind;
#endif

    /* Address and count of the cvar. Set at handle allocation time */
    void *addr;
    int count;

    /* Cached value from cvar_table_entry_t to avoid indirection */
    MPI_Datatype datatype;
    MPIR_T_scope_t scope;
} MPIR_T_cvar_handle_t;

void MPIR_T_CVAR_REGISTER_impl(MPI_Datatype dtype, const char *name, const void *addr, int count,
                               MPIR_T_enum_t * etype, MPIR_T_verbosity_t verb, MPIR_T_bind_t bind,
                               MPIR_T_scope_t scope, MPIR_T_cvar_get_addr_cb get_addr,
                               MPIR_T_cvar_get_count_cb get_count, MPIR_T_cvar_value_t defaultval,
                               const char *cat, const char *desc);

/* MPI_T performance variable (pvar)
 */

extern int pvar_name[64];

/* Forward declaration */
struct MPIR_T_pvar_handle_s;
struct MPIR_T_pvar_session_s;

typedef void MPIR_T_pvar_get_value_cb(void *addr, void *obj_handle, int count, void *buf);
typedef void MPIR_T_pvar_get_count_cb(void *addr, void *obj_handle, int *count);

/* Basic pvar flags defined by MPI_T standard */
#define MPIR_T_PVAR_FLAG_READONLY      0x01
#define MPIR_T_PVAR_FLAG_CONTINUOUS    0x02
#define MPIR_T_PVAR_FLAG_ATOMIC        0x04

/* Auxlilary flags used by MPIR_T */

/* pvar is MPI_T_PVAR_CLASS_{COUNTER, TIMER, AGGREGATE} */
#define MPIR_T_PVAR_FLAG_SUM           0x08

/* pvar is MPI_T_PVAR_CLASS_{HIGH, LOW}WATERMARK */
#define MPIR_T_PVAR_FLAG_WATERMARK     0x10

/* pvar is continuous. If not, it has been started at least once */
#define MPIR_T_PVAR_FLAG_ONCESTARTED   0x20

/* pvar is continuous. If not, it is started */
#define MPIR_T_PVAR_FLAG_STARTED       0x40

/* Used only for watermark handles. Set if a pvar handle is the
 * first handle of an associated watermark.
 */
#define MPIR_T_PVAR_FLAG_FIRST         0x80

/* MPI_T performance variable (pvar) stuff */
typedef struct {
    /* Is the pvar in use (i.e., not unregistered)? */
    int active;

    /* pvar name */
    const char *name;

    /* If not NULL, it is address of the pvar */
    void *addr;

    /* pvar data type */
    MPI_Datatype datatype;

    /* Num. of elements of the pvar */
    int count;

    /* Properties of the pvar */
    MPIR_T_pvar_class_t varclass;
    MPIR_T_verbosity_t verbosity;
    MPIR_T_enum_t *enumtype;
    MPIR_T_bind_t bind;

    /* Basic flags of the pvar */
    int flags;

    /* If not NULL, components provide this callback to read the pvar */
    MPIR_T_pvar_get_value_cb *get_value;

    /* If not NULL, components provide this callback to get count */
    MPIR_T_pvar_get_count_cb *get_count;

    /* Description of the pvar */
    const char *desc;

    //Indexes (used only if pvar is at sub communicator level)
    int sub_comm_index;
    int sub_comm_timer_index;
} pvar_table_entry_t;

/*
 The following two macros do not work since C preprocessor does not support
 nested ifdefs. So we use another woarkable but a little ugly approach.

#define PVAR_GATED_ACTION(MODULE, action_) \
    do { \
        #ifdef ENABLE_PVAR_##MODULE \
            action_; \
        #endif \
    } while (0)
*/

/* ENABLE_PVAR_##MODULE must be defined by configure script either to 0 or 1 */
#define PVAR_GATED_ACTION(MODULE, action_) \
    do { \
        if (ENABLE_PVAR_##MODULE) { \
            action_; \
        } \
    } while (0)

/* For some classes of pvars, internally we can not represent them
 * in basic data types. So come the following typedefs.
 */

/* Timer type */
typedef struct {
    /* Accumulated time */
    MPL_time_t total;

    /* Time when the timer was started recently */
    MPL_time_t curstart;

    /* A counter recording how many times the timer is started */
    unsigned long long count;
} MPIR_T_pvar_timer_t;

/*Aggregating communication details type */
typedef struct 
{
    /*rank*/
    int send_rank[256];
    int recv_rank[256];

    /*count*/
    int count;

    /*tag*/
    int tag[256];
    int site;

    /*time*/
    double timer[256];
    double start[256];
    double end[256];

    /* data */
    int data_size[256];
}MPI_T_PVAR_detail_info_t;

/* An union to represent a watermark value */
typedef union {
    double f;
    unsigned u;
    unsigned long ul;
    unsigned long long ull;
} watermark_value_t;

/* Watermark type */
typedef struct {
    /* current -- current resource utilization level
     * waterarmk -- cached value for the first pvar handle
     */
    watermark_value_t current, watermark;

    /* Datatype of the watermark */
    MPI_Datatype datatype;

    /* Is the cached value (i.e, watermark) in use by a pvar handle? */
    int first_used;

    /* Is the first pvar handle started? */
    int first_started;

    /* A double-linked list of handles of the pvar */
    struct MPIR_T_pvar_handle_s *hlist;
} MPIR_T_pvar_watermark_t;

typedef struct MPIR_T_pvar_handle_s {
#ifdef HAVE_ERROR_CHECKING
    MPIR_T_object_kind kind;
#endif

    /* These are cached fields from pvar table. Do so to avoid extra
     * indirection when accessing them through pvar handles.
     */
    void *addr;
    MPI_Datatype datatype;
    int count;
    MPIR_T_pvar_get_value_cb *get_value;
    MPIR_T_pvar_class_t varclass;

    /* Bytes of an element of datatype */
    int bytes;

    /* Basic flags copied from pvar info + auxilary flags in pvar handle */
    int flags;

    /* Store info here in case we need other fields */
    const pvar_table_entry_t *info;

    /* Owner session from which the handle is allocated */
    struct MPIR_T_pvar_session_s *session;

    /* Object which this pvar is bound to. NULL if no binding */
    void *obj_handle;

    /* This is how we support pvar sessions.
     *
     * For pvars of counter, timer or aggregate type, we cache their value at
     * the last start time in offset, their current value in current, and
     * their accumlated value in accum. Generally, when such a pvar is running,
     * reading the pvar should return
     *      accum[i] + current[i] - offset[i], 0 <= i < count - 1.
     * When the pvar is stopped, reading just returns accum.
     *
     * For pvars of high/lowwatermark type, above method does not work.
     * We have a copy of such a pvar in every handle of the pvar.
     * Handles are registered to the pvar. Whenever a watermark changes,
     * its copies in non-stopped handles are updated. That sounds non-scalable.
     * Considering single-session is common, we reserve room in watermark
     * themselves for cache buffer for the first handle. So when such a pvar
     * changes, it also updates the watermark close to it in memory.
     *
     * For pvars of other classes,  since they are supposed to be readonly
     * and continuous (FIXME: Is it true?), caching is not needed.
     */
    void *accum;    //累计执行时间
    void *offset;   //最后一次开始时间。
    void *current;  //当前时间

    watermark_value_t watermark;

    /* To chain handles in a session */
    struct MPIR_T_pvar_handle_s *prev, *next;

    /* To chain handles of a watermark pvar */
    struct MPIR_T_pvar_handle_s *prev2, *next2;
} MPIR_T_pvar_handle_t;

typedef struct MPIR_T_pvar_session_s {
#ifdef HAVE_ERROR_CHECKING
    MPIR_T_object_kind kind;
#endif

    /* A linked list of pvar handles */
    MPIR_T_pvar_handle_t *hlist;
} MPIR_T_pvar_session_t;

extern void MPIR_T_PVAR_REGISTER_impl(MPIR_T_pvar_class_t varclass, MPI_Datatype dtype,
                                      const char *name, void *addr, int count,
                                      MPIR_T_enum_t * etype, MPIR_T_verbosity_t verb,
                                      MPIR_T_bind_t bind, int flags,
                                      MPIR_T_pvar_get_value_cb get_value,
                                      MPIR_T_pvar_get_count_cb get_count, const char *cat,
                                      const char *desc);

/* For static pvars (i.e., pvars with static storage), we embed their class name
 * into their variable name, so that users can declare pvars with the same name
 * for different classes, without worry of name conflict. "class + pvar name"
 * should be unique as required by MPI_T.
 */

/* MPI_T_PVAR_CLASS_STATE (continuous only)
 */

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_STATE_SET_VAR_impl(ptr_, val_) \
    do { *(ptr_) = (val_); } while (0)
#define MPIR_T_PVAR_STATE_GET_VAR_impl(ptr_) \
    (*(ptr_))

#define MPIR_T_PVAR_STATE_SET_impl(name_, val_) \
    MPIR_T_PVAR_STATE_SET_VAR_impl(&PVAR_STATE_##name_, val_)
#define MPIR_T_PVAR_STATE_GET_impl(name_) \
    MPIR_T_PVAR_STATE_GET_VAR_impl(&PVAR_STATE_##name_)

/* Registration AND initialization for static pvar */
#define MPIR_T_PVAR_STATE_REGISTER_STATIC_impl(dtype_, name_, \
            initval_, etype_, verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_INT); \
        /* Double check if dtype_ and name_ match */ \
        MPIR_Assert(sizeof(PVAR_STATE_##name_) == MPIR_Datatype_get_basic_size(dtype_)); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        /* State pvars should be describled further by an enum */ \
        MPIR_Assert((etype_) != MPI_T_ENUM_NULL); \
        PVAR_STATE_##name_ = (initval_); \
        addr_ = &PVAR_STATE_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_STATE, dtype_, #name_, \
            addr_, 1, etype_, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_STATE_REGISTER_DYNAMIC_impl(dtype_, name_, addr_, count_, \
            etype_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_INT); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        MPIR_Assert((etype_) != MPI_T_ENUM_NULL); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_STATE, dtype_, #name_, \
            addr_, count_, etype_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_); \
    } while (0)


/* MPI_T_PVAR_CLASS_LEVEL (continuous only)
 */

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_LEVEL_SET_VAR_impl(ptr_, val_) \
    do { *(ptr_) = (val_); } while (0)
#define MPIR_T_PVAR_LEVEL_GET_VAR_impl(ptr_) \
    (*(ptr_))
#define MPIR_T_PVAR_LEVEL_INC_VAR_impl(ptr_, val_) \
    do { *(ptr_) += (val_); } while (0)
#define MPIR_T_PVAR_LEVEL_DEC_VAR_impl(ptr_, val_) \
    do { *(ptr_) -= (val_); } while (0)

#define MPIR_T_PVAR_LEVEL_SET_impl(name_, val_) \
    MPIR_T_PVAR_LEVEL_SET_VAR_impl(&PVAR_LEVEL_##name_, val_)
#define MPIR_T_PVAR_LEVEL_GET_impl(name_) \
    MPIR_T_PVAR_LEVEL_GET_VAR_impl(&PVAR_LEVEL_##name_)
#define MPIR_T_PVAR_LEVEL_INC_impl(name_, val_) \
    MPIR_T_PVAR_LEVEL_INC_VAR_impl(&PVAR_LEVEL_##name_, val_)
#define MPIR_T_PVAR_LEVEL_DEC_impl(name_, val_) \
    MPIR_T_PVAR_LEVEL_DEC_VAR_impl(&PVAR_LEVEL_##name_, val_)

/* Registration AND initialization for static pvar */
#define MPIR_T_PVAR_LEVEL_REGISTER_STATIC_impl(dtype_, name_, \
            initval_, verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        /* Double check if dtype_ and name_ match */ \
        MPIR_Assert(sizeof(PVAR_LEVEL_##name_) == MPIR_Datatype_get_basic_size(dtype_)); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        PVAR_LEVEL_##name_ = (initval_); \
        addr_ = &PVAR_LEVEL_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_LEVEL, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_LEVEL_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_LEVEL, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)


/* MPI_T_PVAR_CLASS_SIZE (continuous only)
 */

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_SIZE_SET_VAR_impl(ptr_, val_) \
    do { *(ptr_) = (val_); } while (0)
#define MPIR_T_PVAR_SIZE_GET_VAR_impl(ptr_) \
    (*(ptr_))

#define MPIR_T_PVAR_SIZE_SET_impl(name_, val_) \
    MPIR_T_PVAR_SIZE_SET_VAR_impl(&PVAR_SIZE_##name_, val_)
#define MPIR_T_PVAR_SIZE_GET_impl(name_) \
    MPIR_T_PVAR_SIZE_GET_VAR_impl(&PVAR_SIZE_##name_)

/* Registration AND initialization for static pvar */
#define MPIR_T_PVAR_SIZE_REGISTER_STATIC_impl(dtype_, name_, \
            initval_, verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        /* Double check if dtype_ and name_ match */ \
        MPIR_Assert(sizeof(PVAR_SIZE_##name_) == MPIR_Datatype_get_basic_size(dtype_)); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        PVAR_SIZE_##name_ = (initval_); \
        addr_ = &PVAR_SIZE_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_SIZE, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_SIZE_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_SIZE, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)


/* MPI_T_PVAR_CLASS_PERCENTAGE (continuous only)
 */

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_PERCENTAGE_SET_VAR_impl(ptr_, val_) \
    do { \
        MPIR_Assert(0.0 <= (val_) && (val_) <= 1.0); \
        *(ptr_) = (val_); \
    } while (0)
#define MPIR_T_PVAR_PERCENTAGE_GET_VAR_impl(ptr_) \
    (*(ptr_))

#define MPIR_T_PVAR_PERCENTAGE_SET_impl(name_, val_) \
    MPIR_T_PVAR_PERCENTAGE_SET_VAR_impl(&PVAR_PERCENTAGE_##name_, val_)
#define MPIR_T_PVAR_PERCENTAGE_GET_impl(name_) \
    MPIR_T_PVAR_PERCENTAGE_GET_VAR_impl(&PVAR_PERCENTAGE_##name_)

/* Registration AND initialization for static pvar */
#define MPIR_T_PVAR_PERCENTAGE_REGISTER_STATIC_impl(dtype_, name_, \
            initval_, verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_DOUBLE); \
        /* Double check if dtype_ and name_ match */ \
        MPIR_Assert(sizeof(PVAR_PERCENTAGE_##name_) == MPIR_Datatype_get_basic_size(dtype_)); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        addr_ = &PVAR_PERCENTAGE_##name_; \
        PVAR_PERCENTAGE_##name_ = (initval_); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_PERCENTAGE, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_PERCENTAGE_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_DOUBLE); \
        MPIR_Assert((flags_) & MPIR_T_PVAR_FLAG_CONTINUOUS); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_PERCENTAGE, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)


/* MPI_T_PVAR_CLASS_COUNTER (continuous or not)
 */

typedef struct pvar_bucket
{
    int min;
    int max;
} pvar_bucket;

#define counter_pvar_array_size 32

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_COUNTER_INIT_VAR_impl(ptr_) \
    do { *(ptr_) = 0; } while (0)
/* _COUNTER_SET is intentionally not provided. Users should only INC counters */
#define MPIR_T_PVAR_COUNTER_GET_VAR_impl(ptr_) \
    (*(ptr_))
#define MPIR_T_PVAR_COUNTER_INC_VAR_impl(ptr_, inc_) \
    do { *(ptr_) += (inc_); } while (0)

#define MPIR_T_PVAR_COUNTER_INIT_impl(name_) \
    MPIR_T_PVAR_COUNTER_INIT_VAR_impl(&PVAR_COUNTER_##name_)
#define MPIR_T_PVAR_COUNTER_GET_impl(name_) \
    MPIR_T_PVAR_COUNTER_GET_VAR_impl(&PVAR_COUNTER_##name_)
#define MPIR_T_PVAR_COUNTER_INC_impl(name_, inc_) \
    MPIR_T_PVAR_COUNTER_INC_VAR_impl(&PVAR_COUNTER_##name_, inc_)

#define QUOTE(name) #name

#if ENABLE_PVAR_MVP
#define MPIR_T_PVAR_COMM_COUNTER_INC_impl(name_, inc_, comm)                   \
    do {                                                                       \
        name2index_hash_t *hash_entry;                                         \
        pvar_table_entry_t *pvar;                                              \
        int pvar_idx;                                                          \
        int seq = MPI_T_PVAR_CLASS_COUNTER - MPIR_T_PVAR_CLASS_FIRST;          \
        char *name = QUOTE(name_);                                             \
        HASH_FIND_STR(pvar_hashs[seq], name, hash_entry);                      \
        if (hash_entry != NULL) {                                              \
            pvar_idx = hash_entry->idx;                                        \
            pvar = (pvar_table_entry_t *)utarray_eltptr(pvar_table, pvar_idx); \
            if (comm->dev.ch.sub_comm_counters != NULL)                        \
                comm->dev.ch.sub_comm_counters[pvar->sub_comm_index] += inc_;  \
        }                                                                      \
    } while (0)
#else
   #define MPIR_T_PVAR_COMM_COUNTER_INC_impl(name_, inc_,comm)
#endif

#if ENABLE_PVAR_MVP
#define MPIR_T_PVAR_COMM_TIMER_START_impl(name_, comm)                         \
    do {                                                                       \
        if (MVP_ENABLE_PVAR_TIMER) {                                           \
            name2index_hash_t *hash_entry;                                     \
            pvar_table_entry_t *pvar;                                          \
            int pvar_idx;                                                      \
            int seq = MPI_T_PVAR_CLASS_TIMER - MPIR_T_PVAR_CLASS_FIRST;        \
            int counter_seq =                                                  \
                MPI_T_PVAR_CLASS_COUNTER - MPIR_T_PVAR_CLASS_FIRST;            \
            char *name = QUOTE(name_);                                         \
            HASH_FIND_STR(pvar_hashs[seq], name, hash_entry);                  \
            if (hash_entry != NULL) {                                          \
                pvar_idx = hash_entry->idx;                                    \
                pvar = (pvar_table_entry_t *)utarray_eltptr(pvar_table,        \
                                                            pvar_idx);         \
                if (comm->dev.ch.sub_comm_timers != NULL) {                    \
                    MPL_wtime(                                                 \
                        &((comm->dev.ch                                        \
                               .sub_comm_timers[pvar->sub_comm_timer_index])   \
                              .curstart));                                     \
                    (comm->dev.ch.sub_comm_timers[pvar->sub_comm_timer_index]) \
                        .count++;                                              \
                }                                                              \
            }                                                                  \
            HASH_FIND_STR(pvar_hashs[counter_seq], name, hash_entry);          \
            if (hash_entry != NULL) {                                          \
                pvar_idx = hash_entry->idx;                                    \
                pvar = (pvar_table_entry_t *)utarray_eltptr(pvar_table,        \
                                                            pvar_idx);         \
                if (comm->dev.ch.sub_comm_counters != NULL) {                  \
                    comm->dev.ch.sub_comm_counters[pvar->sub_comm_index] += 1; \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)                                                                
#define MPIR_T_PVAR_COMM_TIMER_END_impl(name_, comm)                           \
    do {                                                                       \
        if (MVP_ENABLE_PVAR_TIMER) {                                           \
            name2index_hash_t *hash_entry;                                     \
            pvar_table_entry_t *pvar;                                          \
            int pvar_idx;                                                      \
            double d,s;\
            int seq = MPI_T_PVAR_CLASS_TIMER - MPIR_T_PVAR_CLASS_FIRST;        \
            char *name = QUOTE(name_);                                         \
            HASH_FIND_STR(pvar_hashs[seq], name, hash_entry);                  \
            if (hash_entry != NULL) {                                          \
                pvar_idx = hash_entry->idx;                                    \
                pvar = (pvar_table_entry_t *)utarray_eltptr(pvar_table,        \
                                                            pvar_idx);         \
                if (comm->dev.ch.sub_comm_timers != NULL &&                    \
                    comm->dev.ch.sub_comm_counters != NULL) {                  \
                    MPL_time_t tmp;                                            \
                    MPL_wtime(&tmp);                                           \
                    MPL_wtime_acc(                                             \
                        &((comm->dev.ch                                        \
                               .sub_comm_timers[pvar->sub_comm_timer_index])   \
                              .curstart),                                      \
                        &tmp,                                                  \
                        &((comm->dev.ch                                        \
                               .sub_comm_timers[pvar->sub_comm_timer_index])   \
                              .total));                                        \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)
#else
   #define MPIR_T_PVAR_COMM_TIMER_START_impl(name_,comm)
   #define MPIR_T_PVAR_COMM_TIMER_END_impl(name_,comm)
#endif

// MPL_wtime_todouble(&((PVAR_TIMER_##name_).total),&d);\
// printf("befor wtime_acc=%lf name=%s \n",d,name);\
// MPL_wtime_acc(                                             \
//     &((comm->dev.ch                                        \
//            .sub_comm_timers[pvar->sub_comm_timer_index])   \
//           .curstart),                                      \
//     &tmp,                                                  \
//     &((PVAR_TIMER_##name_).total));                         \
// MPL_wtime_todouble(&(tmp),&s);\
// MPL_wtime_todouble(&((PVAR_TIMER_##name_).total),&d);\
// printf("end=%lf total=%lf name=%s \n",s,d,name);\


#define MPIR_T_PVAR_COUNTER_ADDR_impl(name_) \
    (&PVAR_COUNTER_##name_)

#define MPIR_T_PVAR_COUNTER_BUCKET_INC_impl(name_, count, datatype)\
        do {\
        int _pSize = 0;                                                                          \
        MPIR_Datatype_get_size_macro(datatype, _pSize);                                          \
        int msgsize = count * _pSize;                                                            \
        if (msgsize < 0) {                                                                       \
            msgsize = 0;                                                                         \
        }                                                                                        \
        int index = -1;                                                                          \
        int i = 0;                                                                               \
        while(i < num_counter_pvar_buckets && index == -1){                                      \
           if((msgsize <= counter_pvar_buckets[i].max && msgsize >=counter_pvar_buckets[i].min)  \
             || (counter_pvar_buckets[i].max == -1 && msgsize >= counter_pvar_buckets[i].min))   \
           { index = i; }                                                                        \
           i++;                                                                                  \
        }                                                                                        \
        if(index!=-1)                                                                            \
        MPIR_T_PVAR_COUNTER_ARRAY_INC_impl(name_, index, 1);                                     \
        }while(0) 

/* Registration AND initialization to zero for static pvar.  */
#define MPIR_T_PVAR_COUNTER_REGISTER_STATIC_impl(dtype_, name_, \
            verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG); \
        /* Double check if dtype_ and name_ match*/ \
        MPIR_Assert(sizeof(PVAR_COUNTER_##name_) == MPIR_Datatype_get_basic_size(dtype_)); \
        PVAR_COUNTER_##name_ = 0; \
        addr_ = &PVAR_COUNTER_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_COUNTER, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/*Wrapper to aid Registration for bucket pvars (uses the dynamic pvar interface in the end). */
#define MPIR_T_PVAR_COUNTER_BUCKET_REGISTER_DYNAMIC_impl(dtype_, name_, count_, \
            verb_, bind_, flags_, cat_, desc_) \
   do {\
    /*TODO : use MPI defined datatypes*/\
    PVAR_COUNTER_##name_ = (unsigned long long *)MPL_malloc(count_ * sizeof(unsigned long long));\
    int i = 0;\
    for(i = 0; i < count_;i++) { PVAR_COUNTER_##name_[i]=0; }\
    MPIR_T_PVAR_COUNTER_REGISTER_DYNAMIC_impl(dtype_, name_, \
              PVAR_COUNTER_##name_, count_, verb_, bind_, flags_,NULL, NULL, cat_, desc_);\
   }while(0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_COUNTER_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_COUNTER, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)

#define MPIR_T_PVAR_COUNTER_BUCKET_DYNAMIC_FREE(name_) \
    if(PVAR_COUNTER_##name_) {                         \
        MPL_free(PVAR_COUNTER_##name_);               \
    }

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_COUNTER_ARRAY_INIT_VAR_impl(ptr_, count_) \
    do { \
        int idx_; \
        idx_ = 0; \
        for (; idx_ < (count_); idx_++) \
            *((ptr_) + idx_) = 0; \
    } while (0)
#define MPIR_T_PVAR_COUNTER_ARRAY_GET_VAR_impl(ptr_, idx_) \
    *((ptr_) + (idx_))
#define MPIR_T_PVAR_COUNTER_ARRAY_INC_VAR_impl(ptr_, idx_, inc_) \
    do { *((ptr_) + (idx_)) += (inc_); } while (0)

#define MPIR_T_PVAR_COUNTER_ARRAY_INIT_impl(name_) \
    do { \
        int count_; \
        count_ = sizeof(PVAR_COUNTER_##name_)/sizeof(PVAR_COUNTER_##name_[0]); \
        MPIR_T_PVAR_COUNTER_ARRAY_INIT_VAR_impl(PVAR_COUNTER_##name_, count_); \
    } while (0)
#define MPIR_T_PVAR_COUNTER_ARRAY_GET_impl(name_, idx_) \
    MPIR_T_PVAR_COUNTER_ARRAY_GET_VAR_impl(PVAR_COUNTER_##name_, idx_)
#define MPIR_T_PVAR_COUNTER_ARRAY_INC_impl(ptr_, idx_, inc_) \
    MPIR_T_PVAR_COUNTER_ARRAY_INC_VAR_impl(PVAR_COUNTER_##ptr_, idx_, inc_)

/* Registration AND initialization to zero for static counter array  */
#define MPIR_T_PVAR_COUNTER_ARRAY_REGISTER_STATIC_impl(dtype_, name_, \
            verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        int count_;  \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG); \
        /* Double check if dtype_ and name_ match */ \
        MPIR_Assert(sizeof(PVAR_COUNTER_##name_[0]) == MPIR_Datatype_get_basic_size(dtype_)); \
        addr_ = PVAR_COUNTER_##name_; \
        MPIR_T_PVAR_COUNTER_ARRAY_INIT_impl(name_); \
        count_ = sizeof(PVAR_COUNTER_##name_)/sizeof(PVAR_COUNTER_##name_[0]); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_COUNTER, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Dynamic counter array is already provided by MPIR_T_PVAR_COUNTER_REGISTER_DYNAMIC */

/* MPI_T_PVAR_CLASS_AGGREGATE (continuous or not)
 */

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_AGGREGATE_INIT_VAR_impl(ptr_) \
    do { *(ptr_) = 0; } while (0)
/* _AGGREGATE_SET is intentionally not provided as for counters */
#define MPIR_T_PVAR_AGGREGATE_GET_VAR_impl(ptr_) \
    (*(ptr_))
#define MPIR_T_PVAR_AGGREGATE_INC_VAR_impl(ptr_, inc_) \
    do { *(ptr_) += (inc_); } while (0)

#define MPIR_T_PVAR_AGGREGATE_INIT_impl(name_) \
    MPIR_T_PVAR_AGGREGATE_INIT_VAR_impl(&PVAR_AGGREGATE_##name_)
#define MPIR_T_PVAR_AGGREGATE_GET_impl(name_) \
    MPIR_T_PVAR_AGGREGATE_GET_VAR_impl(&PVAR_AGGREGATE_##name_)
#define MPIR_T_PVAR_AGGREGATE_INC_impl(name_, inc_) \
    MPIR_T_PVAR_AGGREGATE_INC_VAR_impl(&PVAR_AGGREGATE_##name_, inc_)

/* Registration AND initialization to zero for static aggregate  */
#define MPIR_T_PVAR_AGGREGATE_REGISTER_STATIC_impl(dtype_, name_, \
            verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        /* Double check if dtype_ and name_ match*/ \
        MPIR_Assert(sizeof(PVAR_AGGREGATE_##name_) == MPIR_Datatype_get_basic_size(dtype_)); \
        PVAR_AGGREGATE_##name_ = 0; \
        addr_ = &PVAR_AGGREGATE_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_AGGREGATE, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_AGGREGATE_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_AGGREGATE, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)


/* MPI_T_PVAR_CLASS_TIMER (continuous or not)
 */

/* Interfaces through pointer or name */
#define MPIR_T_PVAR_TIMER_INIT_VAR_impl(ptr_) \
    do { \
        /* FIXME: need a generic approach to init a timer */ \
        memset(&((ptr_)->total), 0, sizeof(MPL_time_t)); \
    } while (0)
#define MPIR_T_PVAR_TIMER_GET_VAR_impl(ptr_, buf) \
    do { \
        MPL_wtime_todouble(&((ptr_)->total), buf); \
    } while (0)
#define MPIR_T_PVAR_TIMER_START_VAR_impl(ptr_) \
    do { \
        MPL_wtime(&((ptr_)->curstart)); \
        (ptr_)->count++; \
    } while (0)
#define MPIR_T_PVAR_TIMER_END_VAR_impl(ptr_) \
    do { \
        MPL_time_t tmp_; \
        MPL_wtime(&tmp_); \
        MPL_wtime_acc(&((ptr_)->curstart), &tmp_, &((ptr_)->total)); \
        double d,e,s;\
        /*MPL_wtime_todouble(&((ptr_)->curstart),&s);\
        MPL_wtime_todouble(&tmp_,&e);\
        MPL_wtime_todouble(&((ptr_)->total),&d);\
        printf("start=%lf ptr_time=%lf end=%lf \n",s,d,e);*/\
    } while (0)

#define MPIR_T_PVAR_TIMER_INIT_impl(name_) \
    MPIR_T_PVAR_TIMER_INIT_VAR_impl(&PVAR_TIMER_##name_)
#define MPIR_T_PVAR_TIMER_GET_impl(name_, buf_) \
    MPIR_T_PVAR_TIMER_GET_VAR_impl(&PVAR_TIMER_##name_, buf_)
#define MPIR_T_PVAR_TIMER_START_impl(name_) \
    MPIR_T_PVAR_TIMER_START_VAR_impl(&PVAR_TIMER_##name_)
#define MPIR_T_PVAR_TIMER_END_impl(name_) \
    do{\
    char *name=QUOTE(name_);\
    /*printf("name=%s\n",name);*/\
    MPIR_T_PVAR_TIMER_END_VAR_impl(&PVAR_TIMER_##name_);\
    }while(0);
#define MPIR_T_PVAR_TIMER_ADDR_impl(name_) \
    (&PVAR_TIMER_##name_)

#define MPIR_T_PVAR_INFO_INIT_VAR_impl(ptr_)\
    do{\
        (ptr_)->site=0;\
        (ptr_)->count=0;\
    }while(0)


#define MPIR_T_PVAR_INFO_INIT_impl(name_) \
    MPIR_T_PVAR_INFO_INIT_VAR_impl(&PVAR_INFO_##name_)

/* Customized get_value() for MPIR_T_pvar_timer_t */
static inline
    void get_timer_time_in_double(MPIR_T_pvar_timer_t * timer, void *obj_handle,
                                  int count, double *buf)
{
    int i;
    for (i = 0; i < count; i++)
        MPL_wtime_todouble(&(timer[i].total), &buf[i]);
}


/* Registration for static storage */
#define MPIR_T_PVAR_TIMER_REGISTER_STATIC_impl(dtype_, name_, \
            verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        void *count_addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_DOUBLE); \
        MPIR_T_PVAR_TIMER_INIT_impl(name_); \
        addr_ = &PVAR_TIMER_##name_; \
        count_addr_ = &(PVAR_TIMER_##name_.count); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_TIMER, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, \
            (MPIR_T_pvar_get_value_cb *)&get_timer_time_in_double, NULL, cat_, desc_); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_COUNTER, MPI_UNSIGNED_LONG_LONG, #name_, \
            count_addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, \
            NULL, NULL, cat_, desc_); \
    } while (0)

#define MPIR_T_PVAR_INFO_REGISTER_STATIC_impl(dtype_, name_, \
            verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        void *count_addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_DOUBLE); \
        MPIR_T_PVAR_INFO_INIT_impl(name_);\
        addr_ = &PVAR_INFO_##name_; \
        count_addr_ = &(PVAR_INFO_##name_.count); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_INFO, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, \
            NULL, NULL, cat_, desc_); \
    } while (0)

// static int find_info_name(int name[], int id){
    
// }

#define MPI_PVAR_INFO_TAG_ADD_impl(ptr_,add_)\
    do{\
        if((ptr_)->count !=0){\
            if(((ptr_)->site) ==((ptr_)->tag)[(ptr_)->count-1] ){\
            ((ptr_)->site)+=(add_);\
            }\
        }\
    }while(0)

#define MPI_detail_info_impl(ptr_,send, recv, count_,start,end,_size)\
    do{\
        ((ptr_)->send_rank)[(ptr_)->count]=send;\
        /*printf("send=%d ptr_send rank=%d\n",send,((ptr_)->send_rank)[(ptr_)->count]);*/\
        ((ptr_)->recv_rank)[(ptr_)->count]=recv;\
        ((ptr_)->timer)[(ptr_)->count]=end-start;\
        ((ptr_)->data_size)[(ptr_)->count]=_size;\
        ((ptr_)->start)[(ptr_)->count]=start;\
        ((ptr_)->end)[(ptr_)->count]=end;\
        ((ptr_)->tag)[(ptr_)->count]=(ptr_)->site;\
        ((ptr_)->count)+=(count_);\
        /*printf("time=%lf ptr_time=%lf \n",time,((ptr_)->timer)[(ptr_)->count]);*/\
    }while(0)
/* MPI_T_PVAR_CLASS_HIGHWATERMARK (continuous or not)
 */

/* Interfaces through pointer or name.
 * In contrast to previous pvar classes, for each type we create a set
 * of interfaces. That is because we have a pointer and a union in the
 * struct. We need to know types to (de)reference them.
*/
#define MPIR_T_PVAR_UINT_HIGHWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_UNSIGNED; \
        (ptr_)->current.u = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_ULONG_HIGHWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_UNSIGNED_LONG; \
        (ptr_)->current.ul = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_ULONG2_HIGHWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_UNSIGNED_LONG_LONG; \
        (ptr_)->current.ull = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_DOUBLE; \
        (ptr_)->current.f = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_UINT_HIGHWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.u = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) > (ptr_)->watermark.u) \
                (ptr_)->watermark.u = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) > head->watermark.u) { \
                head->watermark.u = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_ULONG_HIGHWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.ul = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) > (ptr_)->watermark.ul) \
                (ptr_)->watermark.ul = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) > head->watermark.ul) { \
                head->watermark.ul = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_ULONG2_HIGHWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.ull = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) > (ptr_)->watermark.ull) \
                (ptr_)->watermark.ull = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) > head->watermark.ull) { \
                head->watermark.ull = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.f = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) > (ptr_)->watermark.f) \
                (ptr_)->watermark.f = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) > head->watermark.f) { \
                head->watermark.f = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_UINT_HIGHWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_UINT_HIGHWATERMARK_INIT_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG_HIGHWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_ULONG_HIGHWATERMARK_INIT_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG2_HIGHWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_ULONG2_HIGHWATERMARK_INIT_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)
#define MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_INIT_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)

#define MPIR_T_PVAR_UINT_HIGHWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_UINT_HIGHWATERMARK_UPDATE_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG_HIGHWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_ULONG_HIGHWATERMARK_UPDATE_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG2_HIGHWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_ULONG2_HIGHWATERMARK_UPDATE_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)
#define MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_UPDATE_VAR_impl(&PVAR_HIGHWATERMARK_##name_, val_)

/* Registration AND initialization for static pvar  */
#define MPIR_T_PVAR_HIGHWATERMARK_REGISTER_STATIC_impl(dtype_, name_, \
            initval_, verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        switch (dtype_) { \
        case MPI_UNSIGNED: \
            MPIR_T_PVAR_UINT_HIGHWATERMARK_INIT_impl(name_, initval_); break; \
        case MPI_UNSIGNED_LONG: \
            MPIR_T_PVAR_ULONG_HIGHWATERMARK_INIT_impl(name_, initval_); break; \
        case MPI_UNSIGNED_LONG_LONG: \
            MPIR_T_PVAR_ULONG2_HIGHWATERMARK_INIT_impl(name_, initval_); break; \
        case MPI_DOUBLE: \
            MPIR_T_PVAR_DOUBLE_HIGHWATERMARK_INIT_impl(name_, initval_); break; \
        default: \
            break; \
        }; \
        addr_ = &PVAR_HIGHWATERMARK_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_HIGHWATERMARK, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_HIGHWATERMARK_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_HIGHWATERMARK, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)


/* MPI_T_PVAR_CLASS_LOWWATERMARK (continuous or not)
 */

#define MPIR_T_PVAR_UINT_LOWWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_UNSIGNED; \
        (ptr_)->current.u = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_ULONG_LOWWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_UNSIGNED_LONG; \
        (ptr_)->current.ul = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_ULONG2_LOWWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_UNSIGNED_LONG_LONG; \
        (ptr_)->current.ull = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_DOUBLE_LOWWATERMARK_INIT_VAR_impl(ptr_, val_) \
    do { \
        (ptr_)->datatype = MPI_DOUBLE; \
        (ptr_)->current.f = (val_); \
        (ptr_)->first_started = 0;    \
        (ptr_)->first_used = 0; \
        (ptr_)->hlist = NULL;  \
    } while (0)

#define MPIR_T_PVAR_UINT_LOWWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.u = (val_); \
        /* Update values in all handles */ \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) < (ptr_)->watermark.u) \
                (ptr_)->watermark.u = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) < head->watermark.u) { \
                head->watermark.u = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_ULONG_LOWWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.ul = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) < (ptr_)->watermark.ul) \
                (ptr_)->watermark.ul = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) < head->watermark.ul) { \
                head->watermark.ul = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_ULONG2_LOWWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.ull = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) < (ptr_)->watermark.ull) \
                (ptr_)->watermark.ull = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) < head->watermark.ull) { \
                head->watermark.ull = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_DOUBLE_LOWWATERMARK_UPDATE_VAR_impl(ptr_, val_) \
    do { \
        MPIR_T_pvar_handle_t *head; \
        (ptr_)->current.f = (val_); \
        if ((ptr_)->first_used && (ptr_)->first_started) { \
            if ((val_) < (ptr_)->watermark.f) \
                (ptr_)->watermark.f = (val_); \
        } \
        head = (ptr_)->hlist; \
        while (head != NULL) { \
            if (MPIR_T_pvar_is_started(head) && (val_) < head->watermark.f) { \
                head->watermark.f = (val_); \
            } \
            head = head->next2; \
        } \
    } while (0)

#define MPIR_T_PVAR_UINT_LOWWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_UINT_LOWWATERMARK_INIT_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG_LOWWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_ULONG_LOWWATERMARK_INIT_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG2_LOWWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_ULONG2_LOWWATERMARK_INIT_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)
#define MPIR_T_PVAR_DOUBLE_LOWWATERMARK_INIT_impl(name_, val_) \
    MPIR_T_PVAR_DOUBLE_LOWWATERMARK_INIT_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)

#define MPIR_T_PVAR_UINT_LOWWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_UINT_LOWWATERMARK_UPDATE_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG_LOWWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_ULONG_LOWWATERMARK_UPDATE_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)
#define MPIR_T_PVAR_ULONG2_LOWWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_ULONG2_LOWWATERMARK_UPDATE_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)
#define MPIR_T_PVAR_DOUBLE_LOWWATERMARK_UPDATE_impl(name_, val_) \
    MPIR_T_PVAR_DOUBLE_LOWWATERMARK_UPDATE_VAR_impl(&PVAR_LOWWATERMARK_##name_, val_)

/* Registration AND initialization for static pvar  */
#define MPIR_T_PVAR_LOWWATERMARK_REGISTER_STATIC_impl(dtype_, name_, \
            initval_, verb_, bind_, flags_, cat_, desc_) \
    do { \
        void *addr_; \
        /* Allowable datatypes only */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        switch (dtype_) { \
        case MPI_UNSIGNED: \
            MPIR_T_PVAR_UINT_LOWWATERMARK_INIT_impl(name_, initval_); break; \
        case MPI_UNSIGNED_LONG: \
            MPIR_T_PVAR_ULONG_LOWWATERMARK_INIT_impl(name_, initval_); break; \
        case MPI_UNSIGNED_LONG_LONG: \
            MPIR_T_PVAR_ULONG2_LOWWATERMARK_INIT_impl(name_, initval_); break; \
        case MPI_DOUBLE: \
            MPIR_T_PVAR_DOUBLE_LOWWATERMARK_INIT_impl(name_, initval_); break; \
        default: \
            break; \
        }; \
        addr_ = &PVAR_LOWWATERMARK_##name_; \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_LOWWATERMARK, dtype_, #name_, \
            addr_, 1, MPI_T_ENUM_NULL, verb_, bind_, flags_, NULL, NULL, cat_, desc_); \
    } while (0)

/* Registration for dynamic pvar w/ or w/o callback. Init is left to users */
#define MPIR_T_PVAR_LOWWATERMARK_REGISTER_DYNAMIC_impl(dtype_, name_, \
            addr_, count_, verb_, bind_, flags_, get_value_, get_count_, cat_, desc_) \
    do { \
        /* Allowable datatypes */ \
        MPIR_Assert((dtype_) == MPI_UNSIGNED || (dtype_) == MPI_UNSIGNED_LONG || \
                    (dtype_) == MPI_UNSIGNED_LONG_LONG || (dtype_) == MPI_DOUBLE); \
        MPIR_Assert((addr_) != NULL || (get_value_) != NULL); \
        MPIR_T_PVAR_REGISTER_impl(MPI_T_PVAR_CLASS_LOWWATERMARK, dtype_, #name_, \
            addr_, count_, MPI_T_ENUM_NULL, verb_, bind_, flags_, get_value_, \
            get_count_, cat_, desc_); \
    } while (0)

/* Unregister a pvar by its index */
#define MPIR_T_PVAR_UNREGISTER(idx_) \
    do { \
        if (pvar_table != NULL) { \
            pvar_table_entry_t *pvar = \
                (pvar_table_entry_t *)utarray_eltptr(pvar_table, idx_); \
            if (pvar != NULL) { \
                pvar->active = FALSE; \
                /* Do not do MPL_free(pvar->info), since it may be re-activated */ \
            } \
        } \
    } while (0)

static inline int MPIR_T_pvar_is_readonly(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_READONLY;
}

static inline int MPIR_T_pvar_is_continuous(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_CONTINUOUS;
}

static inline int MPIR_T_pvar_is_atomic(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_ATOMIC;
}

static inline int MPIR_T_pvar_is_sum(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_SUM;
}

static inline int MPIR_T_pvar_is_watermark(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_WATERMARK;
}

static inline int MPIR_T_pvar_is_started(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_STARTED;
}

static inline void MPIR_T_pvar_set_started(MPIR_T_pvar_handle_t * handle)
{
    handle->flags |= (MPIR_T_PVAR_FLAG_STARTED | MPIR_T_PVAR_FLAG_ONCESTARTED);
}

static inline void MPIR_T_pvar_unset_started(MPIR_T_pvar_handle_t * handle)
{
    handle->flags &= ~MPIR_T_PVAR_FLAG_STARTED;
}

static inline int MPIR_T_pvar_is_oncestarted(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_ONCESTARTED;
}

static inline void MPIR_T_pvar_unset_oncestarted(MPIR_T_pvar_handle_t * handle)
{
    handle->flags &= ~MPIR_T_PVAR_FLAG_ONCESTARTED;
}

static inline int MPIR_T_pvar_is_first(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags & MPIR_T_PVAR_FLAG_FIRST;
}

static inline int MPIR_T_pvar_set_first(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags |= MPIR_T_PVAR_FLAG_FIRST;
}

static inline int MPIR_T_pvar_unset_first(MPIR_T_pvar_handle_t * handle)
{
    return handle->flags &= ~MPIR_T_PVAR_FLAG_FIRST;
}

/* A counter that keeps track of the relative balance of calls to
 * MPI_T_init_thread and MPI_T_finalize */
extern int MPIR_T_init_balance;
static inline int MPIR_T_is_initialized(void)
{
    return MPIR_T_init_balance > 0;
}

/* Helper function to update count and volume of send and recv messages for PVARS */
#if ENABLE_PVAR_MVP
#define MPIR_PVAR_INC(_mpicoll, _algo, _operation, _count, _datatype)          \
    do {                                                                       \
        int _pSize = 0;                                                        \
        MPIR_Datatype_get_size_macro(_datatype, _pSize);                       \
        int _size = _count * _pSize;                                           \
        if (_size < 0) {                                                       \
            _size = 0;                                                         \
        }                                                                      \
        MPIR_T_PVAR_COUNTER_INC(                                               \
            MVP, mvp_coll_##_mpicoll##_##_algo##_bytes_##_operation, _size);   \
        MPIR_T_PVAR_COUNTER_INC(                                               \
            MVP, mvp_coll_##_mpicoll##_##_algo##_count_##_operation, 1);       \
        /* MPIR_T_PVAR_COUNTER_INC(MVP,                                        \
         * mvp_coll_##_mpicoll##_bytes_##_operation, _size);*/                 \
        /* MPIR_T_PVAR_COUNTER_INC(MVP,                                        \
         * mvp_coll_##_mpicoll##_count_##_operation, 1); */                    \
    } while (0)
#else /*ENABLE_PVAR_MVP*/
    #define MPIR_PVAR_INC(_mpicoll, _algo, _operation, _count, _datatype)
#endif /*ENABLE_PVAR_MVP*/

/* Helper functions to start/end MVP PVAR timers */

#if ENABLE_PVAR_MVP
#define MPIR_TIMER_START(_optype, _op, _algo)                                  \
    do {                                                                       \
        if (MVP_ENABLE_PVAR_TIMER)                                             \
            MPIR_T_PVAR_TIMER_START(MVP,                                       \
                                    mvp_##_optype##_timer_##_op##_##_algo);    \
    } while (0)
#define MPIR_TIMER_END(_optype, _op, _algo)                                    \
    do {                                                                       \
        if (MVP_ENABLE_PVAR_TIMER)                                             \
            MPIR_T_PVAR_TIMER_END(MVP, mvp_##_optype##_timer_##_op##_##_algo); \
    } while (0)
#else /*ENABLE_PVAR_MVP*/
    #define MPIR_TIMER_START(_optype,_op,_algo)                                                  
    #define MPIR_TIMER_END(_optype,_op,_algo)
#endif /*ENABLE_PVAR_MVP*/



/* A special strncpy to return strings in behavior defined by MPI_T */
extern void MPIR_T_strncpy(char *dst, const char *src, int *len);

/* Stuffs to support multithreaded MPI_T */
extern int MPIR_T_is_threaded;
#define MPIR_T_THREAD_CHECK_BEGIN if (MPIR_T_is_threaded) {
#define MPIR_T_THREAD_CHECK_END }

#ifdef MPICH_IS_THREADED
extern MPID_Thread_mutex_t mpi_t_mutex;
#define MPIR_T_THREAD_CS_INIT() \
    do { \
        int err_; \
        MPIR_T_THREAD_CHECK_BEGIN \
        MPID_Thread_init(&err_); \
        MPIR_Assert(err_ == 0); \
        MPID_Thread_mutex_create(&mpi_t_mutex, &err_); \
        MPIR_Assert(err_ == 0); \
        MPIR_T_THREAD_CHECK_END \
    } while (0)

#define MPIR_T_THREAD_CS_FINALIZE() \
    do { \
        int err_; \
        MPIR_T_THREAD_CHECK_BEGIN \
        MPID_Thread_mutex_destroy(&mpi_t_mutex, &err_); \
        MPIR_Assert(err_ == 0); \
        MPID_Thread_finalize(&err_); \
        MPIR_Assert(err_ == 0); \
        MPIR_T_THREAD_CHECK_END \
    } while (0)

#define MPIR_T_THREAD_CS_ENTER() \
    do { \
        int err;                              \
        MPIR_T_THREAD_CHECK_BEGIN             \
            MPID_Thread_mutex_lock(&mpi_t_mutex,&err);  \
        MPIR_T_THREAD_CHECK_END \
    } while (0)

#define MPIR_T_THREAD_CS_EXIT() \
    do { \
        int err;                  \
        MPIR_T_THREAD_CHECK_BEGIN \
            MPID_Thread_mutex_unlock(&mpi_t_mutex,&err);  \
        MPIR_T_THREAD_CHECK_END \
    } while (0)
#else /* !MPICH_IS_THREADED */
#define MPIR_T_THREAD_CS_INIT()     do { /* nothing */ } while (0)
#define MPIR_T_THREAD_CS_FINALIZE() do { /* nothing */ } while (0)
#define MPIR_T_THREAD_CS_ENTER()    do { /* nothing */ } while (0)
#define MPIR_T_THREAD_CS_EXIT()     do { /* nothing */ } while (0)
#endif

/* Hash tables to quick locate category, cvar, pvar indices by their names */
extern name2index_hash_t *cvar_hash;

static inline int LOOKUP_CVAR_INDEX_BY_NAME_impl(const char* cvar_name)
{
    int cvar_idx = -1;
    name2index_hash_t *hash_entry = NULL;

    /* Do hash lookup by the name */
    HASH_FIND_STR(cvar_hash, cvar_name, hash_entry);
    if (hash_entry != NULL) {
        cvar_idx = hash_entry->idx;
    }
    return cvar_idx;
}

#define MPIR_CVAR_GET_INDEX_impl(name_, out_val_)           \
    do {                                                    \
        (out_val_) = LOOKUP_CVAR_INDEX_BY_NAME_impl(#name_);\
    } while (0)

/* Init and finalize routines */
extern int MPIR_T_env_init(void);
extern void MPIR_T_env_finalize(void);

#endif /* MPITIMPL_H_INCLUDED */

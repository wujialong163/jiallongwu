/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */

#include <mpidimpl.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <search.h>
#include <stdint.h>
#include <sys/shm.h>

#ifdef USE_MEMORY_TRACING
#define mpit_malloc(a, b, line, file)     MPL_trmalloc(a, b, line, file)
#define mpit_calloc(a, b, c, line, file)  MPL_trcalloc(a, b, c, line, file)
#define mpit_free(a, line, file)          MPL_trfree(a, line, file)
#define mpit_strdup(a, line, file)        MPL_trstrdup(a, line, file)
#define mpit_realloc(a, b, c, line, file) MPL_trrealloc(a, b, c, line, file)
#define mpit_memalign(a, b, c, line, file)                                     \
    MPL_traligned_alloc(a, b, c, line, file)
#else
#define mpit_malloc(a, b, line, file)      malloc((size_t)a)
#define mpit_calloc(a, b, c, line, file)   calloc((size_t)a, (size_t)b)
#define mpit_free(a, line, file)           free((void *)a)
#define mpit_strdup(a, line, file)         strdup(a)
#define mpit_realloc(a, b, c, line, file)  realloc((void *)a, (size_t)b)
#define mpit_memalign(a, b, c, line, file) MPL_aligned_alloc(a, b, c)
/*posix_memalign(a, b, c) */
#endif

static unsigned long long PVAR_LEVEL_mem_allocated;
static MPIR_T_pvar_watermark_t PVAR_HIGHWATERMARK_mem_allocated;
static unsigned long PVAR_COUNTER_num_malloc_calls;
static unsigned long PVAR_COUNTER_num_calloc_calls;
static unsigned long PVAR_COUNTER_num_memalign_calls;
static unsigned long PVAR_COUNTER_num_strdup_calls;
static unsigned long PVAR_COUNTER_num_realloc_calls;
static unsigned long PVAR_COUNTER_num_free_calls;
static unsigned long PVAR_COUNTER_num_memalign_free_calls;

typedef struct {
    void *addr;
    size_t size;
} MPIT_MEMORY_T;

static void *oracle = NULL;

/*
 * This variable is used to count memory before MPIT is initialized
 */
static size_t unaccounted = 0;
static size_t unaccounted_malloc = 0;
static size_t unaccounted_calloc = 0;
static size_t unaccounted_memalign = 0;
static size_t unaccounted_strdup = 0;
static size_t unaccounted_realloc = 0;
static size_t unaccounted_free = 0;
static size_t unaccounted_memalign_free = 0;

static int initialized = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t oracle_mutex = PTHREAD_MUTEX_INITIALIZER;

void Real_Free(void *ptr);

static inline void increment_malloc_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_malloc_calls, 1);
    }

    else {
        unaccounted_malloc++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_calloc_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_calloc_calls, 1);
    }

    else {
        unaccounted_calloc++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_realloc_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_realloc_calls, 1);
    }

    else {
        unaccounted_realloc++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_free_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_free_calls, 1);
    }

    else {
        unaccounted_free++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_memalign_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_memalign_calls, 1);
    }

    else {
        unaccounted_memalign++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_memalign_free_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_memalign_free_calls, 1);
    }

    else {
        unaccounted_memalign_free++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_strdup_counter(void)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_COUNTER_INC(MVP, num_strdup_calls, 1);
    }

    else {
        unaccounted_strdup++;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

static int ptr_cmp(const void *mptr1, const void *mptr2)
{
    uintptr_t addr1 = (uintptr_t)((MPIT_MEMORY_T *)mptr1)->addr;
    uintptr_t addr2 = (uintptr_t)((MPIT_MEMORY_T *)mptr2)->addr;

    if (addr1 == addr2) {
        return 0;
    }

    return addr1 < addr2 ? -1 : 1;
}

#if 0
/*
 * This function is used for debugging purposes only
 */
static void
ptr_print (const void * node, const VISIT which, const int depth)
{
    MPIT_MEMORY_T * data;
    int i = 0;

    switch (which) {
        case preorder:
            break;
        case postorder:
            data = *(MPIT_MEMORY_T **)node;
            for (i = 0; i < depth; i++) printf("*");
            printf("[%p: %ld]\n", data->addr, data->size);
            fflush(stdout);
            break;
        case endorder:
            break;
        case leaf:
            data = *(MPIT_MEMORY_T **)node;
            for (i = 0; i < depth; i++) printf("*");
            printf("[%p: %ld]\n", data->addr, data->size);
            fflush(stdout);
            break;
    }
}
#endif

static MPIT_MEMORY_T *oracle_insert(void *ptr, size_t size)
{
    MPIT_MEMORY_T *mptr =
        mpit_malloc(sizeof(MPIT_MEMORY_T), MPL_MEM_MPIT, __LINE__, __FILE__);
    void *result;

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&oracle_mutex);
    MPIR_THREAD_CHECK_END

    if (mptr) {
        mptr->addr = ptr;
        mptr->size = size;
        result = tsearch(mptr, &oracle, ptr_cmp);
        mptr = result ? *(MPIT_MEMORY_T **)result : NULL;
        fflush(stdout);
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&oracle_mutex);
    MPIR_THREAD_CHECK_END

    return mptr;
}

static MPIT_MEMORY_T *oracle_find(void *ptr)
{
    MPIT_MEMORY_T m = {.addr = ptr};
    MPIT_MEMORY_T *mptr;
    void *result;

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&oracle_mutex);
    MPIR_THREAD_CHECK_END
    result = tfind(&m, &oracle, ptr_cmp);
    mptr = result ? *(MPIT_MEMORY_T **)result : NULL;
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&oracle_mutex);
    MPIR_THREAD_CHECK_END

    return mptr;
}

static void oracle_delete(MPIT_MEMORY_T *ptr)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&oracle_mutex);
    MPIR_THREAD_CHECK_END
    tdelete(ptr, &oracle, ptr_cmp);
    mpit_free(ptr, __LINE__, __FILE__);
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&oracle_mutex);
    MPIR_THREAD_CHECK_END
}

static inline void increment_counter(signed long size)
{
    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END

    if (initialized) {
        MPIR_T_PVAR_LEVEL_INC(MVP, mem_allocated, size);
        MPIR_T_PVAR_ULONG2_HIGHWATERMARK_UPDATE(MVP, mem_allocated,
                                                PVAR_LEVEL_mem_allocated);
    }

    else {
        unaccounted += size;
    }

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

void MPIT_MEM_REGISTER_PVARS(void)
{
    MPIR_T_PVAR_LEVEL_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG_LONG, mem_allocated, 0, /* initial value */
        MPI_T_VERBOSITY_USER_BASIC, MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "CH3", /* category name */
        "Current level of allocated memory within the MPI library");
    MPIR_T_PVAR_HIGHWATERMARK_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG_LONG, mem_allocated, 0, /* initial value */
        MPI_T_VERBOSITY_USER_BASIC, MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "CH3", /* category name */
        "Maximum level of memory ever allocated within the MPI library");

    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_malloc_calls, MPI_T_VERBOSITY_TUNER_DETAIL,
        MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_malloc calls");

    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_calloc_calls, MPI_T_VERBOSITY_TUNER_DETAIL,
        MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_calloc calls");

    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_memalign_calls,
        MPI_T_VERBOSITY_TUNER_DETAIL, MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_memalign calls");

    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_strdup_calls, MPI_T_VERBOSITY_TUNER_DETAIL,
        MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_strdup calls");

    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_realloc_calls, MPI_T_VERBOSITY_TUNER_DETAIL,
        MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_realloc calls");

    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_free_calls, MPI_T_VERBOSITY_TUNER_DETAIL,
        MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_free calls");
    MPIR_T_PVAR_COUNTER_REGISTER_STATIC(
        MVP, MPI_UNSIGNED_LONG, num_memalign_free_calls,
        MPI_T_VERBOSITY_TUNER_DETAIL, MPI_T_BIND_NO_OBJECT,
        (MPIR_T_PVAR_FLAG_READONLY | MPIR_T_PVAR_FLAG_CONTINUOUS),
        "MEM", /* category name */
        "Number of MPIT_memalign_free calls");

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_lock(&mutex);
    MPIR_THREAD_CHECK_END
    initialized = 1;
    MPIR_T_PVAR_LEVEL_INC(MVP, mem_allocated, unaccounted);
    MPIR_T_PVAR_ULONG2_HIGHWATERMARK_UPDATE(MVP, mem_allocated,
                                            PVAR_LEVEL_mem_allocated);

    MPIR_T_PVAR_COUNTER_INC(MVP, num_malloc_calls, unaccounted_malloc);
    MPIR_T_PVAR_COUNTER_INC(MVP, num_calloc_calls, unaccounted_calloc);
    MPIR_T_PVAR_COUNTER_INC(MVP, num_memalign_calls, unaccounted_memalign);
    MPIR_T_PVAR_COUNTER_INC(MVP, num_strdup_calls, unaccounted_strdup);
    MPIR_T_PVAR_COUNTER_INC(MVP, num_realloc_calls, unaccounted_realloc);
    MPIR_T_PVAR_COUNTER_INC(MVP, num_free_calls, unaccounted_free);
    MPIR_T_PVAR_COUNTER_INC(MVP, num_memalign_free_calls,
                            unaccounted_memalign_free);

    MPIR_THREAD_CHECK_BEGIN
    pthread_mutex_unlock(&mutex);
    MPIR_THREAD_CHECK_END
}

void *MPIT_malloc(size_t size, MPL_memory_class class, int lineno,
                  char const *filename)
{
    void *ptr;

    if (MVP_ENABLE_PVAR_MEM) {
        ptr = mpit_malloc(size, class, lineno, filename);

        if (ptr) {
            increment_counter(size);
            oracle_insert(ptr, size);
        }
    } else {
        ptr = mpit_malloc(size, class, lineno, filename);
    }

    return ptr;
}

void *MPIT_calloc(size_t nelements, size_t elementSize, MPL_memory_class class,
                  int lineno, char const *filename)
{
    void *ptr;

    if (MVP_ENABLE_PVAR_MEM) {
        increment_calloc_counter();
        ptr = mpit_calloc(nelements, elementSize, class, lineno, filename);

        size_t size = nelements * elementSize;

        if (ptr) {
            increment_counter(size);
            oracle_insert(ptr, size);
        }
    } else {
        ptr = mpit_calloc(nelements, elementSize, class, lineno, filename);
    }

    return ptr;
}

void *MPIT_memalign(size_t alignment, size_t size, MPL_memory_class class,
                    int lineno, char const *filename)
{
    void *ptr;

    if (MVP_ENABLE_PVAR_MEM) {
        increment_memalign_counter();

        ptr = mpit_memalign(alignment, size, class, lineno, filename);

        if (!ptr) {
            increment_counter(size);
            oracle_insert(ptr, size);
        }
    } else {
        ptr = mpit_memalign(alignment, size, class, lineno, filename);
    }
    return ptr;
}

char *MPIT_strdup(const char *s, int lineno, char const *filename)
{
    char *ptr;

    if (MVP_ENABLE_PVAR_MEM) {
        increment_strdup_counter();
        ptr = mpit_strdup(s, lineno, filename);
        size_t size = strlen(s);
        if (ptr) {
            increment_counter(size);
            oracle_insert(ptr, size);
        }
    } else {
        ptr = mpit_strdup(s, lineno, filename);
    }

    return ptr;
}

void *MPIT_realloc(void *ptr, size_t size, MPL_memory_class class, int lineno,
                   char const *filename)
{
    if (MVP_ENABLE_PVAR_MEM) {
        increment_realloc_counter();

        if (ptr) {
            MPIT_MEMORY_T *mptr = oracle_find(ptr);
            size_t oldsize;
            MPIR_Assert(NULL != mptr);
            oldsize = mptr->size;
            ptr = mpit_realloc(ptr, size, class, lineno, filename);
            if (ptr) {
                oracle_delete(mptr);
                oracle_insert(ptr, size);
                increment_counter(size - oldsize);
            } else if (!size) {
                oracle_delete(mptr);
                increment_counter(size - oldsize);
            }
        } else {
            ptr = mpit_realloc(ptr, size, class, lineno, filename);
            if (ptr) {
                oracle_insert(ptr, size);
                increment_counter(size);
            }
        }
    } else {
        ptr = mpit_realloc(ptr, size, class, lineno, filename);
    }
    return ptr;
}

void MPIT_free(void *ptr, int lineno, char const *filename)
{
    if (MVP_ENABLE_PVAR_MEM) {
        increment_free_counter();
        size_t oldsize = 0;

        if (ptr) {
            MPIT_MEMORY_T *mptr = oracle_find(ptr);

            if (mptr) {
                oldsize = mptr->size;
                oracle_delete(mptr);
            }
        }

        mpit_free(ptr, lineno, filename);
        increment_counter(0 - oldsize);
    } else {
        mpit_free(ptr, lineno, filename);
    }
}

void MPIT_memalign_free(void *ptr, int lineno, char const *filename)
{
    if (MVP_ENABLE_PVAR_MEM) {
        increment_memalign_free_counter();
        size_t oldsize = 0;

        if (ptr) {
            MPIT_MEMORY_T *mptr = oracle_find(ptr);

            if (mptr) {
                oldsize = mptr->size;
                oracle_delete(mptr);
            }
        }

        Real_Free(ptr);
        increment_counter(0 - oldsize);
    } else {
        Real_Free(ptr);
    }
}

void MPIT_shmdt(void *ptr, int lineno, char const *filename)
{
    if (MVP_ENABLE_PVAR_MEM) {
        increment_memalign_free_counter();
        size_t oldsize = 0;

        if (ptr) {
            MPIT_MEMORY_T *mptr = oracle_find(ptr);

            if (mptr) {
                oldsize = mptr->size;
                oracle_delete(mptr);
            }
        }

        shmdt(ptr);
        increment_counter(0 - oldsize);
    } else {
        shmdt(ptr);
    }
}

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "mvp_debug_utils.h"
#include "mpiimpl.h"
#include <unistd.h>
#include <execinfo.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


// Prefix to distinguish output from different processes
#define OUTPUT_PREFIX_LENGTH 256
char output_prefix[OUTPUT_PREFIX_LENGTH] = "";

void set_output_prefix(char *prefix)
{
    strncpy(output_prefix, prefix, OUTPUT_PREFIX_LENGTH);
    output_prefix[OUTPUT_PREFIX_LENGTH - 1] = '\0';
}

const char *get_output_prefix() { return output_prefix; }

// Verbosity level for sharp  operations in collectives
int DEBUG_Sharp_verbose = 0;

// Verbosity level for fork/kill/waitpid operations in mpirun_rsh and mpispawn
int DEBUG_Fork_verbose = 0;

// Verbosity level for Fault Tolerance operations
int DEBUG_FT_verbose = 0;

// Verbosity level for Checkpoint/Restart operations
int DEBUG_CR_verbose = 0;

// Verbosity level for Migration operations
int DEBUG_MIG_verbose = 0;

// Verbosity level for UD flow control
int DEBUG_UD_verbose = 0;

// Verbosity level for UD ZCOPY Rndv
int DEBUG_ZCY_verbose = 0;

// Verbosity level for On-Demand Connection Management
int DEBUG_CM_verbose = 0;

// Verbosity level for XRC.
int DEBUG_XRC_verbose = 0;

// Verbosity level for UD stats
int DEBUG_UDSTAT_verbose = 0;

// Verbosity level for memory stats
int DEBUG_MEM_verbose = 0;

// Verbosity level for GPU CUDA
int DEBUG_CUDA_verbose = 0;

// Verbosity level for IB MULTICAST
int DEBUG_MCST_verbose = 0;

// Verbosity level for SHMEM Collectives
int DEBUG_SHM_verbose;

// Verbosity level for Channel manager
int DEBUG_CHM_verbose;

// Verbosity level for RNDV transfers
int DEBUG_RNDV_verbose;

// Verbosity level for Init phase
int DEBUG_INIT_verbose;

// Verbosity level for RDMA_CM
int DEBUG_RDMACM_verbose;

// Verbosity level for One-sided
int DEBUG_1SC_verbose;

// Verbosity level for dreg cache
int DEBUG_DREG_verbose;

// Verbosity level for vbuf cache
int DEBUG_VBUF_verbose;

// Verbosity level for P2P send
int DEBUG_SEND_verbose;

static inline int env2int(char *name)
{
    char *env_str = getenv(name);
    if (env_str == NULL) {
        return 0;
    } else {
        return atoi(env_str);
    }
}

#define MAX_DEPTH 100

// Print backtrace of the current thread
int print_backtrace()
{
    void *trace[MAX_DEPTH];
    unsigned int trace_size;
    char **trace_strings;

    // Get backtrace and symbols
    trace_size = backtrace(trace, MAX_DEPTH);
    trace_strings = backtrace_symbols(trace, trace_size);
    if ( trace_strings == NULL ) {
        PRINT_ERROR( "backtrace_symbols: error\n" );
        return -1;
    }

    // Print backtrace
    unsigned int i;
    for ( i = 0 ; i < trace_size ; ++i )
    {
        PRINT_ERROR( "%3i: %s\n", i, trace_strings[i] );
    }

    // Free trace_strings allocated by backtrace_symbols()
    MPL_free(trace_strings);

    return 0;
}

// Enable/disable backtrace on error
int show_backtrace = 0;

// Signal handler for errors
void error_sighandler(int sig, siginfo_t *info, void *secret) {
    // Always print error
    PRINT_ERROR( "Caught error: %s (signal %d)\n", strsignal(sig), sig );
    // Show backtrace if required
    if (show_backtrace) print_backtrace();
    // Raise the signal again with default handler
    raise( sig );
}

int setup_error_sighandler_helper( int signal ) {
    int rv;

    // Get the current signal handler
    struct sigaction old_sa;
    rv = sigaction(signal , NULL, &old_sa);
    if ( 0 != rv ) {
        PRINT_ERROR_ERRNO( "sigaction(): failed to read old signal handler for signal %d", errno, signal );
        return -1;
    }

    // Check for an existing signal handler (eg setup by the user)
    if ( old_sa.sa_handler != SIG_DFL && old_sa.sa_handler != SIG_IGN ) {
        // Do not overwrite a signal handler setup by the user
        // Silently return
        return -2;
    }

    // Setup the new handler
    struct sigaction sa;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_RESETHAND;
    sa.sa_sigaction = error_sighandler;
    rv = sigaction(signal , &sa, NULL);
    if ( 0 != rv ) {
        PRINT_ERROR_ERRNO( "sigaction(): failed to setup a new signal handler for signal %d", errno, signal );
        return -3;
    }

    return 0;
}

// Configure the error signal handler
int setup_error_sighandler( int backtrace ) {
    // Enable backtrace?
    show_backtrace = backtrace;

    // Setup the handler for these signals
    setup_error_sighandler_helper( SIGILL );
    setup_error_sighandler_helper( SIGABRT );
    setup_error_sighandler_helper( SIGFPE );
    setup_error_sighandler_helper( SIGSEGV );
    setup_error_sighandler_helper( SIGBUS );
    // All return codes are ignored because
    // this is not required for MVAPICH to work properly

    return 0;
}

// Set the core dump size according to coresize parameter
int set_coresize_limit( const char* coresize )
{
    if ( coresize != NULL && strcmp( coresize, "default" ) != 0 ) {
        struct rlimit core_limit;
        int rv;
        // read current rlimit structure
        rv = getrlimit( RLIMIT_CORE, &core_limit );
        if ( rv != 0 ) {
            PRINT_ERROR_ERRNO( "getrlimit", errno );
            return -1;
        }
        // update the rlimit structure
        if ( strcmp( coresize, "unlimited") == 0 ) {
            core_limit.rlim_cur = RLIM_INFINITY;
        } else {
            core_limit.rlim_cur = atoi( coresize );
        }
        // apply new rlimit structure
        rv = setrlimit(RLIMIT_CORE,&core_limit);
        if ( rv != 0 )
        {
            PRINT_ERROR_ERRNO( "setrlimit", errno );
            return -1;
        }
    }
    return 0;
}

// Initialize the verbosity level of the above variables
int initialize_debug_variables()
{
    DEBUG_Sharp_verbose = env2int("MVP_DEBUG_SHARP_VERBOSE");
    DEBUG_Fork_verbose = env2int("MVP_DEBUG_FORK_VERBOSE");
    DEBUG_FT_verbose = env2int("MVP_DEBUG_FT_VERBOSE");
    DEBUG_CR_verbose = env2int("MVP_DEBUG_CR_VERBOSE");
    DEBUG_MIG_verbose = env2int("MVP_DEBUG_MIG_VERBOSE");
    DEBUG_UD_verbose = env2int("MVP_DEBUG_UD_VERBOSE");
    DEBUG_ZCY_verbose = env2int("MVP_DEBUG_ZCOPY_VERBOSE");
    DEBUG_CM_verbose = env2int("MVP_DEBUG_CM_VERBOSE");
    DEBUG_XRC_verbose = env2int("MVP_DEBUG_XRC_VERBOSE");
    DEBUG_UDSTAT_verbose = env2int("MVP_DEBUG_UDSTAT_VERBOSE");
    DEBUG_MEM_verbose = env2int("MVP_DEBUG_MEM_USAGE_VERBOSE");
    DEBUG_CUDA_verbose = env2int("MVP_DEBUG_CUDA_VERBOSE");
    DEBUG_MCST_verbose = env2int("MVP_DEBUG_MCST_VERBOSE");
    DEBUG_SHM_verbose = env2int("MVP_DEBUG_SHM_VERBOSE");
    DEBUG_CHM_verbose = env2int("MVP_DEBUG_CHM_VERBOSE");
    DEBUG_RNDV_verbose = env2int("MVP_DEBUG_RNDV_VERBOSE");
    DEBUG_INIT_verbose = env2int("MVP_DEBUG_INIT_VERBOSE");
    DEBUG_RDMACM_verbose = env2int("MVP_DEBUG_RDMACM_VERBOSE");
    DEBUG_1SC_verbose = env2int("MVP_DEBUG_1SC_VERBOSE");
    DEBUG_DREG_verbose = env2int("MVP_DEBUG_DREG_VERBOSE");
    DEBUG_VBUF_verbose = env2int("MVP_DEBUG_VBUF_VERBOSE");
    DEBUG_SEND_verbose = env2int("MVP_DEBUG_SEND_VERBOSE");
    return 0;
}

void mvp_init_debug() {
    /* Set coresize limit */
    char *value = NULL;
    int backtrace = 0;
    int setup_sighandler = 1;

    set_coresize_limit(MVP_DEBUG_CORESIZE);

    /* Set an error signal handler */
    backtrace = MVP_DEBUG_SHOW_BACKTRACE;

    value = getenv("MVP_DEBUG_SETUP_SIGHDLR");
    if (value != NULL) {
        setup_sighandler = !!atoi(value);
    }

    if (setup_sighandler) {
        setup_error_sighandler(backtrace);
    }

    /* Set prefix for debug output */
    const int MAX_LENGTH = 256;
    char hostname[MAX_LENGTH];
    gethostname(hostname, MAX_LENGTH);
    hostname[MAX_LENGTH-1] = '\0';
    char output_prefix[MAX_LENGTH];
    snprintf(output_prefix, MAX_LENGTH, "%s:mpi_rank_%i", hostname, MPIR_Process.rank);
    set_output_prefix(output_prefix);

    /* Initialize DEBUG variables */
    initialize_debug_variables();
}

void mvp_print_mem_usage()
{
    FILE *file = fopen("/proc/self/status", "r");
    char vmpeak[100], vmhwm[100];

    if (file != NULL) {
        char line[100];
        while (fgets(line, 100, file) != NULL) {
            if (strstr(line, "VmPeak") != NULL) {
                strcpy(vmpeak, line);
                vmpeak[strcspn(vmpeak, "\n")] = '\0';
            }
            if (strstr(line, "VmHWM") != NULL) {
                strcpy(vmhwm, line);
                vmhwm[strcspn(vmhwm, "\n")] = '\0';
            }
        }
        PRINT_INFO(DEBUG_MEM_verbose, "%s %s\n", vmpeak, vmhwm);
        fclose(file);
    } else {
        PRINT_INFO(DEBUG_MEM_verbose, "Status file could not be opened \n");
    }
}

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

#ifndef OFED_ABSTRACTION_H
#define OFED_ABSTRACTION_H

#include <infiniband/verbs.h>

#ifdef RDMA_CM
#include <rdma/rdma_cma.h>
#include <semaphore.h>
#include <pthread.h>
#endif /*RDMA_CM*/

#if defined(HAVE_LIBIBUMAD)

#ifdef USE_MEMORY_TRACING
/* umad_alloc and umad_free call calloc and free underneath. Due to the
 * restructuring of the way header files are getting included, this usage is
 * conflicting with the memory tracing being used as part of MPICH
 * (--enable-g=all). This is to ensure that the memory tracing
 * infrastructure does not tag these calls from underlying libraries */
#undef calloc
#undef free
#define calloc calloc
#define free   free
#endif

#include <infiniband/umad.h>

#ifdef USE_MEMORY_TRACING
#undef calloc
#undef free
#define calloc(a, b) 'Error use MPL_calloc' :::
#define free(a)      'Error use MPL_free' :::
#endif

#endif /*defined(HAVE_LIBIBUMAD)*/

#if defined(_MCST_SUPPORT_)

#include <infiniband/mad.h>

#endif /*defined(_MCST_SUPPORT_)*/

#if defined(_SHARP_SUPPORT_)
#include "api/sharp_coll.h"
#endif /* defined(_SHARP_SUPPORT_) */

#ifdef _ENABLE_IBV_DLOPEN_
#include <dlfcn.h>
#endif /*_ENABLE_IBV_DLOPEN_*/
/*
=== BEGIN_MPI_T_MVP_CVAR_INFO_BLOCK ===

cvars:
    - name        : MVP_LIBIBVERBS_PATH
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_LIBIBMAD_PATH
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_LIBIBUMAD_PATH
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_LIBRDMACM_PATH
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

    - name        : MVP_LIBSHARP_PATH
      category    : CH3
      type        : string
      default     : NULL
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : TODO-DESC

=== END_MPI_T_MVP_CVAR_INFO_BLOCK ===
*/
/* Structure to abstract calls to verbs library */
typedef struct _ibv_ops_t {
    /* Functions to manipulate list of devices */
    struct ibv_device **(*get_device_list)(int *num_devices);
    const char *(*get_device_name)(struct ibv_device *device);
    void (*free_device_list)(struct ibv_device **list);
    /* Functions to manipulate device context */
    struct ibv_context *(*open_device)(struct ibv_device *device);
    int (*close_device)(struct ibv_context *context);
    /* Functions to manipulate completion channel */
    struct ibv_comp_channel *(*create_comp_channel)(
        struct ibv_context *context);
    int (*destroy_comp_channel)(struct ibv_comp_channel *channel);
    /* Functions to manipulate PD */
    struct ibv_pd *(*alloc_pd)(struct ibv_context *context);
    int (*dealloc_pd)(struct ibv_pd *pd);
    /* Functions to manipulate CQ */
    struct ibv_cq *(*create_cq)(struct ibv_context *context, int cqe,
                                void *cq_context,
                                struct ibv_comp_channel *channel,
                                int comp_vector);
    int (*resize_cq)(struct ibv_cq *cq, int cqe);
    int (*get_cq_event)(struct ibv_comp_channel *channel, struct ibv_cq **cq,
                        void **cq_context);
    void (*ack_cq_events)(struct ibv_cq *cq, unsigned int nevents);
    int (*destroy_cq)(struct ibv_cq *cq);
    /* Functions to manipulate QP */
    struct ibv_qp *(*create_qp)(struct ibv_pd *pd,
                                struct ibv_qp_init_attr *qp_init_attr);
    int (*query_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask,
                    struct ibv_qp_init_attr *init_attr);
    int (*modify_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr,
                     int attr_mask);
    int (*destroy_qp)(struct ibv_qp *qp);
    /* Functions to manipulate SRQ */
    struct ibv_srq *(*create_srq)(struct ibv_pd *pd,
                                  struct ibv_srq_init_attr *srq_init_attr);
    int (*query_srq)(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr);
    int (*modify_srq)(struct ibv_srq *srq, struct ibv_srq_attr *srq_attr,
                      int srq_attr_mask);
    int (*destroy_srq)(struct ibv_srq *srq);
    /* Functions to manipulate AH */
    struct ibv_ah *(*create_ah)(struct ibv_pd *pd, struct ibv_ah_attr *attr);
    int (*destroy_ah)(struct ibv_ah *ah);
    /* Functions to query hardware */
    int (*query_device)(struct ibv_context *context,
                        struct ibv_device_attr *device_attr);
    int (*query_port)(struct ibv_context *context, uint8_t port_num,
                      struct ibv_port_attr *port_attr);
    int (*query_gid)(struct ibv_context *context, uint8_t port_num, int index,
                     union ibv_gid *gid);
    int (*query_pkey)(struct ibv_context *context, uint8_t port_num, int index,
                      uint16_t *pkey);
    /* Functions to manipulate MR */
    struct ibv_mr *(*reg_mr)(struct ibv_pd *pd, void *addr, size_t length,
                             int access);
    int (*dereg_mr)(struct ibv_mr *mr);
    /* Functions related to send/recv */
    int (*fork_init)(void);
    /* Functions related to mcast */
    int (*attach_mcast)(struct ibv_qp *qp, const union ibv_gid *gid,
                        uint16_t lid);
    int (*detach_mcast)(struct ibv_qp *qp, const union ibv_gid *gid,
                        uint16_t lid);
    /* Functions related to async events */
    int (*get_async_event)(struct ibv_context *context,
                           struct ibv_async_event *event);

    void (*ack_async_event)(struct ibv_async_event *event);
    /* Functions related to debugging events */
    const char *(*event_type_str)(enum ibv_event_type event_type);
    const char *(*node_type_str)(enum ibv_node_type node_type);
    const char *(*port_state_str)(enum ibv_port_state port_state);
    const char *(*wc_status_str)(enum ibv_wc_status status);
    /* XRC related functions */
    struct ibv_xrc_domain *(*open_xrc_domain)(struct ibv_context *context,
                                              int fd, int oflag);
    struct ibv_srq *(*create_xrc_srq)(struct ibv_pd *pd,
                                      struct ibv_xrc_domain *xrc_domain,
                                      struct ibv_cq *xrc_cq,
                                      struct ibv_srq_init_attr *srq_init_attr);
    int (*close_xrc_domain)(struct ibv_xrc_domain *d);
    int (*create_xrc_rcv_qp)(struct ibv_qp_init_attr *init_attr,
                             uint32_t *xrc_rcv_qpn);
    int (*modify_xrc_rcv_qp)(struct ibv_xrc_domain *xrc_domain,
                             uint32_t xrc_qp_num, struct ibv_qp_attr *attr,
                             int attr_mask);
    int (*query_xrc_rcv_qp)(struct ibv_xrc_domain *xrc_domain,
                            uint32_t xrc_qp_num, struct ibv_qp_attr *attr,
                            int attr_mask, struct ibv_qp_init_attr *init_attr);
    int (*reg_xrc_rcv_qp)(struct ibv_xrc_domain *xrc_domain,
                          uint32_t xrc_qp_num);
    int (*unreg_xrc_rcv_qp)(struct ibv_xrc_domain *xrc_domain,
                            uint32_t xrc_qp_num);
} ibv_ops_t;

extern ibv_ops_t ibv_ops;
extern void *ibv_dl_handle;

#if defined(RDMA_CM)
/* Structure to abstract calls to rdma_cm library */
typedef struct _rdma_ops_t {
    struct rdma_event_channel *(*create_event_channel)(void);
    void (*destroy_event_channel)(struct rdma_event_channel *channel);
    int (*create_id)(struct rdma_event_channel *channel, struct rdma_cm_id **id,
                     void *context, enum rdma_port_space ps);
    int (*create_ep)(struct rdma_cm_id **id, struct rdma_addrinfo *res,
                     struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr);
    void (*destroy_ep)(struct rdma_cm_id *id);
    int (*destroy_id)(struct rdma_cm_id *id);
    int (*bind_addr)(struct rdma_cm_id *id, struct sockaddr *addr);
    int (*resolve_addr)(struct rdma_cm_id *id, struct sockaddr *src_addr,
                        struct sockaddr *dst_addr, int timeout_ms);
    int (*resolve_route)(struct rdma_cm_id *id, int timeout_ms);
    int (*create_qp)(struct rdma_cm_id *id, struct ibv_pd *pd,
                     struct ibv_qp_init_attr *qp_init_attr);
    void (*destroy_qp)(struct rdma_cm_id *id);
    int (*connect)(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);
    int (*listen)(struct rdma_cm_id *id, int backlog);
    int (*get_request)(struct rdma_cm_id *listen, struct rdma_cm_id **id);
    int (*accept)(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);
    int (*reject)(struct rdma_cm_id *id, const void *private_data,
                  uint8_t private_data_len);
    int (*notify)(struct rdma_cm_id *id, enum ibv_event_type event);
    int (*disconnect)(struct rdma_cm_id *id);
    int (*join_multicast)(struct rdma_cm_id *id, struct sockaddr *addr,
                          void *context);
    int (*leave_multicast)(struct rdma_cm_id *id, struct sockaddr *addr);
    int (*get_cm_event)(struct rdma_event_channel *channel,
                        struct rdma_cm_event **event);
    int (*ack_cm_event)(struct rdma_cm_event *event);
    int (*getaddrinfo)(char *node, char *service, struct rdma_addrinfo *hints,
                       struct rdma_addrinfo **res);
    int (*freeaddrinfo)(struct rdma_addrinfo *res);
    char *(*event_str)(enum rdma_cm_event_type event);
    struct ibv_context **(*get_devices)(int *num_devices);
    void (*free_devices)(struct ibv_context **list);
} rdma_ops_t;

extern rdma_ops_t rdma_ops;
extern void *rdma_dl_handle;
#endif /*defined(RDMA_CM)*/

#if defined(HAVE_LIBIBUMAD)
/* Structure to abstract calls to umad library */
typedef struct _umad_ops_t {
    int (*init)(void);
    int (*done)(void);
    int (*get_ca)(char *ca_name, umad_ca_t *ca);
    int (*release_ca)(umad_ca_t *ca);
    void *(*get_mad)(void *umad);
    int (*send)(int portid, int agentid, void *umad, int length, int timeout_ms,
                int retries);
    int (*recv)(int portid, void *umad, int *length, int timeout_ms);
    int (*open_port)(char *ca_name, int portnum);
    int (*close_port)(int portid);
    int (*u_register)(int portid, int mgmt_class, int mgmt_version,
                      uint8_t rmpp_version,
                      long method_mask[16 / sizeof(long)]);
    int (*unregister)(int portid, int agentid);
    void *(*alloc)(int num, size_t size);
    void (*u_free)(void *umad);
    size_t (*size)(void);
} umad_ops_t;

extern umad_ops_t umad_ops;
extern void *umad_dl_handle;
#endif /*defined(HAVE_LIBIBUMAD)*/

#if defined(_MCST_SUPPORT_)
/* Structure to abstract calls to mad library */
typedef struct _mad_ops_t {
    uint32_t (*get_field)(void *buf, int base_offs, enum MAD_FIELDS field);
    void (*set_field)(void *buf, int base_offs, enum MAD_FIELDS field,
                      uint32_t val);
    uint64_t (*get_field64)(void *buf, int base_offs, enum MAD_FIELDS field);
    void (*set_field64)(void *buf, int base_offs, enum MAD_FIELDS field,
                        uint64_t val);
    void (*set_array)(void *buf, int base_offs, enum MAD_FIELDS field,
                      void *val);
    void (*get_array)(void *buf, int base_offs, enum MAD_FIELDS field,
                      void *val);
    int (*build_pkt)(void *umad, ib_rpc_t *rpc, ib_portid_t *dport,
                     ib_rmpp_hdr_t *rmpp, void *data);
} mad_ops_t;

extern mad_ops_t mad_ops;
extern void *mad_dl_handle;
#endif /*defined(_MCST_SUPPORT_)*/

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

#ifdef _ENABLE_IBV_DLOPEN_
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

typedef enum dl_err_t {
    SUCCESS_DLOPEN = 0,
    ERROR_DLOPEN = -1,
    ERROR_DLSYM = -2
} dl_err_t;

/* Calls to create abstractions */
static inline int mvp_ibv_dlopen_init()
{
    char *path;
#ifdef _ENABLE_IBV_DLOPEN_
    ibv_dl_handle = dlopen("libibverbs.so", RTLD_NOW);
    if (!ibv_dl_handle) {
        if (MVP_LIBIBVERBS_PATH) {
            ibv_dl_handle = dlopen(MVP_LIBIBVERBS_PATH, RTLD_NOW);
        }
    }
    if (!ibv_dl_handle) {
        fprintf(stderr, "Error opening libibverbs.so: %s.\n", dlerror());
        if (!path) {
            fprintf(stderr, "Please retry with MVP_LIBIBVERBS_PATH="
                            "<path/to/libibverbs.so>\n");
        }
        return ERROR_DLOPEN;
    }
#endif /*_ENABLE_IBV_DLOPEN_*/

    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, get_device_list);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, get_device_name);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, free_device_list);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, open_device);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, close_device);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_comp_channel);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, destroy_comp_channel);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, alloc_pd);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, dealloc_pd);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_cq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, resize_cq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, get_cq_event);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, ack_cq_events);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, destroy_cq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, modify_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, destroy_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_srq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_srq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, modify_srq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, destroy_srq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_ah);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, destroy_ah);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_device);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_port);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_gid);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_pkey);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, reg_mr);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, dereg_mr);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, fork_init);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, attach_mcast);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, detach_mcast);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, get_async_event);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, ack_async_event);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, event_type_str);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, node_type_str);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, port_state_str);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, wc_status_str);
#ifdef _ENABLE_XRC_
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, open_xrc_domain);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_xrc_srq);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, close_xrc_domain);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, create_xrc_rcv_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, modify_xrc_rcv_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, query_xrc_rcv_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, reg_xrc_rcv_qp);
    MVP_DLSYM(ibv_ops, ibv_dl_handle, ibv, unreg_xrc_rcv_qp);
#endif /*_ENABLE_XRC_*/

    return SUCCESS_DLOPEN;
}

#if defined(_MCST_SUPPORT_)
static inline int mvp_mad_dlopen_init()
{
    char *path;
#ifdef _ENABLE_IBV_DLOPEN_
    mad_dl_handle = dlopen("libibmad.so", RTLD_NOW);
    if (!mad_dl_handle) {
        if (MVP_LIBIBMAD_PATH) {
            mad_dl_handle = dlopen(MVP_LIBIBMAD_PATH, RTLD_NOW);
        }
    }
    if (!mad_dl_handle) {
        fprintf(stderr, "Error opening libibmad.so: %s.\n", dlerror());
        if (!path) {
            fprintf(stderr, "Please retry with MVP_LIBIBMAD_PATH="
                            "<path/to/libibmad.so>\n");
        }
        return ERROR_DLOPEN;
    }
#endif /*_ENABLE_IBV_DLOPEN_*/

    MVP_DLSYM(mad_ops, mad_dl_handle, mad, get_field);
    MVP_DLSYM(mad_ops, mad_dl_handle, mad, set_field);
    MVP_DLSYM(mad_ops, mad_dl_handle, mad, get_field64);
    MVP_DLSYM(mad_ops, mad_dl_handle, mad, set_field64);
    MVP_DLSYM(mad_ops, mad_dl_handle, mad, set_array);
    MVP_DLSYM(mad_ops, mad_dl_handle, mad, get_array);
    MVP_DLSYM(mad_ops, mad_dl_handle, mad, build_pkt);

    return SUCCESS_DLOPEN;
}
#endif /*defined(_MCST_SUPPORT_)*/

#if defined(HAVE_LIBIBUMAD)
static inline int mvp_umad_dlopen_init()
{
    char *path;
#ifdef _ENABLE_IBV_DLOPEN_
    umad_dl_handle = dlopen("libibumad.so", RTLD_NOW);
    if (!umad_dl_handle) {
        if (MVP_LIBIBUMAD_PATH) {
            umad_dl_handle = dlopen(MVP_LIBIBUMAD_PATH, RTLD_NOW);
        }
    }
    if (!umad_dl_handle) {
        fprintf(stderr, "Error opening libibumad.so: %s.\n", dlerror());
        if (!path) {
            fprintf(stderr, "Please retry with MVP_LIBIBUMAD_PATH="
                            "<path/to/libibumad.so>\n");
        }
        return ERROR_DLOPEN;
    }
    /* Since "register" is a keyword and "free" is a known function, we cannot
     * use the macro method for them. */
    umad_ops.u_register = dlsym(umad_dl_handle, "umad_register");
    if (umad_ops.u_register == NULL) {
        return ERROR_DLSYM;
    }
    /* static inline */
    umad_ops.alloc = umad_alloc;
    umad_ops.u_free = umad_free;
#else
    umad_ops.u_register = umad_register;
    umad_ops.u_free = umad_free;
#endif /*_ENABLE_IBV_DLOPEN_*/

    MVP_DLSYM(umad_ops, umad_dl_handle, umad, init);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, done);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, get_ca);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, release_ca);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, get_mad);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, send);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, recv);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, open_port);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, close_port);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, unregister);
    MVP_DLSYM(umad_ops, umad_dl_handle, umad, size);

    return SUCCESS_DLOPEN;
}
#endif /*defined(HAVE_LIBIBUMAD)*/

#if defined(RDMA_CM)
static inline int mvp_rdma_dlopen_init()
{
    char *path;
#ifdef _ENABLE_IBV_DLOPEN_
    rdma_dl_handle = dlopen("librdmacm.so", RTLD_NOW);
    if (!rdma_dl_handle) {
        if (MVP_LIBRDMACM_PATH) {
            rdma_dl_handle = dlopen(MVP_LIBRDMACM_PATH, RTLD_NOW);
        }
    }
    if (!rdma_dl_handle) {
        fprintf(stderr, "Error opening librdmacm.so: %s.\n", dlerror());
        if (!path) {
            fprintf(stderr, "Please retry with MVP_LIBRDMACM_PATH="
                            "<path/to/librdmacm.so>\n");
        }
        return ERROR_DLOPEN;
    }
#endif /*_ENABLE_IBV_DLOPEN_*/

    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, create_event_channel);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, destroy_event_channel);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, create_id);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, create_ep);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, destroy_ep);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, destroy_id);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, bind_addr);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, resolve_addr);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, resolve_route);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, create_qp);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, destroy_qp);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, connect);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, listen);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, get_request);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, accept);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, reject);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, notify);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, disconnect);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, join_multicast);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, leave_multicast);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, get_cm_event);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, ack_cm_event);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, getaddrinfo);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, freeaddrinfo);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, event_str);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, get_devices);
    MVP_DLSYM(rdma_ops, rdma_dl_handle, rdma, free_devices);

    return SUCCESS_DLOPEN;
}
#endif /*defined(RDMA_CM)*/

#if defined(_SHARP_SUPPORT_)
static inline int mvp_sharp_dlopen_init()
{
    char *path;
#ifdef _ENABLE_IBV_DLOPEN_
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
#endif /*_ENABLE_IBV_DLOPEN_*/

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

    err = mvp_ibv_dlopen_init();
    if (err != SUCCESS_DLOPEN) {
        fprintf(stderr, "mvp_ibv_dlopen_init returned %d\n", err);
    }
#if defined(_MCST_SUPPORT_)
    err = mvp_mad_dlopen_init();
    if (err != SUCCESS_DLOPEN) {
        fprintf(stderr, "mvp_mad_dlopen_init returned %d\n", err);
    }
#endif /*defined(_MCST_SUPPORT_)*/
#if defined(HAVE_LIBIBUMAD)
    err = mvp_umad_dlopen_init();
    if (err != SUCCESS_DLOPEN) {
        fprintf(stderr, "mvp_umad_dlopen_init returned %d\n", err);
    }
#endif /*defined(HAVE_LIBIBUMAD)*/
#if defined(RDMA_CM)
    err = mvp_rdma_dlopen_init();
    if (err != SUCCESS_DLOPEN) {
        fprintf(stderr, "mvp_rdma_dlopen_init returned %d\n", err);
    }
#endif /*defined(RDMA_CM)*/

#if defined(_SHARP_SUPPORT_)
    char *value;
    if (MVP_ENABLE_SHARP) {
        err = mvp_sharp_dlopen_init();
        if (err != SUCCESS_DLOPEN) {
            fprintf(stderr, "mvp_sharp_dlopen_init returned %d\n", err);
        }
    }
#endif /*defined(_SHARP_SUPPORT_)*/

    return err;
}

/* Calls to finalize abstractions */
static inline void mvp_ibv_dlopen_finalize()
{
#ifdef _ENABLE_IBV_DLOPEN_
    if (ibv_dl_handle) {
        dlclose(ibv_dl_handle);
    }
#endif /*_ENABLE_IBV_DLOPEN_*/
}

#if defined(_MCST_SUPPORT_)
static inline void mvp_mad_dlopen_finalize()
{
#ifdef _ENABLE_IBV_DLOPEN_
    if (mad_dl_handle) {
        dlclose(mad_dl_handle);
    }
#endif /*_ENABLE_IBV_DLOPEN_*/
}
#endif /*defined(_MCST_SUPPORT_)*/

#if defined(HAVE_LIBIBUMAD)
static inline void mvp_umad_dlopen_finalize()
{
#ifdef _ENABLE_IBV_DLOPEN_
    if (umad_dl_handle) {
        dlclose(umad_dl_handle);
    }
#endif /*_ENABLE_IBV_DLOPEN_*/
}
#endif /*defined(HAVE_LIBIBUMAD)*/

#if defined(RDMA_CM)
static inline void mvp_rdma_dlopen_finalize()
{
#ifdef _ENABLE_IBV_DLOPEN_
    if (rdma_dl_handle) {
        dlclose(rdma_dl_handle);
    }
#endif /*_ENABLE_IBV_DLOPEN_*/
}
#endif /*defined(RDMA_CM)*/

#if defined(_SHARP_SUPPORT_)
static inline void mvp_sharp_dlopen_finalize()
{
#ifdef _ENABLE_IBV_DLOPEN_
    if (sharp_dl_handle) {
        dlclose(sharp_dl_handle);
    }
#endif /*_ENABLE_IBV_DLOPEN_*/
}
#endif /*defined(_SHARP_SUPPORT)*/

static inline int mvp_dlopen_finalize()
{
    mvp_ibv_dlopen_finalize();
#if defined(_MCST_SUPPORT_)
    mvp_mad_dlopen_finalize();
#endif /*defined(_MCST_SUPPORT_)*/
#if defined(HAVE_LIBIBUMAD)
    mvp_umad_dlopen_finalize();
#endif /*defined(HAVE_LIBIBUMAD)*/
#if defined(RDMA_CM)
    mvp_rdma_dlopen_finalize();
#endif /*defined(RDMA_CM)*/
#if defined(_SHARP_SUPPORT_)
    mvp_sharp_dlopen_finalize();
#endif /*defined(_SHARP_SUPPORT_)*/

    return SUCCESS_DLOPEN;
}
#endif /*OFED_ABSTRACTION_H*/

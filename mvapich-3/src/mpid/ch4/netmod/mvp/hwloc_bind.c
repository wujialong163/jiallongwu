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
#include "mpichconf.h"
#include "mpidimpl.h"
#include <mpir_mem.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "upmi.h"
#include "hwloc_bind.h"
#if defined(HAVE_LIBIBVERBS)
#include <hwloc/autogen/config.h>
#endif
#include "mvp_arch_hca_detect.h"
#include "mvp_debug_utils.h"
#include <hwloc/linux.h>

/* CPU Mapping related definitions */

#define CONFIG_FILE     "/proc/cpuinfo"
#define MAX_LINE_LENGTH 512
#define MAX_NAME_LENGTH 64
#define HOSTNAME_LENGTH 255
#define FILENAME_LENGTH 512

extern int mvp_ib_hca_socket_info[];
extern int mvp_ib_hca_numa_info[];
extern int mvp_selected_ib_hca_socket_info[];
extern int mvp_selected_ib_hca_numa_info[];

const char *mvp_cpu_policy_names[] = {"Bunch", "Scatter", "Hybrid"};
const char *mvp_hybrid_policy_names[] = {
    "Linear", "Compact", "Spread", "Bunch", "Scatter", "NUMA",
    "Linear", "Compact", "Spread", "Bunch", "Scatter", "NUMA"};

int num_sockets = 1; /* default */
int num_physical_cores = 0;
int num_pu = 0;
int hw_threads_per_core = 0;
int *mvp_core_map; /* list of core ids achieved after hwloc tree scanning */
int *mvp_core_map_per_numa; /* list of core ids based on NUMA nodes */

int mvp_my_cpu_id = -1;
int mvp_my_sock_id = -1;
int mvp_my_numa_id = -1;
int mvp_my_l3_id = -1;
int mvp_num_intra_node_comm_levels = 0;
int mvp_intra_node_cluster_at_level[5] = {0};
int mvp_my_async_cpu_id = -1;
int *local_core_ids = NULL;
int mvp_user_defined_mapping = FALSE;

#ifdef ENABLE_LLNL_SITE_SPECIFIC_OPTIONS
unsigned int mvp_enable_affinity = 0;
#else
unsigned int mvp_enable_affinity = 1;
#endif /*ENABLE_LLNL_SITE_SPECIFIC_OPTIONS*/

typedef enum {
    CPU_FAMILY_NONE = 0,
    CPU_FAMILY_INTEL,
    CPU_FAMILY_AMD,
} cpu_type_t;

int CLOVERTOWN_MODEL = 15;
int HARPERTOWN_MODEL = 23;
int NEHALEM_MODEL = 26;

int ip = 0;
unsigned long *core_mapping = NULL;
int *obj_tree = NULL;

hwloc_topology_t topology = NULL;
hwloc_topology_t topology_whole = NULL;
level_type_t mvp_binding_level;

static int INTEL_XEON_DUAL_MAPPING[] = {0, 1, 0, 1};

/* ((0,1),(4,5))((2,3),(6,7)) */
static int INTEL_CLOVERTOWN_MAPPING[] = {0, 0, 1, 1, 0, 0, 1, 1};

/* legacy ((0,2),(4,6))((1,3),(5,7)) */
static int INTEL_HARPERTOWN_LEG_MAPPING[] = {0, 1, 0, 1, 0, 1, 0, 1};

/* common ((0,1),(2,3))((4,5),(6,7)) */
static int INTEL_HARPERTOWN_COM_MAPPING[] = {0, 0, 0, 0, 1, 1, 1, 1};

/* legacy (0,2,4,6)(1,3,5,7) with hyperthreading */
static int INTEL_NEHALEM_LEG_MAPPING[] = {0, 1, 0, 1, 0, 1, 0, 1,
                                          0, 1, 0, 1, 0, 1, 0, 1};

/* common (0,1,2,3)(4,5,6,7) with hyperthreading */
static int INTEL_NEHALEM_COM_MAPPING[] = {0, 0, 0, 0, 1, 1, 1, 1,
                                          0, 0, 0, 0, 1, 1, 1, 1};

static int AMD_OPTERON_DUAL_MAPPING[] = {0, 0, 1, 1};
static int AMD_BARCELONA_MAPPING[] = {0, 0, 0, 0, 1, 1, 1, 1,
                                      2, 2, 2, 2, 3, 3, 3, 3};

extern int use_hwloc_cpu_binding;

char *s_cpu_mapping = NULL;
static char *custom_cpu_mapping = NULL;
int s_cpu_mapping_line_max = _POSIX2_LINE_MAX;
static int custom_cpu_mapping_line_max = _POSIX2_LINE_MAX;
char *cpu_mapping = NULL;
char *xmlpath = NULL;
char *whole_topology_xml_path = NULL;
int ib_socket_bind = 0;

#if defined(HAVE_LIBIBVERBS)
/* hwloc/openfabrics-verbs.h defines this function as a static inline. This
 * causes MVAPICH to depend on libibverbs.so. Ideally, we should request hwloc
 * folks fix this issue so that we do not inherit the dependency. In
 * the meantime, we are going to copy paste the function here and use the
 * ibv_ops abstraction to call ibv_get_device_name. We should revisit this later
 */
static inline int hwloc_ibv_get_device_cpuset(
    hwloc_topology_t topology __hwloc_attribute_unused,
    struct ibv_device *ibdev, hwloc_cpuset_t set)
{
#ifdef HWLOC_LINUX_SYS
    /* If we're on Linux, use the verbs-provided sysfs mechanism to
     *      get the local cpus */
#define HWLOC_OPENFABRICS_VERBS_SYSFS_PATH_MAX 128
    char path[HWLOC_OPENFABRICS_VERBS_SYSFS_PATH_MAX];
#ifdef _USE_HWLOC_V1_
    FILE *sysfile = NULL;
#endif /* _USE_HWLOC_V1_ */

    if (!hwloc_topology_is_thissystem(topology)) {
        errno = EINVAL;
        return -1;
    }

    sprintf(path, "/sys/class/infiniband/%s/device/local_cpus",
#if CHANNEL_MRAIL
            ibv_ops.get_device_name(ibdev)
#elif CHANNEL_PSM
            ibv_get_device_name(ibdev)
#endif
    );
#ifdef _USE_HWLOC_V1_
    sysfile = fopen(path, "r");
    if (!sysfile)
        return -1;
#endif /* _USE_HWLOC_V1_ */

    if (
#ifdef _USE_HWLOC_V1_
        hwloc_linux_parse_cpumap_file(sysfile, set) < 0
#else
        hwloc_linux_read_path_as_cpumask(path, set)
#endif
        || hwloc_bitmap_iszero(set))
        hwloc_bitmap_copy(set, hwloc_topology_get_complete_cpuset(topology));

#ifdef _USE_HWLOC_V1_
    fclose(sysfile);
#endif /* _USE_HWLOC_V1_ */
#else
    /* Non-Linux systems simply get a full cpuset */
    hwloc_bitmap_copy(set, hwloc_topology_get_complete_cpuset(topology));
#endif
    return 0;
}
#endif

#if defined(CHANNEL_MRAIL)
int get_ib_socket(struct ibv_device *ibdev)
{
    hwloc_obj_t socket;
    int i = 0, retval = 0, num_sockets = 0;
    hwloc_cpuset_t set = NULL, set2 = NULL, set3 = NULL;

    if (smpi_load_hwloc_topology_whole()) {
        return MPI_ERR_INTERN;
    }

    if (!(set = hwloc_bitmap_alloc())) {
        goto fn_exit;
    }

    if (!(set2 = hwloc_bitmap_alloc())) {
        goto fn_exit;
    }

    if (hwloc_ibv_get_device_cpuset(topology_whole, ibdev, set)) {
        goto fn_exit;
    }

    num_sockets = hwloc_get_nbobjs_by_type(topology_whole, HWLOC_OBJ_SOCKET);
    for (i = 0; i < num_sockets; i++) {
        socket = hwloc_get_obj_by_type(topology_whole, HWLOC_OBJ_SOCKET, i);
        set3 = socket->cpuset;
        hwloc_bitmap_zero(set2);
        hwloc_bitmap_and(set2, set, set3);
        if (hwloc_bitmap_weight(set2)) {
            PRINT_DEBUG(DEBUG_INIT_verbose,
                        "Socket %d (%d) matches for IB %s\n",
                        socket->logical_index, i, ibdev->name);
            retval = socket->logical_index;
            break;
        }
    }

fn_exit:
    if (set) {
        hwloc_bitmap_free(set);
    }
    if (set2) {
        hwloc_bitmap_free(set2);
    }
    return retval;
}

int get_ib_numa(struct ibv_device *ibdev)
{
    hwloc_obj_t numa;
    int i = 0, retval = 0, num_numas = 0;
    hwloc_cpuset_t set = NULL, set2 = NULL, set3 = NULL;

    if (smpi_load_hwloc_topology_whole()) {
        return MPI_ERR_INTERN;
    }

    if (!(set = hwloc_bitmap_alloc())) {
        goto fn_exit;
    }

    if (!(set2 = hwloc_bitmap_alloc())) {
        goto fn_exit;
    }

    if (hwloc_ibv_get_device_cpuset(topology_whole, ibdev, set)) {
        goto fn_exit;
    }

    num_numas = hwloc_get_nbobjs_by_type(topology_whole, HWLOC_OBJ_NUMANODE);
    for (i = 0; i < num_numas; i++) {
        numa = hwloc_get_obj_by_type(topology_whole, HWLOC_OBJ_NUMANODE, i);
        set3 = numa->cpuset;
        hwloc_bitmap_zero(set2);
        hwloc_bitmap_and(set2, set, set3);
        if (hwloc_bitmap_weight(set2)) {
            PRINT_DEBUG(DEBUG_INIT_verbose, "NUMA %d (%d) matches for IB %s\n",
                        numa->logical_index, i, ibdev->name);
            retval = numa->logical_index;
            break;
        }
    }

fn_exit:
    if (set) {
        hwloc_bitmap_free(set);
    }
    if (set2) {
        hwloc_bitmap_free(set2);
    }
    return retval;
}
#endif /* defined(CHANNEL_MRAIL) */

static int first_num_from_str(char **str)
{
    int val = atoi(*str);
    while (isdigit(**str)) {
        (*str)++;
    }
    return val;
}

static inline int compare_float(const float a, const float b)
{
    const float precision = 0.00001;
    if ((a - precision) < b && (a + precision) > b) {
        return 1;
    } else {
        return 0;
    }
}

static int pid_filter(const struct dirent *dir_obj)
{
    int i;
    int length = strlen(dir_obj->d_name);

    for (i = 0; i < length; i++) {
        if (!isdigit(dir_obj->d_name[i])) {
            return 0;
        }
    }
    return 1;
}

static void find_parent(hwloc_obj_t obj, hwloc_obj_type_t type,
                        hwloc_obj_t *parent)
{
    if ((type == HWLOC_OBJ_CORE) || (type == HWLOC_OBJ_SOCKET) ||
        (type == HWLOC_OBJ_NODE)) {
        if (obj->parent->type == type) {
            *parent = obj->parent;
            return;
        } else {
            find_parent(obj->parent, type, parent);
        }
    } else {
        return;
    }
}

static void find_leastload_node(obj_attribute_type *tree, hwloc_obj_t original,
                                hwloc_obj_t *result)
{
    int i, j, k, per, ix, depth_nodes, num_nodes, depth_sockets, num_sockets;
    hwloc_obj_t obj, tmp;

    depth_nodes = hwloc_get_type_depth(topology, HWLOC_OBJ_NODE);
    num_nodes = hwloc_get_nbobjs_by_depth(topology, depth_nodes);

    /* One socket includes multi numanodes. */
    if (original->type == HWLOC_OBJ_SOCKET) {
        depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
        num_sockets = hwloc_get_nbobjs_by_depth(topology, depth_sockets);
        per = num_nodes / num_sockets;
        ix = (original->logical_index) * per;
        if (per == 1) {
            *result = tree[depth_nodes * num_nodes + ix].obj;
        } else {
            i = depth_nodes * num_nodes + ix;
            for (k = 0; k < (per - 1); k++) {
                j = i + k + 1;
                i = (tree[i].load > tree[j].load) ? j : i;
            }
            *result = tree[i].obj;
        }
    } else if (original->type == HWLOC_OBJ_MACHINE) {
        tmp = NULL;
        for (k = 0; k < num_nodes; k++) {
            obj = hwloc_get_obj_by_depth(topology, depth_nodes, k);
            if (tmp == NULL) {
                tmp = obj;
            } else {
                i = depth_nodes * num_nodes + tmp->logical_index;
                j = depth_nodes * num_nodes + obj->logical_index;
                if (tree[i].load > tree[j].load)
                    tmp = obj;
            }
        }
        *result = tmp;
    } else {
        *result = NULL;
    }
    return;
}

static void find_leastload_socket(obj_attribute_type *tree,
                                  hwloc_obj_t original, hwloc_obj_t *result)
{
    int i, j, k, per, ix, depth_sockets, num_sockets, depth_nodes, num_nodes;
    hwloc_obj_t obj, tmp;

    depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    num_sockets = hwloc_get_nbobjs_by_depth(topology, depth_sockets);

    /* One numanode includes multi sockets. */
    if (original->type == HWLOC_OBJ_NODE) {
        depth_nodes = hwloc_get_type_depth(topology, HWLOC_OBJ_NODE);
        num_nodes = hwloc_get_nbobjs_by_depth(topology, depth_nodes);
        per = num_sockets / num_nodes;
        ix = (original->logical_index) * per;
        if (per == 1) {
            *result = tree[depth_sockets * num_sockets + ix].obj;
        } else {
            i = depth_sockets * num_sockets + ix;
            for (k = 0; k < (per - 1); k++) {
                j = i + k + 1;
                i = (tree[i].load > tree[j].load) ? j : i;
            }
            *result = tree[i].obj;
        }
    } else if (original->type == HWLOC_OBJ_MACHINE) {
        tmp = NULL;
        for (k = 0; k < num_sockets; k++) {
            obj = hwloc_get_obj_by_depth(topology, depth_sockets, k);
            if (tmp == NULL) {
                tmp = obj;
            } else {
                i = depth_sockets * num_sockets + tmp->logical_index;
                j = depth_sockets * num_sockets + obj->logical_index;
                if (tree[i].load > tree[j].load)
                    tmp = obj;
            }
        }
        *result = tmp;
    } else {
        *result = NULL;
    }
    return;
}

static void find_leastload_core(obj_attribute_type *tree, hwloc_obj_t original,
                                hwloc_obj_t *result)
{
    int i, j, k, per, ix;
    int depth_cores, num_cores, depth_sockets, num_sockets, depth_nodes,
        num_nodes;

    depth_cores = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
    num_cores = hwloc_get_nbobjs_by_depth(topology, depth_cores);

    /* Core may have Socket or Numanode as direct parent. */
    if (original->type == HWLOC_OBJ_NODE) {
        depth_nodes = hwloc_get_type_depth(topology, HWLOC_OBJ_NODE);
        num_nodes = hwloc_get_nbobjs_by_depth(topology, depth_nodes);
        per = num_cores / num_nodes;
        ix = (original->logical_index) * per;
        if (per == 1) {
            *result = tree[depth_cores * num_cores + ix].obj;
        } else {
            i = depth_cores * num_cores + ix;
            for (k = 0; k < (per - 1); k++) {
                j = i + k + 1;
                i = (tree[i].load > tree[j].load) ? j : i;
            }
            *result = tree[i].obj;
        }
    } else if (original->type == HWLOC_OBJ_SOCKET) {
        depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
        num_sockets = hwloc_get_nbobjs_by_depth(topology, depth_sockets);
        per = num_cores / num_sockets;
        ix = (original->logical_index) * per;
        if (per == 1) {
            *result = tree[depth_cores * num_cores + ix].obj;
        } else {
            i = depth_cores * num_cores + ix;
            for (k = 0; k < (per - 1); k++) {
                j = i + k + 1;
                i = (tree[i].load > tree[j].load) ? j : i;
            }
            *result = tree[i].obj;
        }
    } else {
        *result = NULL;
    }
    return;
}

static void find_leastload_pu(obj_attribute_type *tree, hwloc_obj_t original,
                              hwloc_obj_t *result)
{
    int i, j, k, per, ix, depth_pus, num_pus, depth_cores, num_cores;

    depth_pus = hwloc_get_type_depth(topology, HWLOC_OBJ_PU);
    num_pus = hwloc_get_nbobjs_by_depth(topology, depth_pus);

    /* Assume: pu only has core as direct parent. */
    if (original->type == HWLOC_OBJ_CORE) {
        depth_cores = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
        num_cores = hwloc_get_nbobjs_by_depth(topology, depth_cores);
        per = num_pus / num_cores;
        ix = (original->logical_index) * per;
        if (per == 1) {
            *result = tree[depth_pus * num_pus + ix].obj;
        } else {
            i = depth_pus * num_pus + ix;
            for (k = 0; k < (per - 1); k++) {
                j = i + k + 1;
                i = (tree[i].load > tree[j].load) ? j : i;
            }
            *result = tree[i].obj;
        }
    } else {
        *result = NULL;
    }
    return;
}

static void update_obj_attribute(obj_attribute_type *tree, int ix,
                                 hwloc_obj_t obj, int cpuset, float load)
{
    tree[ix].obj = obj;
    if (!(cpuset < 0)) {
        CPU_SET(cpuset, &(tree[ix].cpuset));
    }
    tree[ix].load += load;
}

static void insert_load(obj_attribute_type *tree, hwloc_obj_t pu, int cpuset,
                        float load)
{
    int k, depth_pus, num_pus = 0;
    int depth_cores, depth_sockets, depth_nodes, num_cores = 0, num_sockets = 0,
                                                 num_nodes = 0;
    hwloc_obj_t parent;

    depth_pus = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_PU);
    num_pus = hwloc_get_nbobjs_by_depth(topology, depth_pus);

    depth_nodes = hwloc_get_type_depth(topology, HWLOC_OBJ_NODE);
    if (depth_nodes != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_nodes = hwloc_get_nbobjs_by_depth(topology, depth_nodes);
    }
    depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    if (depth_sockets != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_sockets = hwloc_get_nbobjs_by_depth(topology, depth_sockets);
    }
    depth_cores = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
    if (depth_cores != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_cores = hwloc_get_nbobjs_by_depth(topology, depth_cores);
    }

    /* Add obj, cpuset and load for HWLOC_OBJ_PU */
    k = depth_pus * num_pus + pu->logical_index;
    update_obj_attribute(tree, k, pu, cpuset, load);
    /* Add cpuset and load for HWLOC_OBJ_CORE */
    if (depth_cores != HWLOC_TYPE_DEPTH_UNKNOWN) {
        find_parent(pu, HWLOC_OBJ_CORE, &parent);
        k = depth_cores * num_cores + parent->logical_index;
        update_obj_attribute(tree, k, parent, cpuset, load);
    }
    /* Add cpuset and load for HWLOC_OBJ_SOCKET */
    if (depth_sockets != HWLOC_TYPE_DEPTH_UNKNOWN) {
        find_parent(pu, HWLOC_OBJ_SOCKET, &parent);
        k = depth_sockets * num_sockets + parent->logical_index;
        update_obj_attribute(tree, k, parent, cpuset, load);
    }
    /* Add cpuset and load for HWLOC_OBJ_NODE */
    if (depth_nodes != HWLOC_TYPE_DEPTH_UNKNOWN) {
        find_parent(pu, HWLOC_OBJ_NODE, &parent);
        k = depth_nodes * num_nodes + parent->logical_index;
        update_obj_attribute(tree, k, parent, cpuset, load);
    }
    return;
}

static void cac_load(obj_attribute_type *tree, cpu_set_t cpuset)
{
    int i, j, depth_pus, num_pus;
    float proc_load;
    int num_processes = 0;
    hwloc_obj_t obj;

    depth_pus = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_PU);
    num_pus = hwloc_get_nbobjs_by_depth(topology, depth_pus);

    for (i = 0; i < num_pus; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            num_processes++;
        }
    }

    /* Process is running on num_processes cores; for each core, the load is
     * proc_load. */
    proc_load = 1 / num_processes;

    /*
     * num_objs is HWLOC_OBJ_PU number, and system CPU number;
     * also HWLOC_OBJ_CORE number when HT disabled or without HT.
     */

    for (i = 0; i < num_pus; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            for (j = 0; j < num_pus; j++) {
                obj = hwloc_get_obj_by_depth(topology, depth_pus, j);
                if (obj->os_index == i) {
                    insert_load(tree, obj, i, proc_load);
                }
            }
        }
    }
    return;
}

static void insert_core_mapping(int ix, hwloc_obj_t pu,
                                obj_attribute_type *tree)
{
    core_mapping[ix] = pu->os_index;
    /* This process will be binding to one pu/core.
     * The load for this pu/core is 1; and not update cpuset.
     */
    insert_load(tree, pu, -1, 1);
    return;
}

void map_scatter_load(obj_attribute_type *tree)
{
    int k;
    int depth_cores, depth_sockets, depth_nodes, num_cores = 0;
    hwloc_obj_t root, node, sockets, core_parent, core, result;

    root = hwloc_get_root_obj(topology);

    depth_nodes = hwloc_get_type_depth(topology, HWLOC_OBJ_NODE);

    depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);

    depth_cores = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
    if (depth_cores != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_cores = hwloc_get_nbobjs_by_depth(topology, depth_cores);
    }

    k = 0;
    /*Assume: there is always existing SOCKET, but not always existing
     * NUMANODE(like Clovertown). */
    while (k < num_cores) {
        if (depth_nodes == HWLOC_TYPE_DEPTH_UNKNOWN) {
            find_leastload_socket(tree, root, &result);
        } else {
            if ((depth_nodes) < (depth_sockets)) {
                find_leastload_node(tree, root, &result);
                node = result;
                find_leastload_socket(tree, node, &result);
            } else {
                find_leastload_socket(tree, root, &result);
                sockets = result;
                find_leastload_node(tree, sockets, &result);
            }
        }
        core_parent = result;
        find_leastload_core(tree, core_parent, &result);
        core = result;
        find_leastload_pu(tree, core, &result);
        insert_core_mapping(k, result, tree);
        k++;
    }
}

void map_bunch_load(obj_attribute_type *tree)
{
    int i, j, k, per = 0;
    int per_socket_node, depth_pus, num_pus = 0;
    float current_socketornode_load = 0, current_core_load = 0;
    int depth_cores, depth_sockets, depth_nodes, num_cores = 0, num_sockets = 0,
                                                 num_nodes = 0;
    hwloc_obj_t root, node, sockets, core_parent, core, pu, result;

    root = hwloc_get_root_obj(topology);

    depth_nodes = hwloc_get_type_depth(topology, HWLOC_OBJ_NODE);
    if (depth_nodes != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_nodes = hwloc_get_nbobjs_by_depth(topology, depth_nodes);
    }

    depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    if (depth_sockets != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_sockets = hwloc_get_nbobjs_by_depth(topology, depth_sockets);
    }

    depth_cores = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
    if (depth_cores != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_cores = hwloc_get_nbobjs_by_depth(topology, depth_cores);
    }

    depth_pus = hwloc_get_type_depth(topology, HWLOC_OBJ_PU);
    if (depth_pus != HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_pus = hwloc_get_nbobjs_by_depth(topology, depth_pus);
    }

    k = 0;
    /*Assume: there is always existing SOCKET, but not always existing
     * NUMANODE(like Clovertown). */
    while (k < num_cores) {
        if (depth_nodes == HWLOC_TYPE_DEPTH_UNKNOWN) {
            find_leastload_socket(tree, root, &result);
            core_parent = result;
            per = num_cores / num_sockets;
            for (i = 0; (i < per) && (k < num_cores); i++) {
                find_leastload_core(tree, core_parent, &result);
                core = result;
                find_leastload_pu(tree, core, &result);
                pu = result;
                if (i == 0) {
                    current_core_load =
                        tree[depth_pus * num_pus + pu->logical_index].load;
                    insert_core_mapping(k, pu, tree);
                    k++;
                } else {
                    if (compare_float(
                            tree[depth_pus * num_pus + pu->logical_index].load,
                            current_core_load)) {
                        insert_core_mapping(k, pu, tree);
                        k++;
                    }
                }
            }
        } else {
            if ((depth_nodes) < (depth_sockets)) {
                find_leastload_node(tree, root, &result);
                node = result;
                per_socket_node = num_sockets / num_nodes;
                for (j = 0; (j < per_socket_node) && (k < num_cores); j++) {
                    find_leastload_socket(tree, node, &result);
                    sockets = result;
                    if (j == 0) {
                        current_socketornode_load =
                            tree[depth_sockets * num_sockets +
                                 sockets->logical_index]
                                .load;
                        per = num_cores / num_sockets;
                        for (i = 0; (i < per) && (k < num_cores); i++) {
                            find_leastload_core(tree, sockets, &result);
                            core = result;
                            find_leastload_pu(tree, core, &result);
                            pu = result;
                            if (i == 0) {
                                current_core_load = tree[depth_pus * num_pus +
                                                         pu->logical_index]
                                                        .load;
                                insert_core_mapping(k, pu, tree);
                                k++;
                            } else {
                                if (compare_float(tree[depth_pus * num_pus +
                                                       pu->logical_index]
                                                      .load,
                                                  current_core_load)) {
                                    insert_core_mapping(k, pu, tree);
                                    k++;
                                }
                            }
                        }
                    } else {
                        if (compare_float(tree[depth_sockets * num_sockets +
                                               sockets->logical_index]
                                              .load,
                                          current_socketornode_load)) {
                            for (i = 0; (i < per) && (k < num_cores); i++) {
                                find_leastload_core(tree, sockets, &result);
                                core = result;
                                find_leastload_pu(tree, core, &result);
                                pu = result;
                                if (i == 0) {
                                    current_core_load =
                                        tree[depth_pus * num_pus +
                                             pu->logical_index]
                                            .load;
                                    insert_core_mapping(k, pu, tree);
                                    k++;
                                } else {
                                    if (compare_float(tree[depth_pus * num_pus +
                                                           pu->logical_index]
                                                          .load,
                                                      current_core_load)) {
                                        insert_core_mapping(k, pu, tree);
                                        k++;
                                    }
                                }
                            }
                        }
                    }
                }
            } else { // depth_nodes > depth_sockets
                find_leastload_socket(tree, root, &result);
                sockets = result;
                per_socket_node = num_nodes / num_sockets;
                for (j = 0; (j < per_socket_node) && (k < num_cores); j++) {
                    find_leastload_node(tree, sockets, &result);
                    node = result;
                    if (j == 0) {
                        current_socketornode_load =
                            tree[depth_nodes * num_nodes + node->logical_index]
                                .load;
                        per = num_cores / num_sockets;
                        for (i = 0; (i < per) && (k < num_cores); i++) {
                            find_leastload_core(tree, node, &result);
                            core = result;
                            find_leastload_pu(tree, core, &result);
                            pu = result;
                            if (i == 0) {
                                current_core_load = tree[depth_pus * num_pus +
                                                         pu->logical_index]
                                                        .load;
                                insert_core_mapping(k, pu, tree);
                                k++;
                            } else {
                                if (compare_float(tree[depth_pus * num_pus +
                                                       pu->logical_index]
                                                      .load,
                                                  current_core_load)) {
                                    insert_core_mapping(k, pu, tree);
                                    k++;
                                }
                            }
                        }
                    } else {
                        if (compare_float(tree[depth_nodes * num_nodes +
                                               node->logical_index]
                                              .load,
                                          current_socketornode_load)) {
                            for (i = 0; (i < per) && (k < num_cores); i++) {
                                find_leastload_core(tree, node, &result);
                                core = result;
                                find_leastload_pu(tree, core, &result);
                                pu = result;
                                if (i == 0) {
                                    current_core_load =
                                        tree[depth_pus * num_pus +
                                             pu->logical_index]
                                            .load;
                                    insert_core_mapping(k, pu, tree);
                                    k++;
                                } else {
                                    if (compare_float(tree[depth_pus * num_pus +
                                                           pu->logical_index]
                                                          .load,
                                                      current_core_load)) {
                                        insert_core_mapping(k, pu, tree);
                                        k++;
                                    }
                                }
                            }
                        }
                    }
                }
            } /* depth_nodes > depth_sockets */
        }
    } /* while */
}

/*
 * Compare two hwloc_obj_t of type HWLOC_OBJ_PU according to sibling_rank, used
 * with qsort
 */
static int cmpproc_smt(const void *a, const void *b)
{
    hwloc_obj_t pa = *(hwloc_obj_t *)a;
    hwloc_obj_t pb = *(hwloc_obj_t *)b;
    return (pa->sibling_rank == pb->sibling_rank) ?
               pa->os_index - pb->os_index :
               pa->sibling_rank - pb->sibling_rank;
}

static int cmpdepth_smt(const void *a, const void *b)
{
    ancestor_type pa = *(ancestor_type *)a;
    ancestor_type pb = *(ancestor_type *)b;
    if ((pa.ancestor)->depth > (pb.ancestor)->depth) {
        return -1;
    } else if ((pa.ancestor)->depth < (pb.ancestor)->depth) {
        return 1;
    } else {
        return 0;
    }
}

static int cmparity_smt(const void *a, const void *b)
{
    ancestor_type pa = *(ancestor_type *)a;
    ancestor_type pb = *(ancestor_type *)b;
    if ((pa.ancestor)->arity > (pb.ancestor)->arity) {
        return -1;
    } else if ((pa.ancestor)->arity < (pb.ancestor)->arity) {
        return 1;
    } else {
        return 0;
    }
}

static void get_first_obj_bunch(hwloc_obj_t *result)
{
    hwloc_obj_t *objs;
    ancestor_type *array;
    int i, j, k, num_objs, num_ancestors;

    if ((num_objs = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU)) <= 0) {
        return;
    }

    if ((objs = (hwloc_obj_t *)MPL_malloc(num_objs * sizeof(hwloc_obj_t),
                                          MPL_MEM_OTHER)) == NULL) {
        return;
    }

    for (i = 0; i < num_objs; i++) {
        objs[i] = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, i);
    }

    num_ancestors = num_objs * (num_objs - 1) / 2;

    if ((array = (ancestor_type *)MPL_malloc(
             num_ancestors * sizeof(ancestor_type), MPL_MEM_OTHER)) == NULL) {
        return;
    }

    k = 0;
    for (i = 0; i < (num_objs - 1); i++) {
        for (j = i + 1; j < num_objs; j++) {
            array[k].obja = objs[i];
            array[k].objb = objs[j];
            array[k].ancestor =
                hwloc_get_common_ancestor_obj(topology, objs[i], objs[j]);
            k++;
        }
    }

    qsort(array, num_ancestors, sizeof(ancestor_type), cmpdepth_smt);

    for (i = 0; i < (num_ancestors - 1); i++) {
        if ((array[i + 1].ancestor)->depth < (array[i].ancestor)->depth) {
            break;
        }
    }

    qsort(array, (i + 1), sizeof(ancestor_type), cmparity_smt);

    *result = array[0].obja;

    MPL_free(objs);
    MPL_free(array);
    return;
}

static void get_first_socket_bunch(hwloc_obj_t *result,
                                   hwloc_obj_type_t binding_level)
{
    hwloc_obj_t *objs;
    ancestor_type *array;
    int i, j, k, num_objs, num_ancestors;

    if ((num_objs = hwloc_get_nbobjs_by_type(topology, binding_level)) <= 0) {
        return;
    }

    if ((objs = (hwloc_obj_t *)MPL_malloc(num_objs * sizeof(hwloc_obj_t),
                                          MPL_MEM_OTHER)) == NULL) {
        return;
    }

    for (i = 0; i < num_objs; i++) {
        objs[i] = hwloc_get_obj_by_type(topology, binding_level, i);
    }

    num_ancestors = num_objs * (num_objs - 1) / 2;

    if ((array = (ancestor_type *)MPL_malloc(
             num_ancestors * sizeof(ancestor_type), MPL_MEM_OTHER)) == NULL) {
        return;
    }

    k = 0;
    for (i = 0; i < (num_objs - 1); i++) {
        for (j = i + 1; j < num_objs; j++) {
            array[k].obja = objs[i];
            array[k].objb = objs[j];
            array[k].ancestor =
                hwloc_get_common_ancestor_obj(topology, objs[i], objs[j]);
            k++;
        }
    }

    qsort(array, num_ancestors, sizeof(ancestor_type), cmpdepth_smt);

    for (i = 0; i < (num_ancestors - 1); i++) {
        if ((array[i + 1].ancestor)->depth < (array[i].ancestor)->depth) {
            break;
        }
    }

    if (i < num_ancestors - 1)
        qsort(array, (i + 1), sizeof(ancestor_type), cmparity_smt);

    *result = array[0].obja;

    MPL_free(objs);
    MPL_free(array);
    return;
}

/*
 * Yields "scatter" affinity scenario in core_mapping.
 */
void map_scatter_core(int num_cpus)
{
    hwloc_obj_t *objs, obj, a;
    unsigned *pdist, maxd;
    int i, j, ix, jp, d, s;

    /* Init and load HWLOC_OBJ_PU objects */
    if ((objs = (hwloc_obj_t *)MPL_malloc(num_cpus * sizeof(hwloc_obj_t *),
                                          MPL_MEM_OTHER)) == NULL)
        return;

    obj = NULL;
    i = 0;
    while ((obj = hwloc_get_next_obj_by_type(topology, HWLOC_OBJ_PU, obj)) !=
           NULL)
        objs[i++] = obj;
    if (i != num_cpus) {
        MPL_free(objs);
        return;
    }

    /* Sort HWLOC_OBJ_PU objects according to sibling_rank */
    qsort(objs, num_cpus, sizeof(hwloc_obj_t *), cmpproc_smt);

    /* Init cumulative distances */
    if ((pdist = (unsigned *)MPL_malloc(num_cpus * sizeof(unsigned),
                                        MPL_MEM_OTHER)) == NULL) {
        MPL_free(objs);
        return;
    }

    /* Loop over objects, ix is index in objs where sorted objects start */
    ix = num_cpus;
    s = -1;
    while (ix > 0) {
        /* If new group of SMT processors starts, zero distances */
        if (s != objs[0]->sibling_rank) {
            s = objs[0]->sibling_rank;
            for (j = 0; j < ix; j++)
                pdist[j] = 0;
        }
        /*
         * Determine object that has max. distance to all already stored
         * objects. Consider only groups of SMT processors with same
         * sibling_rank.
         */
        maxd = 0;
        jp = 0;
        for (j = 0; j < ix; j++) {
            if ((j) && (objs[j - 1]->sibling_rank != objs[j]->sibling_rank))
                break;
            if (pdist[j] > maxd) {
                maxd = pdist[j];
                jp = j;
            }
        }

        /* Rotate found object to the end of the list, map out found object from
         * distances */
        obj = objs[jp];
        for (j = jp; j < num_cpus - 1; j++) {
            objs[j] = objs[j + 1];
            pdist[j] = pdist[j + 1];
        }
        objs[j] = obj;
        ix--;

        /*
         * Update cumulative distances of all remaining objects with new stored
         * one. If two HWLOC_OBJ_PU objects don't share a common ancestor, the
         * topology is broken. Our scheme cannot be used in this case.
         */
        for (j = 0; j < ix; j++) {
            if ((a = hwloc_get_common_ancestor_obj(topology, obj, objs[j])) ==
                NULL) {
                MPL_free(pdist);
                MPL_free(objs);
                return;
            }
            d = objs[j]->depth + obj->depth - 2 * a->depth;
            pdist[j] += d * d;
        }
    }

    /* Collect os_indexes into core_mapping */
    for (i = 0; i < num_cpus; i++) {
        core_mapping[i] = objs[i]->os_index;
    }

    MPL_free(pdist);
    MPL_free(objs);
    return;
}

void map_scatter_socket(int num_sockets, hwloc_obj_type_t binding_level)
{
    hwloc_obj_t *objs, obj, a;
    unsigned *pdist, maxd;
    int i, j, ix, jp, d, s, num_cores;

    /* Init and load HWLOC_OBJ_SOCKET or HWLOC_OBJ_NODE objects */
    if ((objs = (hwloc_obj_t *)MPL_malloc(num_sockets * sizeof(hwloc_obj_t *),
                                          MPL_MEM_OTHER)) == NULL)
        return;

    if ((num_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE)) <= 0) {
        return;
    }

    obj = NULL;
    i = 0;
    while ((obj = hwloc_get_next_obj_by_type(topology, binding_level, obj)) !=
           NULL)
        objs[i++] = obj;
    if (i != num_sockets) {
        MPL_free(objs);
        return;
    }

    /* Sort HWLOC_OBJ_SOCKET or HWLOC_OBJ_NODE objects according to sibling_rank
     */
    qsort(objs, num_sockets, sizeof(hwloc_obj_t *), cmpproc_smt);

    /* Init cumulative distances */
    if ((pdist = (unsigned *)MPL_malloc(num_sockets * sizeof(unsigned),
                                        MPL_MEM_OTHER)) == NULL) {
        MPL_free(objs);
        return;
    }

    /* Loop over objects, ix is index in objs where sorted objects start */
    ix = num_sockets;
    s = -1;
    while (ix > 0) {
        /* If new group of SMT processors starts, zero distances */
        if (s != objs[0]->sibling_rank) {
            s = objs[0]->sibling_rank;
            for (j = 0; j < ix; j++)
                pdist[j] = 0;
        }
        /*
         * Determine object that has max. distance to all already stored
         * objects. Consider only groups of SMT processors with same
         * sibling_rank.
         */
        maxd = 0;
        jp = 0;
        for (j = 0; j < ix; j++) {
            if ((j) && (objs[j - 1]->sibling_rank != objs[j]->sibling_rank))
                break;
            if (pdist[j] > maxd) {
                maxd = pdist[j];
                jp = j;
            }
        }

        /* Rotate found object to the end of the list, map out found object from
         * distances */
        obj = objs[jp];
        for (j = jp; j < num_sockets - 1; j++) {
            objs[j] = objs[j + 1];
            pdist[j] = pdist[j + 1];
        }
        objs[j] = obj;
        ix--;

        /*
         * Update cumulative distances of all remaining objects with new stored
         * one. If two HWLOC_OBJ_SOCKET or HWLOC_OBJ_NODE objects don't share a
         * common ancestor, the topology is broken. Our scheme cannot be used in
         * this case.
         */
        for (j = 0; j < ix; j++) {
            if ((a = hwloc_get_common_ancestor_obj(topology, obj, objs[j])) ==
                NULL) {
                MPL_free(pdist);
                MPL_free(objs);
                return;
            }
            d = objs[j]->depth + obj->depth - 2 * a->depth;
            pdist[j] += d * d;
        }
    }

    /* Collect os_indexes into core_mapping */
    for (i = 0, j = 0; i < num_cores; i++, j++) {
        if (j == num_sockets) {
            j = 0;
        }
        core_mapping[i] =
            hwloc_bitmap_to_ulong((hwloc_const_bitmap_t)(objs[j]->cpuset));
    }

    MPL_free(pdist);
    MPL_free(objs);
    return;
}

/*
 * Yields "bunch" affinity scenario in core_mapping.
 */
void map_bunch_core(int num_cpus)
{
    hwloc_obj_t *objs, obj, a;
    unsigned *pdist, mind;
    int i, j, ix, jp, d, s, num_cores, num_pus;

    /* Init and load HWLOC_OBJ_PU objects */
    if ((objs = (hwloc_obj_t *)MPL_malloc(num_cpus * sizeof(hwloc_obj_t *),
                                          MPL_MEM_OTHER)) == NULL)
        return;

    obj = NULL;
    i = 0;

    if ((num_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE)) <= 0) {
        MPL_free(objs);
        return;
    }

    if ((num_pus = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU)) <= 0) {
        MPL_free(objs);
        return;
    }

    /* SMT Disabled */
    if (num_cores == num_pus) {
        get_first_obj_bunch(&obj);

        if (obj == NULL) {
            MPL_free(objs);
            return;
        }

        objs[i] = obj;
        i++;

        while ((obj = hwloc_get_next_obj_by_type(topology, HWLOC_OBJ_PU,
                                                 obj)) != NULL) {
            objs[i] = obj;
            i++;
        }

        obj = NULL;
        while (i != num_cpus) {
            obj = hwloc_get_next_obj_by_type(topology, HWLOC_OBJ_PU, obj);
            objs[i++] = obj;
        }

        if (i != num_cpus) {
            MPL_free(objs);
            return;
        }

    } else { /* SMT Enabled */

        while ((obj = hwloc_get_next_obj_by_type(topology, HWLOC_OBJ_PU,
                                                 obj)) != NULL)
            objs[i++] = obj;

        if (i != num_cpus) {
            MPL_free(objs);
            return;
        }

        /* Sort HWLOC_OBJ_PU objects according to sibling_rank */
        qsort(objs, num_cpus, sizeof(hwloc_obj_t *), cmpproc_smt);
    }

    /* Init cumulative distances */
    if ((pdist = (unsigned *)MPL_malloc(num_cpus * sizeof(unsigned),
                                        MPL_MEM_OTHER)) == NULL) {
        MPL_free(objs);
        return;
    }

    /* Loop over objects, ix is index in objs where sorted objects start */
    ix = num_cpus;
    s = -1;
    while (ix > 0) {
        /* If new group of SMT processors starts, zero distances */
        if (s != objs[0]->sibling_rank) {
            s = objs[0]->sibling_rank;
            for (j = 0; j < ix; j++)
                pdist[j] = UINT_MAX;
        }
        /*
         * Determine object that has min. distance to all already stored
         * objects. Consider only groups of SMT processors with same
         * sibling_rank.
         */
        mind = UINT_MAX;
        jp = 0;
        for (j = 0; j < ix; j++) {
            if ((j) && (objs[j - 1]->sibling_rank != objs[j]->sibling_rank))
                break;
            if (pdist[j] < mind) {
                mind = pdist[j];
                jp = j;
            }
        }

        /* Rotate found object to the end of the list, map out found object from
         * distances */
        obj = objs[jp];
        for (j = jp; j < num_cpus - 1; j++) {
            objs[j] = objs[j + 1];
            pdist[j] = pdist[j + 1];
        }
        objs[j] = obj;
        ix--;

        /*
         * Update cumulative distances of all remaining objects with new stored
         * one. If two HWLOC_OBJ_PU objects don't share a common ancestor, the
         * topology is broken. Our scheme cannot be used in this case.
         */
        for (j = 0; j < ix; j++) {
            if ((a = hwloc_get_common_ancestor_obj(topology, obj, objs[j])) ==
                NULL) {
                MPL_free(pdist);
                MPL_free(objs);
                return;
            }
            d = objs[j]->depth + obj->depth - 2 * a->depth;
            pdist[j] += d * d;
        }
    }

    /* Collect os_indexes into core_mapping */
    for (i = 0; i < num_cpus; i++) {
        core_mapping[i] = objs[i]->os_index;
    }

    MPL_free(pdist);
    MPL_free(objs);
    return;
}

int check_num_child(hwloc_obj_t obj)
{
    int i = 0, k, num_cores;

    if ((num_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE)) <= 0) {
        return 0;
    }

    for (k = 0; k < num_cores; k++) {
        if (hwloc_bitmap_isset((hwloc_const_bitmap_t)(obj->cpuset), k)) {
            i++;
        }
    }

    return i;
}

void map_bunch_socket(int num_sockets, hwloc_obj_type_t binding_level)
{
    hwloc_obj_t *objs, obj, a;
    unsigned *pdist, mind;
    int i, j, ix, jp, d, s, num_cores, num_pus;

    /* Init and load HWLOC_OBJ_PU objects */
    if ((objs = (hwloc_obj_t *)MPL_malloc(num_sockets * sizeof(hwloc_obj_t *),
                                          MPL_MEM_OTHER)) == NULL)
        return;

    obj = NULL;
    i = 0;

    if ((num_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE)) <= 0) {
        MPL_free(objs);
        return;
    }

    if ((num_pus = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU)) <= 0) {
        MPL_free(objs);
        return;
    }

    /* SMT Disabled */
    if (num_cores == num_pus) {
        get_first_socket_bunch(&obj, binding_level);

        if (obj == NULL) {
            MPL_free(objs);
            return;
        }

        objs[i] = obj;
        i++;

        while ((obj = hwloc_get_next_obj_by_type(topology, binding_level,
                                                 obj)) != NULL) {
            objs[i] = obj;
            i++;
        }

        obj = NULL;
        while (i != num_sockets) {
            obj = hwloc_get_next_obj_by_type(topology, binding_level, obj);
            objs[i++] = obj;
        }

        if (i != num_sockets) {
            MPL_free(objs);
            return;
        }

    } else { /* SMT Enabled */

        while ((obj = hwloc_get_next_obj_by_type(topology, binding_level,
                                                 obj)) != NULL)
            objs[i++] = obj;

        if (i != num_sockets) {
            MPL_free(objs);
            return;
        }

        /* Sort HWLOC_OBJ_SOCKET or HWLOC_OBJ_NODE objects according to
         * sibling_rank */
        qsort(objs, num_sockets, sizeof(hwloc_obj_t *), cmpproc_smt);
    }

    /* Init cumulative distances */
    if ((pdist = (unsigned *)MPL_malloc(num_sockets * sizeof(unsigned),
                                        MPL_MEM_OTHER)) == NULL) {
        MPL_free(objs);
        return;
    }

    /* Loop over objects, ix is index in objs where sorted objects start */
    ix = num_sockets;
    s = -1;
    while (ix > 0) {
        /* If new group of SMT processors starts, zero distances */
        if (s != objs[0]->sibling_rank) {
            s = objs[0]->sibling_rank;
            for (j = 0; j < ix; j++)
                pdist[j] = UINT_MAX;
        }
        /*
         * Determine object that has min. distance to all already stored
         * objects. Consider only groups of SMT processors with same
         * sibling_rank.
         */
        mind = UINT_MAX;
        jp = 0;
        for (j = 0; j < ix; j++) {
            if ((j) && (objs[j - 1]->sibling_rank != objs[j]->sibling_rank))
                break;
            if (pdist[j] < mind) {
                mind = pdist[j];
                jp = j;
            }
        }

        /* Rotate found object to the end of the list, map out found object from
         * distances */
        obj = objs[jp];
        for (j = jp; j < num_sockets - 1; j++) {
            objs[j] = objs[j + 1];
            pdist[j] = pdist[j + 1];
        }
        objs[j] = obj;
        ix--;

        /*
         * Update cumulative distances of all remaining objects with new stored
         * one. If two HWLOC_OBJ_SOCKET or HWLOC_OBJ_NODE objects don't share a
         * common ancestor, the topology is broken. Our scheme cannot be used in
         * this case.
         */
        for (j = 0; j < ix; j++) {
            if ((a = hwloc_get_common_ancestor_obj(topology, obj, objs[j])) ==
                NULL) {
                MPL_free(pdist);
                MPL_free(objs);
                return;
            }
            d = objs[j]->depth + obj->depth - 2 * a->depth;
            pdist[j] += d * d;
        }
    }

    /* Collect os_indexes into core_mapping */
    int num_child_in_socket[num_sockets];

    for (i = 0; i < num_sockets; i++) {
        num_child_in_socket[i] = check_num_child(objs[i]);
    }

    for (i = 1; i < num_sockets; i++)
        num_child_in_socket[i] += num_child_in_socket[i - 1];

    for (i = 0, j = 0; i < num_cores; i++) {
        if (i == num_child_in_socket[j]) {
            j++;
        }
        core_mapping[i] =
            hwloc_bitmap_to_ulong((hwloc_const_bitmap_t)(objs[j]->cpuset));
    }

    MPL_free(pdist);
    MPL_free(objs);
    return;
}

static int num_digits(unsigned long numcpus)
{
    int n_digits = 0;
    while (numcpus > 0) {
        n_digits++;
        numcpus /= 10;
    }
    return n_digits;
}

int get_cpu_mapping_hwloc(long N_CPUs_online, hwloc_topology_t tp)
{
    unsigned topodepth = -1, depth = -1;
    int num_processes = 0, rc = 0, i;
    int num_sockets = 0;
    int num_numanodes = 0;
    int num_cpus = 0;
    char *s;
    struct dirent **namelist;
    pid_t pid;
    obj_attribute_type *tree = NULL;
    char *value;

    /* Determine topology depth */
    topodepth = hwloc_topology_get_depth(tp);
    if (topodepth == HWLOC_TYPE_DEPTH_UNKNOWN) {
        fprintf(stderr, "Warning: %s: Failed to determine topology depth.\n",
                __func__);
        return (topodepth);
    }

    /* Count number of (logical) processors */
    depth = hwloc_get_type_depth(tp, HWLOC_OBJ_PU);

    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
        fprintf(stderr,
                "Warning: %s: Failed to determine number of processors.\n",
                __func__);
        return (depth);
    }
    if ((num_cpus = hwloc_get_nbobjs_by_type(tp, HWLOC_OBJ_PU)) <= 0) {
        fprintf(stderr,
                "Warning: %s: Failed to determine number of processors.\n",
                __func__);
        return -1;
    }

    /* Count number of sockets */
    depth = hwloc_get_type_depth(tp, HWLOC_OBJ_SOCKET);
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
        fprintf(stderr, "Warning: %s: Failed to determine number of sockets.\n",
                __func__);
        return (depth);
    } else {
        num_sockets = hwloc_get_nbobjs_by_depth(tp, depth);
    }

    /* Count number of numanodes */
    depth = hwloc_get_type_depth(tp, HWLOC_OBJ_NODE);
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
        num_numanodes = -1;
    } else {
        num_numanodes = hwloc_get_nbobjs_by_depth(tp, depth);
    }

    if (s_cpu_mapping == NULL) {
        /* We need to do allocate memory for the custom_cpu_mapping array
         * and determine the current load on the different cpu's only
         * when the user has not specified a mapping string. If the user
         * has provided a mapping string, it overrides everything.
         */
        /*TODO: might need a better representation as number of cores per node
         * increases */
        unsigned long long_max = ULONG_MAX;
        int n_digits = num_digits(long_max);
        custom_cpu_mapping = MPL_malloc(
            sizeof(char) * num_cpus * (n_digits + 1) + 1, MPL_MEM_OTHER);
        if (custom_cpu_mapping == NULL) {
            goto error_free;
        }
        MPIR_Memset(custom_cpu_mapping, 0,
                    sizeof(char) * num_cpus * (n_digits + 1) + 1);
        core_mapping = (unsigned long *)MPL_malloc(
            num_cpus * sizeof(unsigned long), MPL_MEM_OTHER);
        if (core_mapping == NULL) {
            goto error_free;
        }
        for (i = 0; i < num_cpus; i++) {
            core_mapping[i] = -1;
        }

        tree = MPL_malloc(num_cpus * topodepth * sizeof(obj_attribute_type),
                          MPL_MEM_OTHER);
        if (tree == NULL) {
            goto error_free;
        }
        for (i = 0; i < num_cpus * topodepth; i++) {
            tree[i].obj = NULL;
            tree[i].load = 0;
            CPU_ZERO(&(tree[i].cpuset));
        }

        if (!(obj_tree = (int *)MPL_malloc(
                  num_cpus * topodepth * sizeof(*obj_tree), MPL_MEM_OTHER))) {
            goto error_free;
        }
        for (i = 0; i < num_cpus * topodepth; i++) {
            obj_tree[i] = -1;
        }

        ip = 0;


        /* MVP_ENABLE_LEASTLOAD=1, map_bunch_load or map_scatter_load is used */
        if (MVP_ENABLE_LEASTLOAD) {
            /*
             * Get all processes' pid and cpuset.
             * Get numanode, socket, and core current load according to
             * processes running on it.
             */
            num_processes = scandir("/proc", &namelist, pid_filter, alphasort);
            if (num_processes < 0) {
                fprintf(stderr, "Warning: %s: Failed to scandir /proc.\n",
                        __func__);
                return -1;
            } else {
                int status;
                cpu_set_t pid_cpuset;
                CPU_ZERO(&pid_cpuset);

                /* Get cpuset for each running process. */
                for (i = 0; i < num_processes; i++) {
                    pid = atol(namelist[i]->d_name);
                    status =
                        sched_getaffinity(pid, sizeof(pid_cpuset), &pid_cpuset);
                    /* Process completed. */
                    if (status < 0) {
                        continue;
                    }
                    cac_load(tree, pid_cpuset);
                }
                while (num_processes--) {
                    MPL_free(namelist[num_processes]);
                }
                MPL_free(namelist);
            }

            switch (MVP_CPU_BINDING_POLICY) {
                case MVP_CPU_BINDING_POLICY_SCATTER:
                    map_scatter_load(tree);
                    break;
                case MVP_CPU_BINDING_POLICY_BUNCH:
                    map_bunch_load(tree);
                    break;
                default:
                    goto error_free;
            }
        } else {
            /* MVP_ENABLE_LEASTLOAD != 1 or MVP_ENABLE_LEASTLOAD == NULL,
             * map_bunch or map_scatter is used */
            hwloc_obj_type_t binding_level = HWLOC_OBJ_SOCKET;
            switch (MVP_CPU_BINDING_POLICY) {
                case MVP_CPU_BINDING_POLICY_SCATTER:
                    /* Scatter */
                    if (mvp_binding_level == LEVEL_SOCKET) {
                        map_scatter_socket(num_sockets, binding_level);
                    } else if (mvp_binding_level == LEVEL_NUMANODE) {
                        if (num_numanodes == -1) {
                            /* There is not numanode, fallback to socket */
                            map_scatter_socket(num_sockets, binding_level);
                        } else {
                            binding_level = HWLOC_OBJ_NODE;
                            map_scatter_socket(num_numanodes, binding_level);
                        }
                    } else {
                        map_scatter_core(num_cpus);
                    }
                    break;
                case MVP_CPU_BINDING_POLICY_BUNCH:
                    /* Bunch */
                    if (mvp_binding_level == LEVEL_SOCKET) {
                        map_bunch_socket(num_sockets, binding_level);
                    } else if (mvp_binding_level == LEVEL_NUMANODE) {
                        if (num_numanodes == -1) {
                            /* There is not numanode, fallback to socket */
                            map_bunch_socket(num_sockets, binding_level);
                        } else {
                            binding_level = HWLOC_OBJ_NODE;
                            map_bunch_socket(num_numanodes, binding_level);
                        }
                    } else {
                        map_bunch_core(num_cpus);
                    }
                    break;
                default:
                    goto error_free;
            }
        }

        /* Assemble custom_cpu_mapping string */
        s = custom_cpu_mapping;
        for (i = 0; i < num_cpus; i++) {
            s += sprintf(s, "%lu:", core_mapping[i]);
        }
    }

    /* Done */
    rc = MPI_SUCCESS;

error_free:
    if (core_mapping != NULL) {
        MPL_free(core_mapping);
    }
    if (tree != NULL) {
        MPL_free(tree);
    }
    if (obj_tree) {
        MPL_free(obj_tree);
    }

    PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                "num_cpus: %d, num_sockets: %d, custom_cpu_mapping: %s\n",
                num_cpus, num_sockets, custom_cpu_mapping);

    return rc;
}

int get_cpu_mapping(long N_CPUs_online)
{
    char line[MAX_LINE_LENGTH];
    char input[MAX_NAME_LENGTH];
    char bogus1[MAX_NAME_LENGTH];
    char bogus2[MAX_NAME_LENGTH];
    char bogus3[MAX_NAME_LENGTH];
    int physical_id; // return value
    int mapping[N_CPUs_online];
    int core_index = 0;
    cpu_type_t cpu_type = 0;
    int model;
    int vendor_set = 0, model_set = 0, num_cpus = 0;

    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        printf("can not open cpuinfo file \n");
        return 0;
    }

    MPIR_Memset(mapping, 0, sizeof(mapping));
    custom_cpu_mapping =
        (char *)MPL_malloc(sizeof(char) * N_CPUs_online * 2, MPL_MEM_OTHER);
    if (custom_cpu_mapping == NULL) {
        return 0;
    }
    MPIR_Memset(custom_cpu_mapping, 0, sizeof(char) * N_CPUs_online * 2);

    while (!feof(fp)) {
        MPIR_Memset(line, 0, MAX_LINE_LENGTH);
        fgets(line, MAX_LINE_LENGTH, fp);

        MPIR_Memset(input, 0, MAX_NAME_LENGTH);
        sscanf(line, "%s", input);

        if (!vendor_set) {
            if (strcmp(input, "vendor_id") == 0) {
                MPIR_Memset(input, 0, MAX_NAME_LENGTH);
                sscanf(line, "%s%s%s", bogus1, bogus2, input);

                if (strcmp(input, "AuthenticAMD") == 0) {
                    cpu_type = CPU_FAMILY_AMD;
                } else {
                    cpu_type = CPU_FAMILY_INTEL;
                }
                vendor_set = 1;
            }
        }

        if (!model_set) {
            if (strcmp(input, "model") == 0) {
                sscanf(line, "%s%s%d", bogus1, bogus2, &model);
                model_set = 1;
            }
        }

        if (strcmp(input, "physical") == 0) {
            sscanf(line, "%s%s%s%d", bogus1, bogus2, bogus3, &physical_id);
            mapping[core_index++] = physical_id;
        }
    }

    num_cpus = core_index;
    if (num_cpus == 4) {
        if ((memcmp(INTEL_XEON_DUAL_MAPPING, mapping, sizeof(int) * num_cpus) ==
             0) &&
            (cpu_type == CPU_FAMILY_INTEL)) {
            strcpy(custom_cpu_mapping, "0:2:1:3");
        } else if ((memcmp(AMD_OPTERON_DUAL_MAPPING, mapping,
                           sizeof(int) * num_cpus) == 0) &&
                   (cpu_type == CPU_FAMILY_AMD)) {
            strcpy(custom_cpu_mapping, "0:1:2:3");
        }
    } else if (num_cpus == 8) {
        if (cpu_type == CPU_FAMILY_INTEL) {
            if (model == CLOVERTOWN_MODEL) {
                if (memcmp(INTEL_CLOVERTOWN_MAPPING, mapping,
                           sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping, "0:1:4:5:2:3:6:7");
                }
            } else if (model == HARPERTOWN_MODEL) {
                if (memcmp(INTEL_HARPERTOWN_LEG_MAPPING, mapping,
                           sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping, "0:1:4:5:2:3:6:7");
                } else if (memcmp(INTEL_HARPERTOWN_COM_MAPPING, mapping,
                                  sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping, "0:4:2:6:1:5:3:7");
                }
            } else if (model == NEHALEM_MODEL) {
                if (memcmp(INTEL_NEHALEM_LEG_MAPPING, mapping,
                           sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping, "0:2:4:6:1:3:5:7");
                } else if (memcmp(INTEL_NEHALEM_COM_MAPPING, mapping,
                                  sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping, "0:4:1:5:2:6:3:7");
                }
            }
        }
    } else if (num_cpus == 16) {
        if (cpu_type == CPU_FAMILY_INTEL) {
            if (model == NEHALEM_MODEL) {
                if (memcmp(INTEL_NEHALEM_LEG_MAPPING, mapping,
                           sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping,
                           "0:2:4:6:1:3:5:7:8:10:12:14:9:11:13:15");
                } else if (memcmp(INTEL_NEHALEM_COM_MAPPING, mapping,
                                  sizeof(int) * num_cpus) == 0) {
                    strcpy(custom_cpu_mapping,
                           "0:4:1:5:2:6:3:7:8:12:9:13:10:14:11:15");
                }
            }
        } else if (cpu_type == CPU_FAMILY_AMD) {
            if (memcmp(AMD_BARCELONA_MAPPING, mapping,
                       sizeof(int) * num_cpus) == 0) {
                strcpy(custom_cpu_mapping,
                       "0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15");
            }
        }
    }
    fclose(fp);

    return MPI_SUCCESS;
}

#if defined(CHANNEL_MRAIL)
int get_socket_id(int ib_socket, int cpu_socket, int num_sockets,
                  tab_socket_t *tab_socket)
{
    extern int rdma_local_id, rdma_num_hcas;

    int rdma_num_proc_per_hca;
    int offset_id;
    int j;
    int socket_id = ib_socket;
    int delta = cpu_socket / tab_socket[ib_socket].num_hca;

    rdma_num_proc_per_hca = rdma_num_local_procs / rdma_num_hcas;

    if (rdma_num_local_procs % rdma_num_hcas) {
        rdma_num_proc_per_hca++;
    }

    offset_id = rdma_local_id % rdma_num_proc_per_hca;

    if (offset_id < delta) {
        return ib_socket;
    }

    for (j = 0; j < num_sockets - 1; j++) {
        socket_id = tab_socket[ib_socket].closest[j];

        if (tab_socket[socket_id].num_hca == 0) {
            offset_id -= delta;

            if (offset_id < delta) {
                return socket_id;
            }
        }
    }

    /*
     * Couldn't find a free socket, spread remaining processes
     */
    return rdma_local_id % num_sockets;
}

int mvp_get_cpu_core_closest_to_hca(int my_local_id, int total_num_cores,
                                    int num_sockets, int depth_sockets)
{
    int i = 0, k = 0;
    int ib_hca_selected = 0;
    int selected_socket = 0;
    int cores_per_socket = 0;
    tab_socket_t *tab_socket = NULL;
    int linelen = strlen(custom_cpu_mapping);

    if (linelen < custom_cpu_mapping_line_max) {
        custom_cpu_mapping_line_max = linelen;
    }

    cores_per_socket = total_num_cores / num_sockets;

    /*
     * Make ib_hca_selected global or make this section a function
     */
    if (FIXED_MAPPING == rdma_rail_sharing_policy) {
        ib_hca_selected =
            rdma_process_binding_rail_offset / rdma_num_rails_per_hca;
    } else {
        ib_hca_selected = 0;
    }

    tab_socket = (tab_socket_t *)MPL_malloc(num_sockets * sizeof(tab_socket_t),
                                            MPL_MEM_OTHER);
    if (NULL == tab_socket) {
        fprintf(stderr, "could not allocate the socket table\n");
        return -1;
    }

    for (i = 0; i < num_sockets; i++) {
        tab_socket[i].num_hca = 0;

        for (k = 0; k < num_sockets; k++) {
            tab_socket[i].closest[k] = -1;
        }
    }

    for (i = 0; i < rdma_num_hcas; i++) {
        struct ibv_device *ibdev = mvp_MPIDI_CH3I_RDMA_Process.ib_dev[i];
        int socket_id = get_ib_socket(ibdev);
        /*
         * Make this information available globally
         */
        if (i == ib_hca_selected) {
            ib_socket_bind = socket_id;
        }
        tab_socket[socket_id].num_hca++;
    }

    hwloc_obj_t obj_src;
    hwloc_obj_t objs[num_sockets];
    char string[20];

    for (i = 0; i < num_sockets; i++) {
        obj_src = hwloc_get_obj_by_type(topology, HWLOC_OBJ_SOCKET, i);
        hwloc_get_closest_objs(topology, obj_src, (hwloc_obj_t *)&objs,
                               num_sockets - 1);

        for (k = 0; k < num_sockets - 1; k++) {
            hwloc_obj_type_snprintf(string, sizeof(string), objs[k], 1);
            tab_socket[i].closest[k] = objs[k]->os_index;
        }
    }

    selected_socket = get_socket_id(ib_socket_bind, cores_per_socket,
                                    num_sockets, tab_socket);
    MPL_free(tab_socket);

    return selected_socket;
}
#endif /* defined(CHANNEL_MRAIL) */

int mvp_get_assigned_cpu_core(int my_local_id, char *cpu_mapping,
                              int max_cpu_map_len, char *tp_str)
{
    int i = 0, j = 0, c = 0;
    char *cp = NULL;
    char *tp = cpu_mapping;
    long N_CPUs_online = sysconf(_SC_NPROCESSORS_ONLN);

    while (*tp != '\0') {
        i = 0;
        cp = tp;

        while (*cp != '\0' && *cp != ':' && i < max_cpu_map_len) {
            ++cp;
            ++i;
        }

        if (j == my_local_id) {
            strncpy(tp_str, tp, i);
            c = atoi(tp);
            if ((mvp_binding_level == LEVEL_CORE) &&
                (c < 0 || c >= N_CPUs_online)) {
                fprintf(stderr,
                        "Warning! : Core id %d does not exist on this "
                        "architecture! \n",
                        c);
                fprintf(stderr, "CPU Affinity is undefined \n");
                mvp_enable_affinity = 0;
                return -1;
            }
            tp_str[i] = '\0';
            return 0;
        }

        if (*cp == '\0') {
            break;
        }

        tp = cp;
        ++tp;
        ++j;
    }

    return -1;
}

#if defined(CHANNEL_MRAIL)
int smpi_set_progress_thread_affinity()
{
    int mpi_errno = MPI_SUCCESS;
    hwloc_cpuset_t cpuset;

    /* Alloc cpuset */
    cpuset = hwloc_bitmap_alloc();
    /* Set cpuset to mvp_my_async_cpu_id */
    hwloc_bitmap_set(cpuset, mvp_my_async_cpu_id);
    /* Attachment progress thread to mvp_my_async_cpu_id */
    hwloc_set_thread_cpubind(topology, pthread_self(), cpuset, 0);
    /* Free cpuset */
    hwloc_bitmap_free(cpuset);

    return mpi_errno;
}

int smpi_identify_allgather_local_core_ids(MPIDI_PG_t *pg)
{
    int mpi_errno = MPI_SUCCESS;
    int p = 0;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIR_Request **request = NULL;
    MPI_Status *status = NULL;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIR_Comm *comm_ptr = NULL;

    MPIR_Comm_get_ptr(MPI_COMM_WORLD, comm_ptr);

    /* Allocate memory */
    local_core_ids =
        MPL_malloc(mvp_smp_info.num_local_nodes * sizeof(int), MPL_MEM_OTHER);
    if (local_core_ids == NULL) {
        ibv_error_abort(GEN_EXIT_ERR,
                        "Failed to allocate memory for local_core_ids\n");
    }
    request =
        MPL_malloc(mvp_smp_info.num_local_nodes * 2 * sizeof(MPIR_Request *),
                   MPL_MEM_OTHER);
    if (request == NULL) {
        ibv_error_abort(GEN_EXIT_ERR,
                        "Failed to allocate memory for requests\n");
    }
    status = MPL_malloc(mvp_smp_info.num_local_nodes * 2 * sizeof(MPI_Status),
                        MPL_MEM_OTHER);
    if (request == NULL) {
        ibv_error_abort(GEN_EXIT_ERR,
                        "Failed to allocate memory for statuses\n");
    }
    /* Perform intra-node allgather */
    for (p = 0; p < mvp_smp_info.num_local_nodes; ++p) {
        MPIDI_PG_Get_vc(pg, mvp_smp_info.l2g_rank[p], &vc);
        if (vc->smp.local_nodes >= 0) {
            mpi_errno =
                MPIC_Irecv((void *)&local_core_ids[vc->smp.local_nodes], 1,
                           MPI_INT, vc->pg_rank, MPIR_ALLGATHER_TAG, comm_ptr,
                           &request[mvp_smp_info.num_local_nodes + p]);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
            mpi_errno =
                MPIC_Isend((void *)&mvp_my_cpu_id, 1, MPI_INT, vc->pg_rank,
                           MPIR_ALLGATHER_TAG, comm_ptr, &request[p], &errflag);
            if (mpi_errno) {
                MPIR_ERR_POP(mpi_errno);
            }
        }
    }
    /* Wait for intra-node allgather to finish */
    mpi_errno = MPIC_Waitall(mvp_smp_info.num_local_nodes * 2, request, status,
                             &errflag);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    if (request) {
        MPL_free(request);
    }
    if (status) {
        MPL_free(status);
    }
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

int smpi_identify_free_cores(hwloc_cpuset_t *sock_cpuset,
                             hwloc_cpuset_t *free_sock_cpuset)
{
    int i = 0;
    int mpi_errno = MPI_SUCCESS;
    int num_sockets = -1;
    int depth_sockets = -1;
    hwloc_obj_t socket = NULL;
    hwloc_cpuset_t my_cpuset = NULL;
    char cpu_str[128];

    /* Alloc cpuset */
    my_cpuset = hwloc_bitmap_alloc();
    *sock_cpuset = hwloc_bitmap_alloc();
    /* Clear CPU set */
    hwloc_bitmap_zero(my_cpuset);
    hwloc_bitmap_zero(*sock_cpuset);
    /* Set cpuset to mvp_my_cpu_id */
    hwloc_bitmap_set(my_cpuset, mvp_my_cpu_id);

    depth_sockets = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    num_sockets = hwloc_get_nbobjs_by_depth(topology, depth_sockets);

    for (i = 0; i < num_sockets; ++i) {
        socket = hwloc_get_obj_by_depth(topology, depth_sockets, i);
        /* Find the list of CPUs we're allowed to use in the socket */
        hwloc_bitmap_and(*sock_cpuset, socket->cpuset,
                         hwloc_topology_get_allowed_cpuset(topology));
        /* Find the socket the core I'm bound to resides on */
        if (hwloc_bitmap_intersects(my_cpuset, *sock_cpuset)) {
            /* Create a copy to identify list of free coress */
            *free_sock_cpuset = hwloc_bitmap_dup(*sock_cpuset);
            /* Store my sock ID */
            mvp_my_sock_id = i;
            break;
        }
    }
    if (i == num_sockets) {
        mpi_errno = MPI_ERR_OTHER;
        MPIR_ERR_POP(mpi_errno);
    } else {
        /* Remove cores used by processes from list of available cores */
        for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
            hwloc_bitmap_clr(*free_sock_cpuset, local_core_ids[i]);
        }
        hwloc_bitmap_snprintf(cpu_str, 128, *free_sock_cpuset);
        PRINT_DEBUG(DEBUG_INIT_verbose, "Free sock_cpuset = %s\n", cpu_str);
    }

    if (my_cpuset) {
        hwloc_bitmap_free(my_cpuset);
    }
fn_fail:
    return mpi_errno;
}

int smpi_identify_core_for_async_thread(MPIDI_PG_t *pg)
{
    int i = 0;
    int mpi_errno = MPI_SUCCESS;
    hwloc_cpuset_t sock_cpuset = NULL;
    hwloc_cpuset_t free_sock_cpuset = NULL;

    /* Gather IDs of cores local processes are bound to */
    mpi_errno = smpi_identify_allgather_local_core_ids(pg);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }
    /* Identify my socket and cores available in my socket */
    mpi_errno = smpi_identify_free_cores(&sock_cpuset, &free_sock_cpuset);
    if (mpi_errno) {
        MPIR_ERR_POP(mpi_errno);
    }
    /* Identify core to be used for async thread */
    if (!hwloc_bitmap_iszero(free_sock_cpuset)) {
        for (i = 0; i < mvp_smp_info.num_local_nodes; ++i) {
            /* If local process 'i' is on a core on my socket */
            if (hwloc_bitmap_isset(sock_cpuset, local_core_ids[i])) {
                mvp_my_async_cpu_id =
                    hwloc_bitmap_next(free_sock_cpuset, mvp_my_async_cpu_id);
                if (i == mvp_smp_info.my_local_id) {
                    break;
                }
            }
        }
        /* Ensure async thread gets bound to a core */
        while (mvp_my_async_cpu_id < 0) {
            mvp_my_async_cpu_id =
                hwloc_bitmap_next(free_sock_cpuset, mvp_my_async_cpu_id);
        }
    }
    PRINT_DEBUG(
        DEBUG_INIT_verbose > 0,
        "[local_rank: %d]: sock_id = %d, cpu_id = %d, async_cpu_id = %d\n",
        mvp_smp_info.my_local_id, mvp_my_sock_id, mvp_my_cpu_id,
        mvp_my_async_cpu_id);

fn_exit:
    /* Free temporary memory */
    if (local_core_ids) {
        MPL_free(local_core_ids);
    }
    /* Free cpuset */
    if (sock_cpuset) {
        hwloc_bitmap_free(sock_cpuset);
    }
    if (free_sock_cpuset) {
        hwloc_bitmap_free(free_sock_cpuset);
    }
    return mpi_errno;

fn_fail:
    goto fn_exit;
}
#endif /*defined(CHANNEL_MRAIL)*/

int smpi_identify_my_sock_id()
{
    int i = 0;
    int mpi_errno = MPI_SUCCESS;
    int num_sockets = -1, depth_numa = -1;
    int depth_sockets = -1;
    int add_topo_comm_level = 1;
    hwloc_obj_t socket = NULL;
    hwloc_cpuset_t my_cpuset = NULL, sock_cpuset = NULL, numa_cpuset = NULL;
    hwloc_obj_t numa = NULL;

    if (mvp_my_sock_id != -1) {
        goto fn_exit;
    }
    /* Alloc cpuset */
    my_cpuset = hwloc_bitmap_alloc();
    sock_cpuset = hwloc_bitmap_alloc();
    /* Clear CPU set */
    hwloc_bitmap_zero(my_cpuset);
    hwloc_bitmap_zero(sock_cpuset);

    hwloc_get_proc_cpubind(topology_whole, getpid(), my_cpuset, 0);

    depth_sockets = hwloc_get_type_depth(topology_whole, HWLOC_OBJ_SOCKET);
    num_sockets = hwloc_get_nbobjs_by_depth(topology_whole, depth_sockets);
    for (i = 0; i < num_sockets; ++i) {
        socket = hwloc_get_obj_by_depth(topology_whole, depth_sockets, i);
        /* Find the list of CPUs we're allowed to use in the socket */
        hwloc_bitmap_and(sock_cpuset, socket->cpuset,
                         hwloc_topology_get_allowed_cpuset(topology_whole));
        /* Find the socket the core I'm bound to resides on */
        if (hwloc_bitmap_intersects(my_cpuset, sock_cpuset)) {
            /* Store my sock ID */
            mvp_my_sock_id = i;
            break;
        }
        hwloc_bitmap_zero(sock_cpuset);
    }

    int numa_id = 0;
    if (mvp_num_intra_node_comm_levels >= 1) {
        numa_id =
            mvp_intra_node_cluster_at_level[mvp_num_intra_node_comm_levels - 1];
    }

    depth_numa = hwloc_get_type_depth(topology_whole, HWLOC_OBJ_NUMANODE);
    numa = hwloc_get_obj_by_depth(topology_whole, depth_numa, numa_id);

    if (numa != NULL) {
        numa_cpuset = numa->cpuset;
    }

    if (num_sockets > 1) {
        /* If numa_cpuset is the same as the process's socket-level cpuset,
         * ignore the current level */
        if (numa_cpuset != NULL &&
            hwloc_bitmap_isincluded(numa_cpuset, sock_cpuset)) {
            add_topo_comm_level = 0;
        }
        if (add_topo_comm_level) {
            mvp_intra_node_cluster_at_level[mvp_num_intra_node_comm_levels] =
                mvp_my_sock_id;
            mvp_num_intra_node_comm_levels++;
        }
    }

    if (my_cpuset) {
        hwloc_bitmap_free(my_cpuset);
    }
    if (sock_cpuset) {
        hwloc_bitmap_free(sock_cpuset);
    }

fn_exit:
    return mpi_errno;
}

int smpi_identify_my_numa_id()
{
    int i = 0;
    int mpi_errno = MPI_SUCCESS;
    int num_numa = -1;
    int depth_numa = -1;
    hwloc_obj_t numa = NULL;
    hwloc_cpuset_t my_cpuset = NULL, numa_cpuset = NULL;

    if (mvp_my_numa_id != -1) {
        goto fn_exit;
    }
    /* Alloc cpuset */
    my_cpuset = hwloc_bitmap_alloc();
    numa_cpuset = hwloc_bitmap_alloc();
    /* Clear CPU set */
    hwloc_bitmap_zero(my_cpuset);
    hwloc_bitmap_zero(numa_cpuset);

    hwloc_get_proc_cpubind(topology_whole, getpid(), my_cpuset, 0);

    depth_numa = hwloc_get_type_depth(topology_whole, HWLOC_OBJ_NUMANODE);
    num_numa = hwloc_get_nbobjs_by_depth(topology_whole, depth_numa);

    for (i = 0; i < num_numa; ++i) {
        numa = hwloc_get_obj_by_depth(topology_whole, depth_numa, i);
        /* Find the list of CPUs we're allowed to use in the numa */
        hwloc_bitmap_and(numa_cpuset, numa->cpuset,
                         hwloc_topology_get_allowed_cpuset(topology_whole));
        /* Find the numa the core I'm bound to resides on */
        if (hwloc_bitmap_intersects(my_cpuset, numa_cpuset)) {
            /* Store my numa ID */
            mvp_my_numa_id = i;
            break;
        }
        hwloc_bitmap_zero(numa_cpuset);
    }
    if (num_numa > 1) {
        mvp_intra_node_cluster_at_level[mvp_num_intra_node_comm_levels] =
            mvp_my_numa_id;
        mvp_num_intra_node_comm_levels++;
    }

    if (my_cpuset) {
        hwloc_bitmap_free(my_cpuset);
    }
    if (numa_cpuset) {
        hwloc_bitmap_free(numa_cpuset);
    }

fn_exit:
    return mpi_errno;
}

#ifdef _USE_HWLOC_V2_
int smpi_identify_my_l3_id()
{
    int i = 0;
    int mpi_errno = MPI_SUCCESS;
    int num_l3 = -1;
    int depth_l3 = -1;
    hwloc_obj_t l3 = NULL;
    hwloc_cpuset_t my_cpuset = NULL, l3_cpuset = NULL;
    char cpu_str[128];

    /* Alloc cpuset */
    my_cpuset = hwloc_bitmap_alloc();
    l3_cpuset = hwloc_bitmap_alloc();
    /* Clear CPU set */
    hwloc_bitmap_zero(my_cpuset);
    hwloc_bitmap_zero(l3_cpuset);
    /* Set cpuset to mvp_my_cpu_id */
    hwloc_bitmap_set(my_cpuset, mvp_my_cpu_id);

    depth_l3 = hwloc_get_type_depth(topology, HWLOC_OBJ_L3CACHE);
    num_l3 = hwloc_get_nbobjs_by_depth(topology, depth_l3);

    for (i = 0; i < num_l3; ++i) {
        l3 = hwloc_get_obj_by_depth(topology, depth_l3, i);
        /* Find the list of CPUs we're allowed to use in the l3 */
        hwloc_bitmap_and(l3_cpuset, l3->cpuset,
                         hwloc_topology_get_allowed_cpuset(topology));
        /* Find the l3 the core I'm bound to resides on */
        if (hwloc_bitmap_intersects(my_cpuset, l3_cpuset)) {
            /* Store my l3 ID */
            mvp_my_l3_id = i;
            break;
        }
    }
    if (num_l3 > 1) {
        mvp_intra_node_cluster_at_level[mvp_num_intra_node_comm_levels] =
            mvp_my_l3_id;
        mvp_num_intra_node_comm_levels++;
    }
    if (my_cpuset) {
        hwloc_bitmap_free(my_cpuset);
    }
    if (l3_cpuset) {
        hwloc_bitmap_free(l3_cpuset);
    }
fn_fail:
    return mpi_errno;
}
#endif /*#ifdef _USE_HWLOC_V2_*/

/* This function is the same as smpi_load_hwloc_topology,
 * but has the HWLOC_TOPOLOGY_FLAG_WHOLE_SYSTEM set. This is
 * useful for certain launchers/clusters where processes don't
 * have a whole view of the system (like in the case of jsrun).
 * It's declared separately to avoid unnecessary overheads in
 * smpi_load_hwloc_topology in cases where a full view of the
 * system is not required.
 * */
int smpi_load_hwloc_topology_whole(void)
{
    int mpi_errno = MPI_SUCCESS;
    char *kvsname = NULL, *value;
    char *hostname = NULL;
    char *tmppath = NULL;
    int uid, my_local_id;
    int kvs_name_sz;

    MPIR_FUNC_TERSE_STATE_DECL(SMPI_LOAD_HWLOC_TOPOLOGY_WHOLE);
    MPIR_FUNC_TERSE_ENTER(SMPI_LOAD_HWLOC_TOPOLOGY_WHOLE);

    if (topology_whole != NULL) {
        goto fn_exit;
    }

    mpi_errno = hwloc_topology_init(&topology_whole);
#if 1 /* _USE_HWLOC_V2_ */
    hwloc_topology_set_io_types_filter(topology_whole,
                                       HWLOC_TYPE_FILTER_KEEP_ALL);
#endif /* _USE_HWLOC_V2_ */
    hwloc_topology_set_flags(topology_whole,
#if 0  /* _USE_HWLOC_V1_ */
            HWLOC_TOPOLOGY_FLAG_IO_DEVICES |
#endif /* _USE_HWLOC_V1_ */
                             HWLOC_TOPOLOGY_FLAG_WHOLE_SYSTEM |
                                 HWLOC_TOPOLOGY_FLAG_IS_THISSYSTEM);

    uid = getuid();
    my_local_id = MPIR_Process.local_rank;

    mpi_errno = UPMI_KVS_GET_NAME_LENGTH_MAX(&kvs_name_sz);
    if (mpi_errno) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                             "**pmi_kvs_get_name_length_max",
                             "**pmi_kvs_get_name_length_max %d", mpi_errno);
    }
    kvsname = (char *)MPL_malloc(kvs_name_sz + 1, MPL_MEM_STRINGS);
    mpi_errno = UPMI_KVS_GET_MY_NAME(kvsname, kvs_name_sz);
    if (mpi_errno) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**pmi_kvs_get_my_name",
                             "**pmi_kvs_get_my_name %d", mpi_errno);
    }

    if (my_local_id < 0) {
        if (MPIR_Process.rank == 0) {
            PRINT_ERROR("WARNING! Invalid my_local_id: %d, Disabling hwloc "
                        "topology broadcast\n",
                        my_local_id);
        }
        MVP_BCAST_HWLOC_TOPOLOGY = 0;
    }

    if (!MVP_BCAST_HWLOC_TOPOLOGY) {
        /* Each process loads topology individually */
        mpi_errno = hwloc_topology_load(topology_whole);
        goto fn_exit;
    }

    hostname =
        (char *)MPL_malloc(sizeof(char) * HOSTNAME_LENGTH, MPL_MEM_OTHER);
    tmppath = (char *)MPL_malloc(sizeof(char) * FILENAME_LENGTH, MPL_MEM_OTHER);
    whole_topology_xml_path =
        (char *)MPL_malloc(sizeof(char) * FILENAME_LENGTH, MPL_MEM_OTHER);
    if (hostname == NULL || tmppath == NULL ||
        whole_topology_xml_path == NULL) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
                                  "**nomem %s", "mvp_hwloc_topology_file");
    }

    if (gethostname(hostname, sizeof(char) * HOSTNAME_LENGTH) < 0) {
        MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail", "%s: %s",
                                  "gethostname", strerror(errno));
    }
    sprintf(tmppath, "/tmp/mvp-hwloc-%s-%s-%d-whole.tmp", kvsname, hostname,
            uid);
    sprintf(whole_topology_xml_path, "/tmp/mvp-hwloc-%s-%s-%d-whole.xml",
            kvsname, hostname, uid);

    /* Local Rank 0 broadcasts topology using xml */
    if (0 == my_local_id) {
        mpi_errno = hwloc_topology_load(topology_whole);
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
#if 0 /* _USE_HWLOC_V1_ */
        mpi_errno = hwloc_topology_export_xml(topology_whole, tmppath);
#else
        mpi_errno = hwloc_topology_export_xml(topology_whole, tmppath, 0);
#endif
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
        if (rename(tmppath, whole_topology_xml_path) < 0) {
            MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "%s: %s", "rename", strerror(errno));
        }
    } else {
        while (access(whole_topology_xml_path, F_OK) == -1) {
            usleep(1000);
        }
        mpi_errno =
            hwloc_topology_set_xml(topology_whole, whole_topology_xml_path);
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
        mpi_errno = hwloc_topology_load(topology_whole);
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
    }

fn_exit:
    if (hostname) {
        MPL_free(hostname);
    }
    if (tmppath) {
        MPL_free(tmppath);
    }
    if (kvsname) {
        MPL_free(kvsname);
    }
    MPIR_FUNC_TERSE_EXIT(SMPI_LOAD_HWLOC_TOPOLOGY_WHOLE);
    return mpi_errno;

fn_fail:
    if (MVP_BCAST_HWLOC_TOPOLOGY != 0) {
        PRINT_ERROR("Please retry after setting MVP_BCAST_HWLOC_TOPOLOGY=0\n");
    }
    goto fn_exit;
}

int smpi_load_hwloc_topology(void)
{
    int mpi_errno = MPI_SUCCESS;
    char *kvsname = NULL, *value;
    char *hostname = NULL;
    char *tmppath = NULL;
    int uid, my_local_id;
    int kvs_name_sz;

    MPIR_FUNC_TERSE_STATE_DECL(SMPI_LOAD_HWLOC_TOPOLOGY);
    MPIR_FUNC_TERSE_ENTER(SMPI_LOAD_HWLOC_TOPOLOGY);

    if (topology != NULL) {
        goto fn_exit;
    }

    mpi_errno = hwloc_topology_init(&topology);
#if 1 /* _USE_HWLOC_V2_ */
    hwloc_topology_set_io_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_ALL);
#endif /* _USE_HWLOC_V2_ */
    hwloc_topology_set_flags(topology,
#if 0  /* _USE_HWLOC_V1_ */
                                HWLOC_TOPOLOGY_FLAG_IO_DEVICES |
#endif /* _USE_HWLOC_V1_ */
                             HWLOC_TOPOLOGY_FLAG_IS_THISSYSTEM);

    uid = getuid();
    my_local_id = MPIR_Process.local_rank;

    mpi_errno = UPMI_KVS_GET_NAME_LENGTH_MAX(&kvs_name_sz);
    if (mpi_errno) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                             "**pmi_kvs_get_name_length_max",
                             "**pmi_kvs_get_name_length_max %d", mpi_errno);
    }
    kvsname = (char *)MPL_malloc(kvs_name_sz + 1, MPL_MEM_STRINGS);
    mpi_errno = UPMI_KVS_GET_MY_NAME(kvsname, kvs_name_sz);
    if (mpi_errno) {
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**pmi_kvs_get_my_name",
                             "**pmi_kvs_get_my_name %d", mpi_errno);
    }

    if (my_local_id < 0) {
        if (MPIR_Process.rank == 0) {
            PRINT_ERROR("WARNING! Invalid my_local_id: %d, Disabling hwloc "
                        "topology broadcast\n",
                        my_local_id);
        }
        MVP_BCAST_HWLOC_TOPOLOGY = 0;
    }

    if (!MVP_BCAST_HWLOC_TOPOLOGY) {
        /* Each process loads topology individually */
        mpi_errno = hwloc_topology_load(topology);
        goto fn_exit;
    }

    hostname =
        (char *)MPL_malloc(sizeof(char) * HOSTNAME_LENGTH, MPL_MEM_OTHER);
    tmppath = (char *)MPL_malloc(sizeof(char) * FILENAME_LENGTH, MPL_MEM_OTHER);
    xmlpath = (char *)MPL_malloc(sizeof(char) * FILENAME_LENGTH, MPL_MEM_OTHER);
    if (hostname == NULL || tmppath == NULL || xmlpath == NULL) {
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**nomem",
                                  "**nomem %s", "mvp_hwloc_topology_file");
    }

    if (gethostname(hostname, sizeof(char) * HOSTNAME_LENGTH) < 0) {
        MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail", "%s: %s",
                                  "gethostname", strerror(errno));
    }
    sprintf(tmppath, "/tmp/mvp-hwloc-%s-%s-%d.tmp", kvsname, hostname, uid);
    sprintf(xmlpath, "/tmp/mvp-hwloc-%s-%s-%d.xml", kvsname, hostname, uid);

    /* Local Rank 0 broadcasts topology using xml */
    if (0 == my_local_id) {
        mpi_errno = hwloc_topology_load(topology);
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
#if 0 /* _USE_HWLOC_V1_ */
        mpi_errno = hwloc_topology_export_xml(topology, tmppath);
#else
        mpi_errno = hwloc_topology_export_xml(topology, tmppath, 0);
#endif
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
        if (rename(tmppath, xmlpath) < 0) {
            MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "%s: %s", "rename", strerror(errno));
        }
    } else {
        while (access(xmlpath, F_OK) == -1) {
            usleep(1000);
        }
        mpi_errno = hwloc_topology_set_xml(topology, xmlpath);
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
        mpi_errno = hwloc_topology_load(topology);
        if (mpi_errno) {
            /* MPICH error handlers do not seem to recognize error codes from
             * HWLOC correctly. Set mpi_errno = MPI_ERR_INTERN. */
            mpi_errno = MPI_ERR_INTERN;
            MPIR_ERR_POP(mpi_errno);
        }
    }

fn_exit:
    if (hostname) {
        MPL_free(hostname);
    }
    if (tmppath) {
        MPL_free(tmppath);
    }
    if (kvsname) {
        MPL_free(kvsname);
    }
    MPIR_FUNC_TERSE_EXIT(SMPI_LOAD_HWLOC_TOPOLOGY);
    return mpi_errno;

fn_fail:
    if (MVP_BCAST_HWLOC_TOPOLOGY != 0) {
        PRINT_ERROR("Please retry after setting MVP_BCAST_HWLOC_TOPOLOGY=0\n");
    }
    goto fn_exit;
}

int smpi_unlink_hwloc_topology_file(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_TERSE_STATE_DECL(SMPI_UNLINK_HWLOC_TOPOLOGY_FILE);
    MPIR_FUNC_TERSE_ENTER(SMPI_UNLINK_HWLOC_TOPOLOGY_FILE);

    if (xmlpath) {
        unlink(xmlpath);
    }

    if (whole_topology_xml_path) {
        unlink(whole_topology_xml_path);
    }

    MPIR_FUNC_TERSE_EXIT(SMPI_UNLINK_HWLOC_TOPOLOGY_FILE);
    return mpi_errno;
}

int smpi_destroy_hwloc_topology(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_TERSE_STATE_DECL(SMPI_DESTROY_HWLOC_TOPOLOGY);
    MPIR_FUNC_TERSE_ENTER(SMPI_DESTROY_HWLOC_TOPOLOGY);

    if (topology) {
        hwloc_topology_destroy(topology);
        topology = NULL;
    }

    if (topology_whole) {
        hwloc_topology_destroy(topology_whole);
        topology_whole = NULL;
    }

    if (xmlpath) {
        MPL_free(xmlpath);
    }

    if (whole_topology_xml_path) {
        MPL_free(whole_topology_xml_path);
    }

    MPIR_FUNC_TERSE_EXIT(SMPI_DESTROY_HWLOC_TOPOLOGY);
    return mpi_errno;
}

int smpi_setaffinity(int my_local_id)
{
    int selected_socket = 0;
    int mpi_errno = MPI_SUCCESS;

    hwloc_cpuset_t cpuset;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_SMPI_SETAFFINITY);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_SMPI_SETAFFINITY);

    PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                "my_local_id: %d, mvp_enable_affinity: %d, mvp_binding_level: "
                "%d, MVP_CPU_BINDING_POLICY: %s\n",
                my_local_id, mvp_enable_affinity, mvp_binding_level,
                mvp_cpu_policy_names[MVP_CPU_BINDING_POLICY]);

    if (mvp_enable_affinity > 0) {
        long N_CPUs_online = sysconf(_SC_NPROCESSORS_ONLN);

        if (N_CPUs_online < 1) {
            MPIR_ERR_SETFATALANDJUMP2(mpi_errno, MPI_ERR_OTHER, "**fail",
                                      "%s: %s", "sysconf", strerror(errno));
        }

        mpi_errno = smpi_load_hwloc_topology();
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        cpuset = hwloc_bitmap_alloc();

        /* Call the cpu_mapping function to find out about how the
         * processors are numbered on the different sockets.
         * The hardware information gathered from this function
         * is required to determine the best set of intra-node thresholds.
         * However, since the user has specified a mapping pattern,
         * we are not going to use any of our proposed binding patterns
         */
        mpi_errno = get_cpu_mapping_hwloc(N_CPUs_online, topology);
        if (mpi_errno != MPI_SUCCESS) {
            /* In case, we get an error from the hwloc mapping function */
            mpi_errno = get_cpu_mapping(N_CPUs_online);
        }

        if (s_cpu_mapping) {
            /* If the user has specified how to map the processes, use it */
            char tp_str[s_cpu_mapping_line_max + 1];

            mpi_errno = mvp_get_assigned_cpu_core(
                my_local_id, s_cpu_mapping, s_cpu_mapping_line_max, tp_str);
            if (mpi_errno != 0) {
                fprintf(stderr, "Error parsing CPU mapping string\n");
                mvp_enable_affinity = 0;
                MPL_free(s_cpu_mapping);
                s_cpu_mapping = NULL;
                MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                          "**fail %s",
                                          "Error parsing CPU mapping string");
            }

            // parsing of the string
            char *token = tp_str;
            int cpunum = 0;
            while (*token != '\0') {
                if (isdigit(*token)) {
                    cpunum = first_num_from_str(&token);
                    if (cpunum >= N_CPUs_online) {
                        fprintf(stderr,
                                "Warning! : Core id %d does not exist on this "
                                "architecture! \n",
                                cpunum);
                        fprintf(stderr, "CPU Affinity is undefined \n");
                        mvp_enable_affinity = 0;
                        MPL_free(s_cpu_mapping);
                        MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                                 "**cpuaffinity");
                    }
                    hwloc_bitmap_set(cpuset, cpunum);
                    mvp_my_cpu_id = cpunum;
                    PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                                "Set mvp_my_cpu_id = %d\n", mvp_my_cpu_id);
                } else if (*token == ',') {
                    token++;
                } else if (*token == '-') {
                    token++;
                    if (!isdigit(*token)) {
                        fprintf(stderr,
                                "Warning! : Core id %c does not exist on this "
                                "architecture! \n",
                                *token);
                        fprintf(stderr, "CPU Affinity is undefined \n");
                        mvp_enable_affinity = 0;
                        MPL_free(s_cpu_mapping);
                        MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                                 "**cpuaffinity");
                    } else {
                        int cpuend = first_num_from_str(&token);
                        if (cpuend >= N_CPUs_online || cpuend < cpunum) {
                            fprintf(stderr,
                                    "Warning! : Core id %d does not exist on "
                                    "this architecture! \n",
                                    cpuend);
                            fprintf(stderr, "CPU Affinity is undefined \n");
                            mvp_enable_affinity = 0;
                            MPL_free(s_cpu_mapping);
                            MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                                     "**cpuaffinity");
                        }
                        int cpuval;
                        for (cpuval = cpunum + 1; cpuval <= cpuend; cpuval++)
                            hwloc_bitmap_set(cpuset, cpuval);
                    }
                } else if (*token != '\0') {
                    fprintf(stderr,
                            "Warning! Error parsing the given CPU mask! \n");
                    fprintf(stderr, "CPU Affinity is undefined \n");
                    mvp_enable_affinity = 0;
                    MPL_free(s_cpu_mapping);
                    MPIR_ERR_SETFATALANDJUMP(mpi_errno, MPI_ERR_OTHER,
                                             "**cpuaffinity");
                }
            }
            // then attachment
            hwloc_set_cpubind(topology, cpuset, 0);

            MPL_free(s_cpu_mapping);
            s_cpu_mapping = NULL;
        } else {
            /* The user has not specified how to map the processes,
             * use the data available in /proc/cpuinfo file to decide
             * on the best cpu mapping pattern
             */
            if (mpi_errno != MPI_SUCCESS || custom_cpu_mapping == NULL) {
                /* For some reason, we were not able to retrieve the cpu mapping
                 * information. We are falling back on the linear mapping.
                 * This may not deliver the best performance
                 */
                hwloc_bitmap_only(cpuset, my_local_id % N_CPUs_online);
                mvp_my_cpu_id = (my_local_id % N_CPUs_online);
                PRINT_DEBUG(DEBUG_INIT_verbose > 0, "Set mvp_my_cpu_id = %d\n",
                            mvp_my_cpu_id);
                hwloc_set_cpubind(topology, cpuset, 0);
            } else {
                /*
                 * We have all the information that we need. We will bind the
                 * processes to the cpu's now
                 */
                char tp_str[custom_cpu_mapping_line_max + 1];

                mpi_errno = mvp_get_assigned_cpu_core(
                    my_local_id, custom_cpu_mapping,
                    custom_cpu_mapping_line_max, tp_str);
                if (mpi_errno != 0) {
                    fprintf(stderr, "Error parsing CPU mapping string\n");
                    mvp_enable_affinity = 0;
                    MPIR_ERR_SETFATALANDJUMP1(
                        mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                        "Error parsing CPU mapping string");
                }

                int cores_per_socket = 0;

                if (mvp_binding_level == LEVEL_CORE) {
                    if (
#if defined(CHANNEL_MRAIL)
                        mvp_smp_only ||
#endif
                        mvp_user_defined_mapping) {
                        hwloc_bitmap_only(cpuset, atol(tp_str));
                        mvp_my_cpu_id = atol(tp_str);
                        PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                                    "Set mvp_my_cpu_id = %d\n", mvp_my_cpu_id);
                    } else {
                        hwloc_bitmap_only(
                            cpuset, (atol(tp_str) % cores_per_socket) +
                                        (selected_socket * cores_per_socket));
                        mvp_my_cpu_id = ((atol(tp_str) % cores_per_socket) +
                                         (selected_socket * cores_per_socket));
                        PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                                    "Set mvp_my_cpu_id = %d\n", mvp_my_cpu_id);
                    }
                } else {
                    if (
#if defined(CHANNEL_MRAIL)
                        mvp_smp_only ||
#endif
                        mvp_user_defined_mapping) {
                        hwloc_bitmap_from_ulong(cpuset, atol(tp_str));
                    } else {
                        hwloc_bitmap_from_ulong(
                            cpuset, (atol(tp_str) % cores_per_socket) +
                                        (selected_socket * cores_per_socket));
                    }
                }
                hwloc_set_cpubind(topology, cpuset, 0);
            }

            MPL_free(custom_cpu_mapping);
        }
        /* Free cpuset */
        hwloc_bitmap_free(cpuset);
    }
fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_SMPI_SETAFFINITY);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

int mvp_show_cpu_affinity(int verbosity)
{
    int i = 0, j = 0, num_cpus = 0, my_rank = 0, pg_size = 0;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    char *buf = NULL;
    cpu_set_t *allproc_cpu_set = NULL;
    MPIR_Comm *comm_world = NULL;
    int *mvp_all_numa_id = NULL;
    int *mvp_all_sock_id = NULL;
    int my_node = 0, remote_node = 0;

    comm_world = MPIR_Process.comm_world;
    pg_size = comm_world->local_size;
    my_rank = comm_world->rank;

    mvp_all_numa_id = (int *)MPL_malloc(sizeof(int) * pg_size, MPL_MEM_OTHER);
    mvp_all_sock_id = (int *)MPL_malloc(sizeof(int) * pg_size, MPL_MEM_OTHER);
    allproc_cpu_set =
        (cpu_set_t *)MPL_malloc(sizeof(cpu_set_t) * pg_size, MPL_MEM_OTHER);
    CPU_ZERO(&allproc_cpu_set[my_rank]);
    sched_getaffinity(0, sizeof(cpu_set_t), &allproc_cpu_set[my_rank]);

    mpi_errno =
        MPIR_Allgather_impl(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, allproc_cpu_set,
                            sizeof(cpu_set_t), MPI_BYTE, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        fprintf(stderr, "MPIR_Allgather_impl returned error");
        return mpi_errno;
    }
    /* Gather Numa ID */
    mpi_errno =
        MPIR_Allgather_impl(&mvp_my_numa_id, 1, MPI_INT, mvp_all_numa_id, 1,
                            MPI_INT, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        fprintf(stderr, "MPIR_Allgather_impl returned error");
        return mpi_errno;
    }
    /* Gather Sock ID */
    mpi_errno =
        MPIR_Allgather_impl(&mvp_my_sock_id, 1, MPI_INT, mvp_all_sock_id, 1,
                            MPI_INT, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        fprintf(stderr, "MPIR_Allgather_impl returned error");
        return mpi_errno;
    }
    if (my_rank == 0) {
        char *value;
        value = getenv("OMP_NUM_THREADS");
        num_cpus = sysconf(_SC_NPROCESSORS_CONF);
        fprintf(stderr, "-------------CPU AFFINITY-------------\n");
        fprintf(stderr, "OMP_NUM_THREADS           : %d\n",
                (value != NULL) ? atoi(value) : 0);
        fprintf(stderr, "MVP_THREADS_PER_PROCESS   : %d\n",
                MVP_THREADS_PER_PROCESS);
        fprintf(stderr, "MVP_CPU_BINDING_POLICY    : %s\n",
                mvp_cpu_policy_names[MVP_CPU_BINDING_POLICY]);
        /* hybrid binding policy is only applicable when MVP_CPU_BINDING_POLICY
         * is hybrid */
        if (MVP_CPU_BINDING_POLICY_HYBRID == MVP_CPU_BINDING_POLICY) {
            fprintf(stderr, "MVP_HYBRID_BINDING_POLICY : %s\n",
                    mvp_hybrid_policy_names[MVP_HYBRID_BINDING_POLICY]);
        }
        fprintf(stderr, "--------------------------------------\n");

        buf = (char *)MPL_malloc(sizeof(char) * 6 * num_cpus, MPL_MEM_OTHER);
        MPID_Get_node_id(comm_world, my_rank, &my_node);
        for (i = 0; i < pg_size; i++) {
            MPID_Get_node_id(comm_world, i, &remote_node);
            if (my_node == remote_node || verbosity > 1) {
                MPIR_Memset(buf, 0, sizeof(buf));
                for (j = 0; j < num_cpus; j++) {
                    if (CPU_ISSET(j, &allproc_cpu_set[i])) {
                        sprintf((char *)(buf + strlen(buf)), "%4d", j);
                    }
                }
                fprintf(stderr, "RANK:%2d  CPU_SET: %s; NUMA: %d  Socket: %d\n",
                        i, buf, mvp_all_numa_id[i], mvp_all_sock_id[i]);
            }
        }
        fprintf(stderr, "-------------------------------------\n");
        MPL_free(buf);
    }

    MPL_free(allproc_cpu_set);
    MPL_free(mvp_all_numa_id);
    MPL_free(mvp_all_sock_id);
    return mpi_errno;
}

#if defined(CHANNEL_MRAIL)
int mvp_show_hca_affinity(int verbosity)
{
    int pg_size = 0;
    int my_rank = 0;
    int i = 0, j = 0, k = 0, l = 0;
    int mpi_errno = MPI_SUCCESS;
    int *mvp_all_numa_id = NULL;
    int *mvp_all_sock_id = NULL;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    struct ibv_device **hcas = NULL;

    char *hca_names = NULL;
    char *all_hca_names = NULL;
    MPIDI_MVP_ep_t *vc = NULL;
    MPIR_Comm *comm_world = NULL;

    comm_world = MPIR_Process.comm_world;
    pg_size = comm_world->local_size;
    my_rank = comm_world->rank;

    hcas = mvp_MPIDI_CH3I_RDMA_Process.ib_dev;

    hca_names = (char *)MPL_malloc(
        MAX_NUM_HCAS * (IBV_SYSFS_NAME_MAX + 1) * sizeof(char), MPL_MEM_OTHER);
    mvp_all_numa_id =
        (int *)MPL_malloc(sizeof(int) * pg_size * MAX_NUM_HCAS, MPL_MEM_OTHER);
    mvp_all_sock_id =
        (int *)MPL_malloc(sizeof(int) * pg_size * MAX_NUM_HCAS, MPL_MEM_OTHER);

    k = 0;
    for (i = 0; i < rdma_num_hcas; i++) {
        if (i > 0) {
            strcat(hca_names, " ");
            strcat(hca_names, hcas[i]->name);
        } else {
            strcpy(hca_names, hcas[i]->name);
        }
        PRINT_DEBUG(DEBUG_INIT_verbose > 0, "Adding hcas[%d]->name = %s\n", i,
                    hcas[i]->name);
    }
    strcat(hca_names, ";");

    if (my_rank == 0) {
        all_hca_names =
            (char *)MPL_malloc(strlen(hca_names) * pg_size, MPL_MEM_OTHER);
    }

    PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                "hca_names = %s, strlen(hca_names) = %ld\n", hca_names,
                strlen(hca_names));
    mpi_errno =
        MPIR_Gather_impl(hca_names, strlen(hca_names), MPI_CHAR, all_hca_names,
                         strlen(hca_names), MPI_CHAR, 0, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        fprintf(stderr, "MPIR_Gather_impl returned error: %d", mpi_errno);
        return mpi_errno;
    }
    /* Gather Numa ID */
    mpi_errno = MPIR_Allgather_impl(mvp_selected_ib_hca_numa_info, MAX_NUM_HCAS,
                                    MPI_INT, mvp_all_numa_id, MAX_NUM_HCAS,
                                    MPI_INT, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        fprintf(stderr, "MPIR_Allgather_impl returned error");
        return mpi_errno;
    }
    /* Gather Sock ID */
    mpi_errno = MPIR_Allgather_impl(
        mvp_selected_ib_hca_socket_info, MAX_NUM_HCAS, MPI_INT, mvp_all_sock_id,
        MAX_NUM_HCAS, MPI_INT, comm_world, &errflag);
    if (mpi_errno != MPI_SUCCESS) {
        fprintf(stderr, "MPIR_Allgather_impl returned error");
        return mpi_errno;
    }
    if (my_rank == 0 && all_hca_names != NULL) {
        fprintf(stderr, "-------------HCA AFFINITY-------------\n");
        j = 0;

        char *buffer =
            MPL_malloc((IBV_SYSFS_NAME_MAX + 1) * sizeof(char), MPL_MEM_OTHER);
        for (i = 0; i < pg_size; i++) {
            l = 0;
            MPIDI_Comm_get_vc(comm_world, i, &vc);
            if (vc->smp.local_rank != -1 || verbosity > 1) {
                MPIR_Memset(buffer, 0, sizeof(buffer));
                fprintf(stderr, "RANK: %d :- ", i);
                while (all_hca_names[j] != ';') {
                    k = 0;
                    while ((all_hca_names[j] != ' ') &&
                           (all_hca_names[j] != ';')) {
                        buffer[k] = all_hca_names[j];
                        j++;
                        k++;
                    }
                    buffer[k] = '\0';
                    fprintf(stderr, "HCA-%d: %s, NUMA: %d, Socket: %d; ", l,
                            buffer, mvp_all_numa_id[(i * MAX_NUM_HCAS) + l],
                            mvp_all_sock_id[(i * MAX_NUM_HCAS) + l]);
                    if (all_hca_names[j] == ' ') {
                        j++;
                    }
                    l++;
                }
                fprintf(stderr, "\n");
                j++;
            }
        }
        MPL_free(buffer);

        fprintf(stderr, "-------------------------------------\n");
        MPL_free(all_hca_names);
    }
    MPL_free(hca_names);
    MPL_free(mvp_all_numa_id);
    MPL_free(mvp_all_sock_id);
    return mpi_errno;
}
#endif /* defined(CHANNEL_MRAIL) */

/* helper function to get PU ids of a given socket */
void mvp_get_pu_list_on_socket(hwloc_topology_t topology, hwloc_obj_t obj,
                               int depth, int *pu_ids, int *idx)
{
    int i;
    if (obj->type == HWLOC_OBJ_PU) {
        pu_ids[*idx] = obj->os_index;
        *idx = *idx + 1;
        return;
    }

    for (i = 0; i < obj->arity; i++) {
        mvp_get_pu_list_on_socket(topology, obj->children[i], depth + 1, pu_ids,
                                  idx);
    }

    return;
}

static int mvp_generate_implicit_cpu_mapping(int local_procs,
                                             int num_app_threads)
{
    hwloc_obj_t obj;
    hwloc_cpuset_t allowed_cpuset;

    int i, j, k, l, curr, count, chunk, size, scanned, step, node_offset,
        node_base_pu, index;
    int topodepth, num_physical_cores_per_socket ATTRIBUTE((unused)),
        num_pu_per_socket;
    int num_numanodes, num_pu_per_numanode;
    char mapping[s_cpu_mapping_line_max];

    i = j = k = l = curr = count = chunk = size = scanned = step = node_offset =
        node_base_pu = index = 0;
    count = MVP_PIVOT_CORE_ID;
    
    if (!MVP_ENABLE_AFFINITY) {
        return MPI_SUCCESS;
    }
    /* call optimized topology load */
    if (smpi_load_hwloc_topology()) {
        return MPI_ERR_INTERN;
    }

    num_sockets = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_SOCKET);
    num_numanodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NUMANODE);

    num_physical_cores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
    num_pu = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);

    /* non-socket */
    if (num_sockets == 0) {
        num_pu_per_socket = num_pu;
        num_physical_cores_per_socket = num_physical_cores;
    } else {
        num_pu_per_socket = num_pu / num_sockets;
        num_physical_cores_per_socket = num_physical_cores / num_sockets;
    }

    /* non-NUMA */
    if (num_numanodes == 0)
        num_pu_per_numanode = num_pu;
    else
        num_pu_per_numanode = num_pu / num_numanodes;

    topodepth = hwloc_get_type_depth(topology, HWLOC_OBJ_CORE);
    obj = hwloc_get_obj_by_depth(topology, topodepth, 0); /* check on core 0*/

    /* get allowed cpuset and check number of hw threads on any core e.g., 0 */
    allowed_cpuset = hwloc_bitmap_alloc();
    hwloc_bitmap_zero(allowed_cpuset);
    hwloc_bitmap_and(allowed_cpuset, obj->cpuset,
                     hwloc_topology_get_allowed_cpuset(topology));

    hw_threads_per_core = hwloc_bitmap_weight(allowed_cpuset);

    mvp_core_map = MPL_malloc(sizeof(int) * num_pu, MPL_MEM_OTHER);
    mvp_core_map_per_numa = MPL_malloc(sizeof(int) * num_pu, MPL_MEM_OTHER);

    /* generate core map of the system by scanning the hwloc tree and save it
     *  in mvp_core_map array. All the policies below are core_map aware now */
    topodepth = hwloc_get_type_depth(topology, HWLOC_OBJ_SOCKET);
    for (i = 0; i < num_sockets; i++) {
        obj = hwloc_get_obj_by_depth(topology, topodepth, i);
        mvp_get_pu_list_on_socket(topology, obj, topodepth, mvp_core_map,
                                  &scanned);
    }

    size = scanned;

#if defined(CHANNEL_MRAIL)
    /* If num_physical_cores == local_procs, we can automatically enable
     * mvp_process_placement_aware_hca_mapping */
    if (num_physical_cores == local_procs) {
        mvp_process_placement_aware_hca_mapping = 1;
    }
#endif /*defined(CHANNEL_MRAIL)*/

#ifdef HAVE_CH4_NETMOD_UCX
    if (num_physical_cores * hw_threads_per_core <= local_procs) {
        PRINT_INFO(!MPIR_Process.rank,
                   "WARNING: You appear to be running "
                   "at full subscription for this job. UCX spawns an "
                   "additional thread for each process which may result in "
                   "oversubscribed cores and poor performance. Please "
                   "consider reserving at least 2 cores per node for the "
                   "additional threads, enabling SMT, or setting "
                   "MVP_THREADS_PER_PROCESS=2 to ensure that sufficient "
                   "resources are available.\n\n");
    }
#endif

    switch (MVP_HYBRID_BINDING_POLICY) {
        case MVP_HYBRID_BINDING_POLICY_LINEAR:
            /* Linear mapping: Bind each MPI rank as well as its associated
             * threads to phyical cores. Only use hardware threads when you run
             * out of physical resources  */
            if (num_app_threads == 1 ||
                num_physical_cores >= (local_procs * num_app_threads)) {
                /* one app thread or enough phsyical cores for all threads */
                for (i = 0; i < local_procs; i++) {
                    for (k = 0; k < num_app_threads; k++) {
                        j += snprintf(mapping + j, _POSIX2_LINE_MAX, "%d,",
                                      mvp_core_map[curr]);

                        curr =
                            ((curr + hw_threads_per_core) >= num_pu) ?
                                (curr + hw_threads_per_core + ++step) % num_pu :
                                (curr + hw_threads_per_core) % num_pu;
                    }
                    mapping[--j] = '\0';
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                }
            } else {
                /*
                 * multiple application threads and not enough physical cores
                 * use compact binding scheme
                 */
                for (i = 0; i < local_procs; i++) {
                    curr = count;
                    for (k = 0; k < num_app_threads; k++) {
                        j += snprintf(mapping + j, _POSIX2_LINE_MAX, "%d,",
                                      mvp_core_map[curr]);
                        curr = (curr + 1) % num_pu;
                    }
                    mapping[--j] = '\0';
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                    count = (count + hw_threads_per_core) % num_pu;
                }
            }
            break;
        case MVP_HYBRID_BINDING_POLICY_COMPACT:
            /* Compact mapping: Bind each MPI rank to a single phyical core,
             * and bind its associated threads to the hardware threads of the
             * same physical core. Use first socket followed by the second
             * socket. */
            if (num_app_threads > hw_threads_per_core) {
                PRINT_INFO(
                    (MPIR_Process.rank == 0),
                    "WARNING: COMPACT mapping is "
                    "only meant for hardware multi-threaded "
                    "(hyper-threaded) processors. "
                    "We have detected that your processor does not have "
                    "hyper-threading "
                    "enabled. Note that proceeding with this option on "
                    "current system will cause "
                    "over-subscription, hence leading to severe performance "
                    "degradation. "
                    "We recommend using LINEAR or SPREAD policy for this "
                    "run.\n");
            }

            for (i = 0; i < local_procs; i++) {
                curr = count;
                for (k = 0; k < num_app_threads; k++) {
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, "%d,",
                                  mvp_core_map[curr]);
                    curr = (curr + 1) % num_pu;
                }
                mapping[--j] = '\0';
                j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                count = (count + hw_threads_per_core) % num_pu;
            }
            break;
        case MVP_HYBRID_BINDING_POLICY_SPREAD:
#if defined(CHANNEL_MRAIL)
            /* For spread mapping, we can automatically enable
             * mvp_process_placement_aware_hca_mapping */
            mvp_process_placement_aware_hca_mapping = 1;
#endif /*defined(CHANNEL_MRAIL)*/
            /* Spread mapping: Evenly distributes all the PUs among MPI ranks
             * and
             * ensures that no two MPI ranks get bound to the same physical
             * core. */
            if (num_physical_cores < (local_procs * num_app_threads)) {
                PRINT_INFO((MPIR_Process.rank == 0),
                           "WARNING: This configuration "
                           "might lead to oversubscription of cores !!!\n");
                /* limit the mapping to max available PUs */
                num_physical_cores = num_pu;
            }
            chunk = num_physical_cores / local_procs;

            if (chunk > 1) {
                for (i = 0; i < local_procs; i++) {
                    for (k = curr; k < curr + chunk; k++) {
                        for (l = 0; l < hw_threads_per_core; l++) {
                            j += snprintf(
                                mapping + j, _POSIX2_LINE_MAX, "%d,",
                                mvp_core_map[k * hw_threads_per_core + l]);
                        }
                    }
                    mapping[--j] = '\0';
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                    curr = (curr + chunk) % size;
                }
            } else {
                /*
                 * when MPI ranks are more than half-subscription but less than
                 * full-subcription, instead of following the bunch strategy,
                 * try to spread-out the ranks evenly across all the PUs
                 * available on all the sockets
                 */
                int ranks_per_sock = local_procs / num_sockets;

                curr = 0;
                for (i = 0; i < num_sockets; i++) {
                    for (k = curr; k < curr + ranks_per_sock; k++) {
                        for (l = 0; l < hw_threads_per_core; l++) {
                            j += snprintf(
                                mapping + j, _POSIX2_LINE_MAX, "%d,",
                                mvp_core_map[k * hw_threads_per_core + l]);
                        }
                        mapping[--j] = '\0';
                        j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                    }
                    curr = (curr + ((num_pu_per_socket / hw_threads_per_core) *
                                    chunk)) %
                           size;
                }
            }
            break;
        case MVP_HYBRID_BINDING_POLICY_BUNCH:
            /* Bunch mapping: Bind each MPI rank to a single phyical core of
             * first socket followed by second secket. */
            for (i = 0; i < local_procs; i++) {
                for (l = 0; l < num_app_threads; l++) {
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, "%d,",
                                  mvp_core_map[k]);
                    k = (k + hw_threads_per_core) % size;
                }
                mapping[--j] = '\0';
                j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
            }
            break;
        case MVP_HYBRID_BINDING_POLICY_SCATTER:
#if defined(CHANNEL_MRAIL)
            /* For scatter mapping, we can automatically enable
             * mvp_process_placement_aware_hca_mapping */
            mvp_process_placement_aware_hca_mapping = 1;
#endif /*defined(CHANNEL_MRAIL)*/
            /* scatter mapping: Bind consecutive MPI ranks to different sockets
             * in round-robin fashion */
            if (num_sockets < 2) {
                PRINT_INFO(
                    (MPIR_Process.rank == 0),
                    "WARNING: Scatter is not a valid policy "
                    "for single-socket systems. Please re-run with Bunch "
                    "or any other "
                    "applicable policy\n");
                return MPI_ERR_OTHER;
            }
            for (i = 0; i < local_procs; i++) {
                for (l = 0; l < num_app_threads; l++) {
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, "%d,",
                                  mvp_core_map[node_base_pu + node_offset + l]);
                }
                mapping[--j] = '\0';
                j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                node_base_pu = (node_base_pu + num_pu_per_socket) % size;
                /*
                 * when wrapping around to the first socket, advance the offset
                 * based on hardware threads and number of application threads
                 * in use. For all other sockets nodes, maintain the offset
                 */
                node_offset = (!node_base_pu) ?
                                  (node_offset + hw_threads_per_core) % num_pu :
                                  node_offset;
            }
            break;
        case MVP_HYBRID_BINDING_POLICY_NUMA:
            /* generate core map of the system based on NUMA domains by scanning
             * the hwloc tree and save it in mvp_core_map_per_numa array. NUMA
             * based policies are now map-aware. */
            for (i = 0; i < num_numanodes; i++) {
                /* reset the auxiliary bitmap */
                hwloc_bitmap_zero(allowed_cpuset);

                /* get next numa node object */
                obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NUMANODE, i);

                /* only get the allowed cpuset */
                hwloc_bitmap_and(allowed_cpuset, obj->cpuset,
                                 hwloc_topology_get_allowed_cpuset(topology));

                /* iterate over current node's bitmap and copy to our
                 * placeholder. */
                unsigned int id;
                hwloc_bitmap_foreach_begin(id, allowed_cpuset);
                mvp_core_map_per_numa[index++] = id;
                hwloc_bitmap_foreach_end();
            }

            /* NUMA mapping: Bind consecutive MPI ranks to different NUMA
             * domains in round-robin fashion. */
            for (i = 0; i < local_procs; i++) {
                for (l = 0; l < num_app_threads; l++) {
                    j += snprintf(mapping + j, _POSIX2_LINE_MAX, "%d,",
                                  mvp_core_map[node_base_pu + node_offset + l]);
                }
                mapping[--j] = '\0';
                j += snprintf(mapping + j, _POSIX2_LINE_MAX, ":");
                node_base_pu = (node_base_pu + num_pu_per_numanode) % size;
                /*
                 * when wrapping around to the first numa, advance the offset
                 * based on hardware threads and number of application threads
                 * in use. For all other NUMA nodes, maintain the offset
                 */
                node_offset = (!node_base_pu) ?
                                  (node_offset + hw_threads_per_core) % num_pu :
                                  node_offset;
            }
            break;
        default:
            PRINT_ERROR("WARNING: Undefined hybrid binding policy.\n");
    }

    /* copy the generated mapping string to final mapping*/
    s_cpu_mapping = (char *)MPL_malloc(sizeof(char) * j, MPL_MEM_OTHER);
    strncpy(s_cpu_mapping, mapping, j);
    s_cpu_mapping[j - 1] = '\0';

    if (MPIR_Process.rank == 0) {
        PRINT_DEBUG(DEBUG_INIT_verbose > 0,
                    "num_physical_cores_per_socket %d, "
                    "mapping: %s\n",
                    num_physical_cores_per_socket, s_cpu_mapping);
    }

    /* cleanup */
    MPL_free(mvp_core_map);
    MPL_free(mvp_core_map_per_numa);
    hwloc_bitmap_free(allowed_cpuset);

    return MPI_SUCCESS;
}

int MPIDI_MVP_CH4_set_affinity(int rank, int size)
{
    char *value;
    int mpi_errno = MPI_SUCCESS;
    int my_local_id;
    int num_local_procs;
    long N_CPUs_online;
    mvp_arch_t arch_type;

    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPIDI_MVP_CH4_set_affinity);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_MVP_CH4_set_affinity);

    num_local_procs = MPIR_Process.local_size;

    N_CPUs_online = sysconf(_SC_NPROCESSORS_ONLN);

#if defined(CHANNEL_MRAIL)
    /* The code for detecting DPM support should come before setting process
     * affinity since forcing process to core affinity can lead to
     * oversubscription in a DPM scenario. */
    if (MVP_SUPPORT_DPM) {
        MVP_ENABLE_AFFINITY = 0;
    }
#endif /* defined(CHANNEL_MRAIL)*/

    if (MVP_ENABLE_AFFINITY != -1) {
        mvp_enable_affinity = MVP_ENABLE_AFFINITY;
    }

    arch_type = mvp_get_arch_type();
    /* set CPU_BINDING_POLICY=hybrid for Power, Skylake, Frontera, and KNL */
    if (arch_type == MVP_ARCH_IBM_POWER8 || arch_type == MVP_ARCH_IBM_POWER9 ||
        arch_type == MVP_ARCH_INTEL_XEON_PHI_7250 ||
        arch_type == MVP_ARCH_INTEL_PLATINUM_8170_2S_52 ||
        arch_type == MVP_ARCH_INTEL_PLATINUM_8160_2S_48 ||
        arch_type == MVP_ARCH_INTEL_PLATINUM_8268_2S_48 ||
        arch_type == MVP_ARCH_INTEL_PLATINUM_8280_2S_56 || /* frontera */
        arch_type == MVP_ARCH_AMD_EPYC_7401_48 /* EPYC */ ||
        arch_type == MVP_ARCH_AMD_EPYC_7551_64 /* EPYC */ ||
        arch_type == MVP_ARCH_AMD_EPYC_7742_128 /* rome */) {
        if (!MVP_CVAR_IS_SET_BY_USER(MVP_CPU_BINDING_POLICY)) {
            MVP_CPU_BINDING_POLICY = MVP_CPU_BINDING_POLICY_HYBRID;
        }

        /* if system is Frontera, further force hybrid_binding_policy to spread
         */
        if (arch_type == MVP_ARCH_INTEL_PLATINUM_8280_2S_56 &&
            !MVP_CVAR_IS_SET_BY_USER(MVP_HYBRID_BINDING_POLICY)) {
            MVP_HYBRID_BINDING_POLICY = MVP_HYBRID_BINDING_POLICY_SPREAD;
        }

        /* if CPU is EPYC, further force hybrid_binding_policy to NUMA */
        if ((arch_type == MVP_ARCH_AMD_EPYC_7551_64 ||
             arch_type == MVP_ARCH_AMD_EPYC_7401_48 ||
             arch_type == MVP_ARCH_AMD_EPYC_7742_128 /* rome */) &&
            !MVP_CVAR_IS_SET_BY_USER(MVP_HYBRID_BINDING_POLICY)) {
            MVP_HYBRID_BINDING_POLICY = MVP_HYBRID_BINDING_POLICY_NUMA;
        }
    }

    if (mvp_enable_affinity && (num_local_procs > N_CPUs_online)) {
        if (MPIR_Process.rank == 0) {
            PRINT_ERROR(
                "WARNING: You are running %d MPI processes on a processor "
                "that supports up to %ld cores. If you still wish to run "
                "in oversubscribed mode, please set MVP_ENABLE_AFFINITY=0 "
                "and re-run the program.\n\n",
                num_local_procs, N_CPUs_online);
        }
        MPIR_ERR_SETFATALANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**fail",
                                  "**fail %s",
                                  "MVP_ENABLE_AFFINITY: oversubscribed cores.");
    }

    if (mvp_enable_affinity && MVP_CPU_MAPPING != NULL) {
        /* Affinity is on and the user has supplied a cpu mapping string */
        int linelen = strlen(MVP_CPU_MAPPING);
        if (linelen < s_cpu_mapping_line_max) {
            s_cpu_mapping_line_max = linelen;
        }
        s_cpu_mapping = (char *)MPL_malloc(
            sizeof(char) * (s_cpu_mapping_line_max + 1), MPL_MEM_OTHER);
        strncpy(s_cpu_mapping, MVP_CPU_MAPPING, s_cpu_mapping_line_max);
        s_cpu_mapping[s_cpu_mapping_line_max] = '\0';
        mvp_user_defined_mapping = TRUE;
    }

    if (mvp_enable_affinity && MVP_CPU_MAPPING == NULL) {
        /* Affinity is on and the user has not specified a mapping string */
        if (MVP_CVAR_IS_SET_BY_USER(MVP_CPU_BINDING_POLICY)) {
            /* User has specified a binding policy */
            mvp_user_defined_mapping = TRUE;
        }

        /* configure hybrid binding policy specifics */
        if (MVP_CPU_BINDING_POLICY_HYBRID == MVP_CPU_BINDING_POLICY) {
            /* Check to make sure the user did not set MVP_THREADS_PER_PROCESS
             * to a negative value. */
            if (MVP_THREADS_PER_PROCESS < 0) {
                if (MPIR_Process.rank == 0) {
                    PRINT_ERROR("MVP_THREADS_PER_PROCESS: "
                                "value can not be set to negative.\n");
                    MPIR_ERR_SETFATALANDJUMP1(
                        mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                        "MVP_THREADS_PER_PROCESS: negative value.");
                }
            }

            /* Set if the OMP_NUM_THREADS is exported by the user and it is
             * larger than the MVP_THREADS_PER_PROCESS variable. */
            if ((NULL != (value = getenv("OMP_NUM_THREADS"))) &&
                atoi(value) > MVP_THREADS_PER_PROCESS) {
                MVP_THREADS_PER_PROCESS = atoi(value);
                if (MVP_THREADS_PER_PROCESS < 0) {
                    if (MPIR_Process.rank == 0) {
                        PRINT_ERROR("OMP_NUM_THREADS: value can not be set "
                                    "to negative.\n");
                        MPIR_ERR_SETFATALANDJUMP1(
                            mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                            "OMP_NUM_THREADS: negative value.");
                    }
                }
            }

            if (MVP_THREADS_PER_PROCESS > 0) {
                if ((MVP_THREADS_PER_PROCESS * num_local_procs) >
                    N_CPUs_online) {
                    if (MPIR_Process.rank == 0) {
                        PRINT_ERROR("User defined values for "
                                    "MVP_CPU_BINDING_POLICY and "
                                    "MVP_THREADS_PER_PROCESS will lead to "
                                    "oversubscription of "
                                    "the available CPUs. If this was "
                                    "intentional, please "
                                    "re-run the application after setting "
                                    "MVP_ENABLE_AFFINITY=0 or "
                                    "with explicit CPU mapping using "
                                    "MVP_CPU_MAPPING.\n");
                        MPIR_ERR_SETFATALANDJUMP1(
                            mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                            "CPU_BINDING_PRIMITIVE: over-subscribed hybrid "
                            "configuration.");
                    }
                }

                if (MVP_HYBRID_BINDING_POLICY_NUMA ==
                    MVP_HYBRID_BINDING_POLICY) {
                    /* we only force NUMA binding if we have more than 2
                     * ppn, otherwise we use bunch (linear) mapping */
                    if (2 >= num_local_procs) {
                        MVP_HYBRID_BINDING_POLICY =
                            MVP_HYBRID_BINDING_POLICY_LINEAR;
                    }
                    /* if arch is KNL, disable hybrid NUMA policy and
                     * fallback to spread */
                    if (MVP_ARCH_INTEL_XEON_PHI_7250 == arch_type) {
                        PRINT_INFO((0 == MPIR_Process.rank),
                                   "WARNING: Process "
                                   "mapping mode is being set to NUMA on "
                                   "KNL architecture which "
                                   "is unsupported. We are falling back to "
                                   "'spread' to ensure "
                                   "better performance.\n");
                        MVP_HYBRID_BINDING_POLICY =
                            MVP_HYBRID_BINDING_POLICY_SPREAD;
                    }
                }
                mvp_binding_level = LEVEL_MULTIPLE_CORES;
            } else {
                PRINT_INFO((MPIR_Process.rank == 0),
                           "WARNING: Process mapping "
                           "mode has been set to 'hybrid' "
                           "indicating an attempt to run a multi-threaded "
                           "program. However, "
                           "neither the MVP_THREADS_PER_PROCESS nor "
                           "OMP_NUM_THREADS have been "
                           "set. Please set either one of these variable to "
                           "the number threads "
                           "desired per process for optimal performance\n");
            }
        }
    }

    /* generate implicit mapping string based on hybrid binding policy */
    if (MVP_ENABLE_AFFINITY &&
        MVP_CPU_BINDING_POLICY_HYBRID == MVP_CPU_BINDING_POLICY &&
        !MVP_CPU_MAPPING) {
        mpi_errno = mvp_generate_implicit_cpu_mapping(num_local_procs,
                                                      MVP_THREADS_PER_PROCESS);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
    }

    if (mvp_enable_affinity && MVP_CPU_MAPPING == NULL) {
        /* Affinity is on and the user has not specified a mapping string */
        if (MVP_CPU_BINDING_LEVEL != NULL) {
            /* User has specified a binding level */
            if (!strcmp(MVP_CPU_BINDING_LEVEL, "core") ||
                !strcmp(MVP_CPU_BINDING_LEVEL, "CORE")) {
                mvp_binding_level = LEVEL_CORE;
            } else if (!strcmp(MVP_CPU_BINDING_LEVEL, "socket") ||
                       !strcmp(MVP_CPU_BINDING_LEVEL, "SOCKET")) {
                mvp_binding_level = LEVEL_SOCKET;
            } else if (!strcmp(MVP_CPU_BINDING_LEVEL, "numanode") ||
                       !strcmp(MVP_CPU_BINDING_LEVEL, "NUMANODE")) {
                mvp_binding_level = LEVEL_NUMANODE;
            } else {
                MPIR_ERR_SETFATALANDJUMP1(
                    mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                    "CPU_BINDING_PRIMITIVE: Level should be core, "
                    "socket, or numanode.");
            }
            if (MVP_ARCH_INTEL_XEON_PHI_7250 == arch_type &&
                mvp_binding_level != LEVEL_CORE) {
                MPIR_ERR_SETFATALANDJUMP1(
                    mpi_errno, MPI_ERR_OTHER, "**fail", "**fail %s",
                    "CPU_BINDING_PRIMITIVE: Only core level binding"
                    " supported for this architecture.");
            }
            mvp_user_defined_mapping = TRUE;
        } else {
            /* User has not specified a binding level and we've not
             * assigned LEVEL_MULTIPLE_CORES earlier. We are going to
             * do "core" binding, by default  */
            if (mvp_binding_level != LEVEL_MULTIPLE_CORES) {
                mvp_binding_level = LEVEL_CORE;
            }
        }
    }

    if (mvp_enable_affinity) {
        my_local_id = MPIR_Process.local_rank;
        mpi_errno = smpi_setaffinity(my_local_id);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        mpi_errno = smpi_load_hwloc_topology_whole();
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        /* Find the NUMA domain I am bound to */
        mpi_errno = smpi_identify_my_numa_id();
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        /* Find the socket I am bound to */
        mpi_errno = smpi_identify_my_sock_id();
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
        PRINT_DEBUG(DEBUG_INIT_verbose,
                    "cpu_id = %d, sock_id = %d, numa_id = %d\n", mvp_my_cpu_id,
                    mvp_my_sock_id, mvp_my_numa_id);
    }
fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_MVP_CH4_set_affinity);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

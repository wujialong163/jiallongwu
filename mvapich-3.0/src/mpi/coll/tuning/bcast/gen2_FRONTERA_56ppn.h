#define GEN2__INTEL_PLATINUM_8280_2S_56__MLX_CX_EDR__56PPN                     \
    {                                                                          \
        {                                                                      \
            56, 8192, 4, 4, {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                  \
                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                    \
                21,                                                            \
                {                                                              \
                    {1, &MPIR_Bcast_binomial_MVP},                             \
                    {2, &MPIR_Bcast_scatter_ring_allgather_MVP},               \
                    {4, &MPIR_Bcast_binomial_MVP},                             \
                    {8, &MPIR_Bcast_scatter_ring_allgather_MVP},               \
                    {16, &MPIR_Bcast_binomial_MVP},                            \
                    {32, &MPIR_Bcast_scatter_ring_allgather_MVP},              \
                    {64, &MPIR_Bcast_scatter_ring_allgather_MVP},              \
                    {128, &MPIR_Bcast_scatter_ring_allgather_MVP},             \
                    {256, &MPIR_Bcast_binomial_MVP},                           \
                    {512, &MPIR_Bcast_scatter_ring_allgather_MVP},             \
                    {1024, &MPIR_Bcast_binomial_MVP},                          \
                    {2048, &MPIR_Bcast_binomial_MVP},                          \
                    {4096, &MPIR_Bcast_binomial_MVP},                          \
                    {8192, &MPIR_Bcast_binomial_MVP},                          \
                    {16384, &MPIR_Bcast_binomial_MVP},                         \
                    {32768, &MPIR_Pipelined_Bcast_MVP},                        \
                    {65536, &MPIR_Pipelined_Bcast_MVP},                        \
                    {131072, &MPIR_Pipelined_Bcast_MVP},                       \
                    {262144, &MPIR_Pipelined_Bcast_MVP},                       \
                    {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},      \
                    {1048576, &MPIR_Pipelined_Bcast_MVP},                      \
                },                                                             \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Shmem_Bcast_MVP}, {2, &MPIR_Shmem_Bcast_MVP},        \
                    {4, &MPIR_Shmem_Bcast_MVP}, {8, &MPIR_Shmem_Bcast_MVP},    \
                    {16, &MPIR_Shmem_Bcast_MVP}, {32, &MPIR_Shmem_Bcast_MVP},  \
                    {64, &MPIR_Shmem_Bcast_MVP}, {128, &MPIR_Shmem_Bcast_MVP}, \
                    {256, &MPIR_Shmem_Bcast_MVP},                              \
                    {512, &MPIR_Shmem_Bcast_MVP},                              \
                    {1024, &MPIR_Shmem_Bcast_MVP},                             \
                    {2048, &MPIR_Shmem_Bcast_MVP},                             \
                    {4096, &MPIR_Shmem_Bcast_MVP},                             \
                    {8192, &MPIR_Shmem_Bcast_MVP},                             \
                    {16384, &MPIR_Shmem_Bcast_MVP},                            \
                    {32768, &MPIR_Shmem_Bcast_MVP},                            \
                    {65536, &MPIR_Shmem_Bcast_MVP},                            \
                    {131072, &MPIR_Shmem_Bcast_MVP},                           \
                    {262144, &MPIR_Shmem_Bcast_MVP},                           \
                    {524288, &MPIR_Shmem_Bcast_MVP},                           \
                    {1048576, &MPIR_Shmem_Bcast_MVP},                          \
            }                                                                  \
        }                                                                      \
    }

#define GEN2__INTEL_PLATINUM_8280_2S_56__MLX_CX_EDR__56PPN_CNT 1

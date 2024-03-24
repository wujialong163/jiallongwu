#define GEN2__AMD_EPYC_7401_24__MLX_CX_EDR__16PPN                              \
    {                                                                          \
        {16,                                                                   \
         8192,                                                                 \
         4,                                                                    \
         4,                                                                    \
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},      \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},                  \
             {2, &MPIR_Bcast_binomial_MVP},                                    \
             {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                           \
             {8, &MPIR_Bcast_binomial_MVP},                                    \
             {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                          \
             {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                          \
             {64, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},                 \
             {128, &MPIR_Bcast_scatter_ring_allgather_MVP},                    \
             {256, &MPIR_Bcast_scatter_ring_allgather_MVP},                    \
             {512, &MPIR_Bcast_binomial_MVP},                                  \
             {1024, &MPIR_Bcast_scatter_ring_allgather_MVP},                   \
             {2048, &MPIR_Bcast_scatter_ring_allgather_MVP},                   \
             {4096, &MPIR_Bcast_scatter_ring_allgather_MVP},                   \
             {8192, &MPIR_Bcast_scatter_ring_allgather_MVP},                   \
             {16384, &MPIR_Bcast_scatter_ring_allgather_MVP},                  \
             {32768, &MPIR_Bcast_scatter_ring_allgather_MVP},                  \
             {65536, &MPIR_Bcast_scatter_ring_allgather_MVP},                  \
             {131072, &MPIR_Bcast_scatter_ring_allgather_MVP},                 \
             {262144, &MPIR_Bcast_scatter_ring_allgather_MVP},                 \
             {524288, &MPIR_Bcast_scatter_ring_allgather_MVP},                 \
             {1048576, &MPIR_Bcast_scatter_ring_allgather_MVP},                \
         },                                                                    \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Shmem_Bcast_MVP},                                       \
             {2, &MPIR_Shmem_Bcast_MVP},                                       \
             {4, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                       \
             {8, &MPIR_Shmem_Bcast_MVP},                                       \
             {16, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                      \
             {32, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                      \
             {64, &MPIR_Shmem_Bcast_MVP},                                      \
             {128, &MPIR_Shmem_Bcast_MVP},                                     \
             {256, &MPIR_Shmem_Bcast_MVP},                                     \
             {512, &MPIR_Shmem_Bcast_MVP},                                     \
             {1024, &MPIR_Shmem_Bcast_MVP},                                    \
             {2048, &MPIR_Shmem_Bcast_MVP},                                    \
             {4096, &MPIR_Shmem_Bcast_MVP},                                    \
             {8192, &MPIR_Shmem_Bcast_MVP},                                    \
             {16384, &MPIR_Shmem_Bcast_MVP},                                   \
             {32768, &MPIR_Shmem_Bcast_MVP},                                   \
             {65536, &MPIR_Shmem_Bcast_MVP},                                   \
             {131072, &MPIR_Shmem_Bcast_MVP},                                  \
             {262144, &MPIR_Shmem_Bcast_MVP},                                  \
             {524288, &MPIR_Shmem_Bcast_MVP},                                  \
             {1048576, &MPIR_Shmem_Bcast_MVP},                                 \
         }},                                                                   \
            {32,                                                               \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {2, &MPIR_Bcast_binomial_MVP},                                \
                 {4, &MPIR_Bcast_binomial_MVP},                                \
                 {8, &MPIR_Bcast_binomial_MVP},                                \
                 {16, &MPIR_Bcast_binomial_MVP},                               \
                 {32, &MPIR_Bcast_binomial_MVP},                               \
                 {64, &MPIR_Bcast_binomial_MVP},                               \
                 {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                     \
                 {256, &MPIR_Bcast_binomial_MVP},                              \
                 {512, &MPIR_Bcast_binomial_MVP},                              \
                 {1024, &MPIR_Bcast_binomial_MVP},                             \
                 {2048, &MPIR_Bcast_binomial_MVP},                             \
                 {4096, &MPIR_Bcast_binomial_MVP},                             \
                 {8192, &MPIR_Bcast_binomial_MVP},                             \
                 {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                 {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                   \
                 {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                   \
                 {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {262144, &MPIR_Bcast_binomial_MVP},                           \
                 {524288, &MPIR_Bcast_binomial_MVP},                           \
                 {1048576, &MPIR_Bcast_scatter_ring_allgather_shm_MVP},        \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {2, &MPIR_Shmem_Bcast_MVP},                                   \
                 {4, &MPIR_Shmem_Bcast_MVP},                                   \
                 {8, &MPIR_Shmem_Bcast_MVP},                                   \
                 {16, &MPIR_Shmem_Bcast_MVP},                                  \
                 {32, &MPIR_Shmem_Bcast_MVP},                                  \
                 {64, &MPIR_Shmem_Bcast_MVP},                                  \
                 {128, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                 \
                 {256, &MPIR_Shmem_Bcast_MVP},                                 \
                 {512, &MPIR_Shmem_Bcast_MVP},                                 \
                 {1024, &MPIR_Shmem_Bcast_MVP},                                \
                 {2048, &MPIR_Shmem_Bcast_MVP},                                \
                 {4096, &MPIR_Shmem_Bcast_MVP},                                \
                 {8192, &MPIR_Shmem_Bcast_MVP},                                \
                 {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                 {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 8},               \
                 {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},               \
                 {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},              \
                 {262144, &MPIR_Shmem_Bcast_MVP},                              \
                 {524288, &MPIR_Shmem_Bcast_MVP},                              \
                 {1048576, &MPIR_Knomial_Bcast_intra_node_MVP},                \
             }},                                                               \
            {64,                                                               \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                      \
                 {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {256, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},            \
                 {512, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},            \
                 {1024, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},           \
                 {2048, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},           \
                 {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},           \
                 {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},           \
                 {16384, &MPIR_Pipelined_Bcast_MVP},                           \
                 {32768, &MPIR_Pipelined_Bcast_MVP},                           \
                 {65536, &MPIR_Pipelined_Bcast_MVP},                           \
                 {131072, &MPIR_Pipelined_Bcast_MVP},                          \
                 {262144, &MPIR_Bcast_binomial_MVP},                           \
                 {524288, &MPIR_Bcast_binomial_MVP},                           \
                 {1048576, &MPIR_Bcast_scatter_ring_allgather_shm_MVP},        \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
                 {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
                 {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {32, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                  \
                 {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                 \
                 {256, &MPIR_Shmem_Bcast_MVP},                                 \
                 {512, &MPIR_Shmem_Bcast_MVP},                                 \
                 {1024, &MPIR_Shmem_Bcast_MVP},                                \
                 {2048, &MPIR_Shmem_Bcast_MVP},                                \
                 {4096, &MPIR_Shmem_Bcast_MVP},                                \
                 {8192, &MPIR_Shmem_Bcast_MVP},                                \
                 {16384, &MPIR_Shmem_Bcast_MVP},                               \
                 {32768, &MPIR_Shmem_Bcast_MVP},                               \
                 {65536, &MPIR_Shmem_Bcast_MVP},                               \
                 {131072, &MPIR_Shmem_Bcast_MVP},                              \
                 {262144, &MPIR_Shmem_Bcast_MVP},                              \
                 {524288, &MPIR_Shmem_Bcast_MVP},                              \
                 {1048576, &MPIR_Knomial_Bcast_intra_node_MVP},                \
             }},                                                               \
            {128,                                                              \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                     \
                 {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                     \
                 {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                     \
                 {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                 {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                 {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                 {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},           \
                 {16384, &MPIR_Pipelined_Bcast_MVP},                           \
                 {32768, &MPIR_Pipelined_Bcast_MVP},                           \
                 {65536, &MPIR_Pipelined_Bcast_MVP},                           \
                 {131072, &MPIR_Pipelined_Bcast_MVP},                          \
                 {262144, &MPIR_Pipelined_Bcast_MVP},                          \
                 {524288, &MPIR_Bcast_binomial_MVP},                           \
                 {1048576, &MPIR_Bcast_scatter_ring_allgather_shm_MVP},        \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {128, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                 \
                 {256, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                 \
                 {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                 \
                 {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                 {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                 {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                 {8192, &MPIR_Shmem_Bcast_MVP},                                \
                 {16384, &MPIR_Shmem_Bcast_MVP},                               \
                 {32768, &MPIR_Shmem_Bcast_MVP},                               \
                 {65536, &MPIR_Shmem_Bcast_MVP},                               \
                 {131072, &MPIR_Shmem_Bcast_MVP},                              \
                 {262144, &MPIR_Shmem_Bcast_MVP},                              \
                 {524288, &MPIR_Shmem_Bcast_MVP},                              \
                 {1048576, &MPIR_Knomial_Bcast_intra_node_MVP},                \
             }},                                                               \
        {                                                                      \
            256, 8192, 4, 4, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                 \
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                   \
                21,                                                            \
                {                                                              \
                    {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                    {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                    {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                    {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                    {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                    {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                    {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                    {128, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},         \
                    {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                  \
                    {512, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},         \
                    {1024, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},        \
                    {2048, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},        \
                    {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},        \
                    {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},        \
                    {16384, &MPIR_Pipelined_Bcast_MVP},                        \
                    {32768, &MPIR_Pipelined_Bcast_MVP},                        \
                    {65536, &MPIR_Pipelined_Bcast_MVP},                        \
                    {131072, &MPIR_Pipelined_Bcast_MVP},                       \
                    {262144, &MPIR_Pipelined_Bcast_MVP},                       \
                    {524288, &MPIR_Bcast_binomial_MVP},                        \
                    {1048576, &MPIR_Bcast_scatter_ring_allgather_shm_MVP},     \
                },                                                             \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                    \
                    {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {4, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {8, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {32, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {128, &MPIR_Shmem_Bcast_MVP},                              \
                    {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},              \
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
                    {1048576, &MPIR_Knomial_Bcast_intra_node_MVP},             \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__AMD_EPYC_7401_24__MLX_CX_EDR__16PPN_CNT 5

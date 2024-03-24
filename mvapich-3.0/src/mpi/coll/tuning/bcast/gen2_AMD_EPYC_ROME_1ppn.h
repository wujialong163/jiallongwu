#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__1PPN                              \
    {                                                                          \
        {2,                                                                    \
         8192,                                                                 \
         4,                                                                    \
         4,                                                                    \
         {1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},      \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                           \
             {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                           \
             {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                           \
             {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                           \
             {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                          \
             {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                          \
             {64, &MPIR_Bcast_binomial_MVP},                                   \
             {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                         \
             {256, &MPIR_Bcast_binomial_MVP},                                  \
             {512, &MPIR_Bcast_binomial_MVP},                                  \
             {1024, &MPIR_Bcast_binomial_MVP},                                 \
             {2048, &MPIR_Bcast_binomial_MVP},                                 \
             {4096, &MPIR_Bcast_binomial_MVP},                                 \
             {8192, &MPIR_Bcast_binomial_MVP},                                 \
             {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
             {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
             {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
             {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
             {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
             {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
             {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
         },                                                                    \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                       \
             {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                       \
             {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                       \
             {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                       \
             {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
             {32, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                      \
             {64, &MPIR_Shmem_Bcast_MVP},                                      \
             {128, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                     \
             {256, &MPIR_Shmem_Bcast_MVP},                                     \
             {512, &MPIR_Shmem_Bcast_MVP},                                     \
             {1024, &MPIR_Shmem_Bcast_MVP},                                    \
             {2048, &MPIR_Shmem_Bcast_MVP},                                    \
             {4096, &MPIR_Shmem_Bcast_MVP},                                    \
             {8192, &MPIR_Shmem_Bcast_MVP},                                    \
             {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
             {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
             {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
             {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
             {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
             {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
             {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                 \
         }},                                                                   \
            {4,                                                                \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
                 {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                      \
                 {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                      \
                 {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                 {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                   \
                 {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                 {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                   \
                 {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                 \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
                 {4, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
                 {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
                 {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                  \
                 {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
                 {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                 \
                 {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                 \
                 {512, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                 \
                 {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                 {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                 {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                 {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                 {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 8},               \
                 {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                 {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},               \
                 {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},              \
                 {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},              \
                 {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},              \
                 {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 2},             \
             }},                                                               \
        {                                                                      \
            8, 8192, 4, 4, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   \
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                     \
                21,                                                            \
                {                                                              \
                    {1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                    {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                    {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                    {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                    \
                    {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                   \
                    {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                   \
                    {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                   \
                    {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                  \
                    {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                  \
                    {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                  \
                    {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                 \
                    {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                 \
                    {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                 \
                    {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                 \
                    {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                \
                    {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                \
                    {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                \
                    {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},               \
                    {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},               \
                    {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},               \
                    {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},              \
                },                                                             \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                    \
                    {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                    {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                    {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                \
                    {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},               \
                    {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},               \
                    {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},               \
                    {128, &MPIR_Knomial_Bcast_intra_node_MVP, 8},              \
                    {256, &MPIR_Knomial_Bcast_intra_node_MVP, 8},              \
                    {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},              \
                    {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 8},             \
                    {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},             \
                    {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},             \
                    {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},             \
                    {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 2},            \
                    {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 2},            \
                    {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},            \
                    {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                    {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                    {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                    {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 2},          \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__1PPN_CNT 3
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__1PPN_MAX 8
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__1PPN_CNT 3

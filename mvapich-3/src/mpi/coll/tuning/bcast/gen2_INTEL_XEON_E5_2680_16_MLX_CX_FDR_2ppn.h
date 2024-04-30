#define GEN2__INTEL_XEON_E5_2680_16__MLX_CX_FDR__2PPN                          \
    {{2,                                                                       \
      8192,                                                                    \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},         \
      20,                                                                      \
      {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                                 \
       {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                                \
       {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                               \
       {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                               \
       {1024, &MPIR_Bcast_binomial_MVP, -1},                                   \
       {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                              \
       {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                              \
       {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {16384, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                    \
       {32768, &MPIR_Bcast_binomial_MVP, -1},                                  \
       {65536, &MPIR_Bcast_binomial_MVP, -1},                                  \
       {131072, &MPIR_Bcast_binomial_MVP, -1},                                 \
       {262144, &MPIR_Bcast_binomial_MVP, -1},                                 \
       {524288, &MPIR_Bcast_binomial_MVP, -1},                                 \
       {1048576, &MPIR_Bcast_binomial_MVP, -1}},                               \
      20,                                                                      \
      {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                             \
       {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {8, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {64, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                            \
       {128, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                           \
       {256, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {512, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                           \
       {1024, &MPIR_Shmem_Bcast_MVP, -1},                                      \
       {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                          \
       {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                          \
       {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {16384, &MPIR_Shmem_Bcast_MVP, -1},                                     \
       {32768, &MPIR_Shmem_Bcast_MVP, -1},                                     \
       {65536, &MPIR_Shmem_Bcast_MVP, -1},                                     \
       {131072, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                       \
       {262144, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                       \
       {524288, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                       \
       {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, -1}}},                    \
     {4,                                                                       \
      8192,                                                                    \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},         \
      20,                                                                      \
      {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                             \
       {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                             \
       {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                            \
       {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                            \
       {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                            \
       {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8}},                          \
      20,                                                                      \
      {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {8, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {256, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {512, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                         \
       {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                         \
       {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                        \
       {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                        \
       {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                        \
       {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 8}}},                     \
     {8,                                                                       \
      8192,                                                                    \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},         \
      20,                                                                      \
      {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                             \
       {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2}},                          \
      20,                                                                      \
      {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                         \
       {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 2}}},                     \
     {16,                                                                      \
      8192,                                                                    \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},         \
      20,                                                                      \
      {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                            \
       {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2}},                          \
      20,                                                                      \
      {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {128, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {256, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                        \
       {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 2}}},                     \
     {32,                                                                      \
      8192,                                                                    \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},         \
      20,                                                                      \
      {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                 \
       {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                                \
       {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                               \
       {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                             \
       {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {1048576, &MPIR_Bcast_scatter_ring_allgather_MVP, -1}},                 \
      20,                                                                      \
      {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {4, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {8, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                             \
       {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {32, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                            \
       {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {512, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                           \
       {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                         \
       {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, -1}}},                    \
     {64,                                                                      \
      8192,                                                                    \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},         \
      20,                                                                      \
      {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                 \
       {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                                \
       {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                               \
       {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                              \
       {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                              \
       {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                             \
       {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                            \
       {1048576, &MPIR_Bcast_scatter_ring_allgather_MVP, -1}},                 \
      20,                                                                      \
      {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                             \
       {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                            \
       {128, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {256, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                           \
       {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
       {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
       {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                         \
       {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
       {1048576, &MPIR_Shmem_Bcast_MVP, -1}}}};
#define GEN2__INTEL_XEON_E5_2680_16__MLX_CX_FDR__2PPN_CNT 6

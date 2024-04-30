#define GEN2__INTEL_XEON_E5630_8__MLX_CX_QDR__8PPN                             \
    {{16,                                                                      \
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},         \
      20,                                                                      \
      {{1, &MPIR_Alltoall_RD_MVP},                                             \
       {2, &MPIR_Alltoall_RD_MVP},                                             \
       {4, &MPIR_Alltoall_RD_MVP},                                             \
       {8, &MPIR_Alltoall_RD_MVP},                                             \
       {16, &MPIR_Alltoall_RD_MVP},                                            \
       {32, &MPIR_Alltoall_RD_MVP},                                            \
       {64, &MPIR_Alltoall_bruck_MVP},                                         \
       {128, &MPIR_Alltoall_bruck_MVP},                                        \
       {256, &MPIR_Alltoall_bruck_MVP},                                        \
       {512, &MPIR_Alltoall_bruck_MVP},                                        \
       {1024, &MPIR_Alltoall_Scatter_dest_MVP},                                \
       {2048, &MPIR_Alltoall_Scatter_dest_MVP},                                \
       {4096, &MPIR_Alltoall_Scatter_dest_MVP},                                \
       {8192, &MPIR_Alltoall_pairwise_MVP},                                    \
       {16384, &MPIR_Alltoall_pairwise_MVP},                                   \
       {32768, &MPIR_Alltoall_pairwise_MVP},                                   \
       {65536, &MPIR_Alltoall_pairwise_MVP},                                   \
       {131072, &MPIR_Alltoall_pairwise_MVP},                                  \
       {262144, &MPIR_Alltoall_pairwise_MVP},                                  \
       {524288, &MPIR_Alltoall_pairwise_MVP},                                  \
       {1048576, &MPIR_Alltoall_pairwise_MVP}}},                               \
     {32,                                                                      \
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},         \
      20,                                                                      \
      {{1, &MPIR_Alltoall_RD_MVP},                                             \
       {2, &MPIR_Alltoall_RD_MVP},                                             \
       {4, &MPIR_Alltoall_RD_MVP},                                             \
       {8, &MPIR_Alltoall_RD_MVP},                                             \
       {16, &MPIR_Alltoall_bruck_MVP},                                         \
       {32, &MPIR_Alltoall_bruck_MVP},                                         \
       {64, &MPIR_Alltoall_bruck_MVP},                                         \
       {128, &MPIR_Alltoall_bruck_MVP},                                        \
       {256, &MPIR_Alltoall_bruck_MVP},                                        \
       {512, &MPIR_Alltoall_bruck_MVP},                                        \
       {1024, &MPIR_Alltoall_bruck_MVP},                                       \
       {2048, &MPIR_Alltoall_Scatter_dest_MVP},                                \
       {4096, &MPIR_Alltoall_Scatter_dest_MVP},                                \
       {8192, &MPIR_Alltoall_pairwise_MVP},                                    \
       {16384, &MPIR_Alltoall_pairwise_MVP},                                   \
       {32768, &MPIR_Alltoall_pairwise_MVP},                                   \
       {65536, &MPIR_Alltoall_pairwise_MVP},                                   \
       {131072, &MPIR_Alltoall_pairwise_MVP},                                  \
       {262144, &MPIR_Alltoall_pairwise_MVP},                                  \
       {524288, &MPIR_Alltoall_pairwise_MVP},                                  \
       {1048576, &MPIR_Alltoall_pairwise_MVP}}},                               \
     {64,                                                                      \
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},         \
      20,                                                                      \
      {{1, &MPIR_Alltoall_RD_MVP},                                             \
       {2, &MPIR_Alltoall_RD_MVP},                                             \
       {4, &MPIR_Alltoall_bruck_MVP},                                          \
       {8, &MPIR_Alltoall_bruck_MVP},                                          \
       {16, &MPIR_Alltoall_bruck_MVP},                                         \
       {32, &MPIR_Alltoall_bruck_MVP},                                         \
       {64, &MPIR_Alltoall_bruck_MVP},                                         \
       {128, &MPIR_Alltoall_bruck_MVP},                                        \
       {256, &MPIR_Alltoall_bruck_MVP},                                        \
       {512, &MPIR_Alltoall_bruck_MVP},                                        \
       {1024, &MPIR_Alltoall_bruck_MVP},                                       \
       {2048, &MPIR_Alltoall_pairwise_MVP},                                    \
       {4096, &MPIR_Alltoall_pairwise_MVP},                                    \
       {8192, &MPIR_Alltoall_pairwise_MVP},                                    \
       {16384, &MPIR_Alltoall_pairwise_MVP},                                   \
       {32768, &MPIR_Alltoall_pairwise_MVP},                                   \
       {65536, &MPIR_Alltoall_pairwise_MVP},                                   \
       {131072, &MPIR_Alltoall_pairwise_MVP},                                  \
       {262144, &MPIR_Alltoall_pairwise_MVP},                                  \
       {524288, &MPIR_Alltoall_pairwise_MVP},                                  \
       {1048576, &MPIR_Alltoall_pairwise_MVP}}},                               \
     {128,                                                                     \
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},         \
      20,                                                                      \
      {{1, &MPIR_Alltoall_RD_MVP},                                             \
       {2, &MPIR_Alltoall_bruck_MVP},                                          \
       {4, &MPIR_Alltoall_bruck_MVP},                                          \
       {8, &MPIR_Alltoall_bruck_MVP},                                          \
       {16, &MPIR_Alltoall_bruck_MVP},                                         \
       {32, &MPIR_Alltoall_bruck_MVP},                                         \
       {64, &MPIR_Alltoall_bruck_MVP},                                         \
       {128, &MPIR_Alltoall_bruck_MVP},                                        \
       {256, &MPIR_Alltoall_bruck_MVP},                                        \
       {512, &MPIR_Alltoall_bruck_MVP},                                        \
       {1024, &MPIR_Alltoall_bruck_MVP},                                       \
       {2048, &MPIR_Alltoall_pairwise_MVP},                                    \
       {4096, &MPIR_Alltoall_pairwise_MVP},                                    \
       {8192, &MPIR_Alltoall_pairwise_MVP},                                    \
       {16384, &MPIR_Alltoall_pairwise_MVP},                                   \
       {32768, &MPIR_Alltoall_pairwise_MVP},                                   \
       {65536, &MPIR_Alltoall_pairwise_MVP},                                   \
       {131072, &MPIR_Alltoall_pairwise_MVP},                                  \
       {262144, &MPIR_Alltoall_pairwise_MVP},                                  \
       {524288, &MPIR_Alltoall_pairwise_MVP},                                  \
       {1048576, &MPIR_Alltoall_pairwise_MVP}}},                               \
     {256,                                                                     \
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},         \
      20,                                                                      \
      {{1, &MPIR_Alltoall_bruck_MVP},                                          \
       {2, &MPIR_Alltoall_bruck_MVP},                                          \
       {4, &MPIR_Alltoall_bruck_MVP},                                          \
       {8, &MPIR_Alltoall_bruck_MVP},                                          \
       {16, &MPIR_Alltoall_bruck_MVP},                                         \
       {32, &MPIR_Alltoall_bruck_MVP},                                         \
       {64, &MPIR_Alltoall_bruck_MVP},                                         \
       {128, &MPIR_Alltoall_bruck_MVP},                                        \
       {256, &MPIR_Alltoall_bruck_MVP},                                        \
       {512, &MPIR_Alltoall_bruck_MVP},                                        \
       {1024, &MPIR_Alltoall_bruck_MVP},                                       \
       {2048, &MPIR_Alltoall_pairwise_MVP},                                    \
       {4096, &MPIR_Alltoall_pairwise_MVP},                                    \
       {8192, &MPIR_Alltoall_pairwise_MVP},                                    \
       {16384, &MPIR_Alltoall_pairwise_MVP},                                   \
       {32768, &MPIR_Alltoall_pairwise_MVP},                                   \
       {65536, &MPIR_Alltoall_pairwise_MVP},                                   \
       {131072, &MPIR_Alltoall_pairwise_MVP},                                  \
       {262144, &MPIR_Alltoall_pairwise_MVP},                                  \
       {524288, &MPIR_Alltoall_pairwise_MVP},                                  \
       {1048576, &MPIR_Alltoall_pairwise_MVP}}}};
#define GEN2__INTEL_XEON_E5630_8__MLX_CX_QDR__8PPN_CNT 5

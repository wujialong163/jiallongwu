#define GEN2__AMD_OPTERON_6136_32__MLX_CX_QDR__16PPN                           \
    {{8,                                                                       \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1},               \
      18,                                                                      \
      {{1, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {2, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {4, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {8, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {16, &MPIR_Reduce_inter_knomial_wrapper_MVP},                           \
       {32, &MPIR_Reduce_inter_knomial_wrapper_MVP},                           \
       {64, &MPIR_Reduce_inter_knomial_wrapper_MVP},                           \
       {128, &MPIR_Reduce_inter_knomial_wrapper_MVP},                          \
       {256, &MPIR_Reduce_redscat_gather_MVP},                                 \
       {512, &MPIR_Reduce_binomial_MVP},                                       \
       {1024, &MPIR_Reduce_redscat_gather_MVP},                                \
       {2048, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {4096, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {8192, &MPIR_Reduce_binomial_MVP},                                      \
       {16384, &MPIR_Reduce_binomial_MVP},                                     \
       {32768, &MPIR_Reduce_binomial_MVP},                                     \
       {65536, &MPIR_Reduce_inter_knomial_wrapper_MVP},                        \
       {131072, &MPIR_Reduce_inter_knomial_wrapper_MVP},                       \
       {262144, &MPIR_Reduce_binomial_MVP}},                                   \
      18,                                                                      \
      {{1, &MPIR_Reduce_shmem_MVP},                                            \
       {2, &MPIR_Reduce_intra_knomial_wrapper_MVP},                            \
       {4, &MPIR_Reduce_binomial_MVP},                                         \
       {8, &MPIR_Reduce_intra_knomial_wrapper_MVP},                            \
       {16, &MPIR_Reduce_binomial_MVP},                                        \
       {32, &MPIR_Reduce_intra_knomial_wrapper_MVP},                           \
       {64, &MPIR_Reduce_intra_knomial_wrapper_MVP},                           \
       {128, &MPIR_Reduce_binomial_MVP},                                       \
       {256, &MPIR_Reduce_binomial_MVP},                                       \
       {512, &MPIR_Reduce_binomial_MVP},                                       \
       {1024, &MPIR_Reduce_binomial_MVP},                                      \
       {2048, &MPIR_Reduce_binomial_MVP},                                      \
       {4096, &MPIR_Reduce_shmem_MVP},                                         \
       {8192, &MPIR_Reduce_shmem_MVP},                                         \
       {16384, &MPIR_Reduce_shmem_MVP},                                        \
       {32768, &MPIR_Reduce_shmem_MVP},                                        \
       {65536, &MPIR_Reduce_shmem_MVP},                                        \
       {131072, &MPIR_Reduce_shmem_MVP},                                       \
       {262144, &MPIR_Reduce_shmem_MVP}}},                                     \
                                                                               \
     {16,                                                                      \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1},               \
      18,                                                                      \
      {{1, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {2, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {4, &MPIR_Reduce_redscat_gather_MVP},                                   \
       {8, &MPIR_Reduce_inter_knomial_wrapper_MVP},                            \
       {16, &MPIR_Reduce_binomial_MVP},                                        \
       {32, &MPIR_Reduce_inter_knomial_wrapper_MVP},                           \
       {64, &MPIR_Reduce_inter_knomial_wrapper_MVP},                           \
       {128, &MPIR_Reduce_redscat_gather_MVP},                                 \
       {256, &MPIR_Reduce_inter_knomial_wrapper_MVP},                          \
       {512, &MPIR_Reduce_inter_knomial_wrapper_MVP},                          \
       {1024, &MPIR_Reduce_binomial_MVP},                                      \
       {2048, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {4096, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {8192, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {16384, &MPIR_Reduce_binomial_MVP},                                     \
       {32768, &MPIR_Reduce_inter_knomial_wrapper_MVP},                        \
       {65536, &MPIR_Reduce_inter_knomial_wrapper_MVP},                        \
       {131072, &MPIR_Reduce_inter_knomial_wrapper_MVP},                       \
       {262144, &MPIR_Reduce_binomial_MVP}},                                   \
      18,                                                                      \
      {{1, &MPIR_Reduce_binomial_MVP},                                         \
       {2, &MPIR_Reduce_intra_knomial_wrapper_MVP},                            \
       {4, &MPIR_Reduce_shmem_MVP},                                            \
       {8, &MPIR_Reduce_shmem_MVP},                                            \
       {16, &MPIR_Reduce_shmem_MVP},                                           \
       {32, &MPIR_Reduce_intra_knomial_wrapper_MVP},                           \
       {64, &MPIR_Reduce_shmem_MVP},                                           \
       {128, &MPIR_Reduce_binomial_MVP},                                       \
       {256, &MPIR_Reduce_shmem_MVP},                                          \
       {512, &MPIR_Reduce_shmem_MVP},                                          \
       {1024, &MPIR_Reduce_binomial_MVP},                                      \
       {2048, &MPIR_Reduce_shmem_MVP},                                         \
       {4096, &MPIR_Reduce_shmem_MVP},                                         \
       {8192, &MPIR_Reduce_shmem_MVP},                                         \
       {16384, &MPIR_Reduce_shmem_MVP},                                        \
       {32768, &MPIR_Reduce_shmem_MVP},                                        \
       {65536, &MPIR_Reduce_shmem_MVP},                                        \
       {131072, &MPIR_Reduce_shmem_MVP},                                       \
       {262144, &MPIR_Reduce_shmem_MVP}}},                                     \
                                                                               \
     {32,                                                                      \
      4,                                                                       \
      4,                                                                       \
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1},               \
      18,                                                                      \
      {{1, &MPIR_Reduce_binomial_MVP},                                         \
       {2, &MPIR_Reduce_redscat_gather_MVP},                                   \
       {4, &MPIR_Reduce_redscat_gather_MVP},                                   \
       {8, &MPIR_Reduce_redscat_gather_MVP},                                   \
       {16, &MPIR_Reduce_redscat_gather_MVP},                                  \
       {32, &MPIR_Reduce_redscat_gather_MVP},                                  \
       {64, &MPIR_Reduce_inter_knomial_wrapper_MVP},                           \
       {128, &MPIR_Reduce_redscat_gather_MVP},                                 \
       {256, &MPIR_Reduce_redscat_gather_MVP},                                 \
       {512, &MPIR_Reduce_redscat_gather_MVP},                                 \
       {1024, &MPIR_Reduce_redscat_gather_MVP},                                \
       {2048, &MPIR_Reduce_redscat_gather_MVP},                                \
       {4096, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {8192, &MPIR_Reduce_inter_knomial_wrapper_MVP},                         \
       {16384, &MPIR_Reduce_inter_knomial_wrapper_MVP},                        \
       {32768, &MPIR_Reduce_inter_knomial_wrapper_MVP},                        \
       {65536, &MPIR_Reduce_inter_knomial_wrapper_MVP},                        \
       {131072, &MPIR_Reduce_inter_knomial_wrapper_MVP},                       \
       {262144, &MPIR_Reduce_redscat_gather_MVP}},                             \
      18,                                                                      \
      {{1, &MPIR_Reduce_binomial_MVP},                                         \
       {2, &MPIR_Reduce_binomial_MVP},                                         \
       {4, &MPIR_Reduce_binomial_MVP},                                         \
       {8, &MPIR_Reduce_shmem_MVP},                                            \
       {16, &MPIR_Reduce_binomial_MVP},                                        \
       {32, &MPIR_Reduce_shmem_MVP},                                           \
       {64, &MPIR_Reduce_shmem_MVP},                                           \
       {128, &MPIR_Reduce_shmem_MVP},                                          \
       {256, &MPIR_Reduce_shmem_MVP},                                          \
       {512, &MPIR_Reduce_shmem_MVP},                                          \
       {1024, &MPIR_Reduce_shmem_MVP},                                         \
       {2048, &MPIR_Reduce_shmem_MVP},                                         \
       {4096, &MPIR_Reduce_shmem_MVP},                                         \
       {8192, &MPIR_Reduce_shmem_MVP},                                         \
       {16384, &MPIR_Reduce_shmem_MVP},                                        \
       {32768, &MPIR_Reduce_shmem_MVP},                                        \
       {65536, &MPIR_Reduce_shmem_MVP},                                        \
       {131072, &MPIR_Reduce_shmem_MVP},                                       \
       {262144, &MPIR_Reduce_shmem_MVP}}}};
#define GEN2__AMD_OPTERON_6136_32__MLX_CX_QDR__16PPN_CNT 3

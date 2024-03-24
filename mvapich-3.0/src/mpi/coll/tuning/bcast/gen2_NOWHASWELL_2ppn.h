#define GEN2__INTEL_XEON_E5_2687W_V3_2S_20__MLX_CX_HDR__2PPN                   \
    {                                                                          \
        {2,                                                                    \
         8192,                                                                 \
         4,                                                                    \
         4,                                                                    \
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1},      \
         21,                                                                   \
         {{1, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {2, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {4, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {8, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {16, &MPIR_Bcast_binomial_MVP, -1},                                  \
          {32, &MPIR_Bcast_binomial_MVP, -1},                                  \
          {64, &MPIR_Bcast_binomial_MVP, -1},                                  \
          {128, &MPIR_Bcast_binomial_MVP, -1},                                 \
          {256, &MPIR_Bcast_binomial_MVP, -1},                                 \
          {512, &MPIR_Bcast_binomial_MVP, -1},                                 \
          {1024, &MPIR_Bcast_binomial_MVP, -1},                                \
          {2048, &MPIR_Bcast_binomial_MVP, -1},                                \
          {4096, &MPIR_Bcast_binomial_MVP, -1},                                \
          {8192, &MPIR_Bcast_binomial_MVP, -1},                                \
          {16384, &MPIR_Bcast_binomial_MVP, -1},                               \
          {32768, &MPIR_Bcast_binomial_MVP, -1},                               \
          {65536, &MPIR_Bcast_binomial_MVP, -1},                               \
          {131072, &MPIR_Bcast_binomial_MVP, -1},                              \
          {262144, &MPIR_Bcast_binomial_MVP, -1},                              \
          {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},            \
          {1048576, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1}},          \
         21,                                                                   \
         {{1, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {2, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {4, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {8, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {16, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {32, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {64, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {128, &MPIR_Shmem_Bcast_MVP, -1},                                    \
          {256, &MPIR_Shmem_Bcast_MVP, -1},                                    \
          {512, &MPIR_Shmem_Bcast_MVP, -1},                                    \
          {1024, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {2048, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {4096, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {8192, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {16384, &MPIR_Shmem_Bcast_MVP, -1},                                  \
          {32768, &MPIR_Shmem_Bcast_MVP, -1},                                  \
          {65536, &MPIR_Shmem_Bcast_MVP, -1},                                  \
          {131072, &MPIR_Shmem_Bcast_MVP, -1},                                 \
          {262144, &MPIR_Shmem_Bcast_MVP, -1},                                 \
          {524288, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                    \
          {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, -1}}},                 \
            {4,                                                                \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                          \
              {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                          \
              {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                          \
              {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                          \
              {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                         \
              {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                         \
              {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                         \
              {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                        \
              {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                        \
              {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                        \
              {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                       \
              {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                       \
              {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
              {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
              {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                      \
              {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                      \
              {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                      \
              {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                     \
              {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                     \
              {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 8},                     \
              {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4}},                   \
             21,                                                               \
             {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                      \
              {4, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                      \
              {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {16, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                     \
              {32, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                     \
              {64, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                     \
              {128, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                    \
              {256, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                    \
              {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                    \
              {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                   \
              {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
              {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
              {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                  \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                  \
              {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                  \
              {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                 \
              {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                 \
              {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                 \
              {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 4}}},              \
    }
#define GEN2__INTEL_XEON_E5_2687W_V3_2S_20__MLX_CX_HDR__2PPN_CNT 2
#define GEN2__INTEL_XEON_E5_2687W_V3_2S_20__MLX_CX_HDR__2PPN_MAX 4
#define GEN2__INTEL_XEON_E5_2687W_V3_2S_20__MLX_CX_HDR__2PPN_CNT 2

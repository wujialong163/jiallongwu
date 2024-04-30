#define GEN2__ARM_CAVIUM_V8_2S_28__MLX_CX_EDR__56PPN                           \
    {                                                                          \
        {56,                                                                   \
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},                     \
         16,                                                                   \
         {{1, &MPIR_Alltoall_RD_MVP},                                          \
          {2, &MPIR_Alltoall_RD_MVP},                                          \
          {4, &MPIR_Alltoall_RD_MVP},                                          \
          {8, &MPIR_Alltoall_bruck_MVP},                                       \
          {16, &MPIR_Alltoall_bruck_MVP},                                      \
          {32, &MPIR_Alltoall_bruck_MVP},                                      \
          {64, &MPIR_Alltoall_bruck_MVP},                                      \
          {128, &MPIR_Alltoall_bruck_MVP},                                     \
          {256, &MPIR_Alltoall_bruck_MVP},                                     \
          {512, &MPIR_Alltoall_Scatter_dest_MVP},                              \
          {1024, &MPIR_Alltoall_Scatter_dest_MVP},                             \
          {2048, &MPIR_Alltoall_Scatter_dest_MVP},                             \
          {4096, &MPIR_Alltoall_Scatter_dest_MVP},                             \
          {8192, &MPIR_Alltoall_Scatter_dest_MVP},                             \
          {16384, &MPIR_Alltoall_Scatter_dest_MVP},                            \
          {32768, &MPIR_Alltoall_Scatter_dest_MVP}}},                          \
            {112,                                                              \
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 \
             16,                                                               \
             {{1, &MPIR_Alltoall_bruck_MVP},                                   \
              {2, &MPIR_Alltoall_bruck_MVP},                                   \
              {4, &MPIR_Alltoall_bruck_MVP},                                   \
              {8, &MPIR_Alltoall_bruck_MVP},                                   \
              {16, &MPIR_Alltoall_bruck_MVP},                                  \
              {32, &MPIR_Alltoall_bruck_MVP},                                  \
              {64, &MPIR_Alltoall_bruck_MVP},                                  \
              {128, &MPIR_Alltoall_bruck_MVP},                                 \
              {256, &MPIR_Alltoall_bruck_MVP},                                 \
              {512, &MPIR_Alltoall_bruck_MVP},                                 \
              {1024, &MPIR_Alltoall_bruck_MVP},                                \
              {2048, &MPIR_Alltoall_bruck_MVP},                                \
              {4096, &MPIR_Alltoall_pairwise_MVP},                             \
              {8192, &MPIR_Alltoall_pairwise_MVP},                             \
              {16384, &MPIR_Alltoall_pairwise_MVP},                            \
              {32768, &MPIR_Alltoall_pairwise_MVP}}},                          \
            {224,                                                              \
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 \
             16,                                                               \
             {{1, &MPIR_Alltoall_bruck_MVP},                                   \
              {2, &MPIR_Alltoall_bruck_MVP},                                   \
              {4, &MPIR_Alltoall_bruck_MVP},                                   \
              {8, &MPIR_Alltoall_bruck_MVP},                                   \
              {16, &MPIR_Alltoall_bruck_MVP},                                  \
              {32, &MPIR_Alltoall_bruck_MVP},                                  \
              {64, &MPIR_Alltoall_bruck_MVP},                                  \
              {128, &MPIR_Alltoall_bruck_MVP},                                 \
              {256, &MPIR_Alltoall_bruck_MVP},                                 \
              {512, &MPIR_Alltoall_bruck_MVP},                                 \
              {1024, &MPIR_Alltoall_bruck_MVP},                                \
              {2048, &MPIR_Alltoall_bruck_MVP},                                \
              {4096, &MPIR_Alltoall_bruck_MVP},                                \
              {8192, &MPIR_Alltoall_pairwise_MVP},                             \
              {16384, &MPIR_Alltoall_pairwise_MVP},                            \
              {32768, &MPIR_Alltoall_pairwise_MVP}}},                          \
        {                                                                      \
            448, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 16,         \
            {                                                                  \
                {1, &MPIR_Alltoall_bruck_MVP}, {2, &MPIR_Alltoall_bruck_MVP},  \
                    {4, &MPIR_Alltoall_bruck_MVP},                             \
                    {8, &MPIR_Alltoall_bruck_MVP},                             \
                    {16, &MPIR_Alltoall_bruck_MVP},                            \
                    {32, &MPIR_Alltoall_bruck_MVP},                            \
                    {64, &MPIR_Alltoall_bruck_MVP},                            \
                    {128, &MPIR_Alltoall_bruck_MVP},                           \
                    {256, &MPIR_Alltoall_bruck_MVP},                           \
                    {512, &MPIR_Alltoall_bruck_MVP},                           \
                    {1024, &MPIR_Alltoall_bruck_MVP},                          \
                    {2048, &MPIR_Alltoall_bruck_MVP},                          \
                    {4096, &MPIR_Alltoall_bruck_MVP},                          \
                    {8192, &MPIR_Alltoall_pairwise_MVP},                       \
                    {16384, &MPIR_Alltoall_pairwise_MVP},                      \
                {                                                              \
                    32768, &MPIR_Alltoall_pairwise_MVP                         \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__ARM_CAVIUM_V8_2S_28__MLX_CX_EDR__56PPN_CNT 4
#define GEN2__ARM_CAVIUM_V8_2S_28__MLX_CX_EDR__56PPN_MAX 448
#define GEN2__ARM_CAVIUM_V8_2S_28__MLX_CX_EDR__56PPN_CNT 4

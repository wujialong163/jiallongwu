#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__64PPN                         \
    {                                                                          \
        {64,                                                                   \
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},                     \
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
          {512, &MPIR_Alltoall_bruck_MVP},                                     \
          {1024, &MPIR_Alltoall_bruck_MVP},                                    \
          {2048, &MPIR_Alltoall_Scatter_dest_MVP},                             \
          {4096, &MPIR_Alltoall_pairwise_MVP},                                 \
          {8192, &MPIR_Alltoall_Scatter_dest_MVP},                             \
          {16384, &MPIR_Alltoall_Scatter_dest_MVP},                            \
          {32768, &MPIR_Alltoall_Scatter_dest_MVP}}},                          \
            {128,                                                              \
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},                 \
             16,                                                               \
             {{1, &MPIR_Alltoall_RD_MVP},                                      \
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
              {2048, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {4096, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {8192, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {16384, &MPIR_Alltoall_Scatter_dest_MVP},                        \
              {32768, &MPIR_Alltoall_Scatter_dest_MVP}}},                      \
            {256,                                                              \
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},                 \
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
              {1024, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {2048, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {4096, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {8192, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {16384, &MPIR_Alltoall_Scatter_dest_MVP},                        \
              {32768, &MPIR_Alltoall_Scatter_dest_MVP}}},                      \
            {512,                                                              \
             {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1},                 \
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
              {512, &MPIR_Alltoall_Scatter_dest_MVP},                          \
              {1024, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {2048, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {4096, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {8192, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {16384, &MPIR_Alltoall_Scatter_dest_MVP},                        \
              {32768, &MPIR_Alltoall_pairwise_MVP}}},                          \
            {1024,                                                             \
             {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0},                 \
             16,                                                               \
             {{1, &MPIR_Alltoall_pairwise_MVP},                                \
              {2, &MPIR_Alltoall_pairwise_MVP},                                \
              {4, &MPIR_Alltoall_RD_MVP},                                      \
              {8, &MPIR_Alltoall_inplace_MVP},                                 \
              {16, &MPIR_Alltoall_pairwise_MVP},                               \
              {32, &MPIR_Alltoall_pairwise_MVP},                               \
              {64, &MPIR_Alltoall_inplace_MVP},                                \
              {128, &MPIR_Alltoall_Scatter_dest_MVP},                          \
              {256, &MPIR_Alltoall_pairwise_MVP},                              \
              {512, &MPIR_Alltoall_Scatter_dest_MVP},                          \
              {1024, &MPIR_Alltoall_bruck_MVP},                                \
              {2048, &MPIR_Alltoall_pairwise_MVP},                             \
              {4096, &MPIR_Alltoall_Scatter_dest_MVP},                         \
              {8192, &MPIR_Alltoall_pairwise_MVP},                             \
              {16384, &MPIR_Alltoall_inplace_MVP},                             \
              {32768, &MPIR_Alltoall_inplace_MVP}}},                           \
        {                                                                      \
            2048, {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}, 16,        \
            {                                                                  \
                {1, &MPIR_Alltoall_pairwise_MVP},                              \
                    {2, &MPIR_Alltoall_pairwise_MVP},                          \
                    {4, &MPIR_Alltoall_inplace_MVP},                           \
                    {8, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                    {16, &MPIR_Alltoall_bruck_MVP},                            \
                    {32, &MPIR_Alltoall_bruck_MVP},                            \
                    {64, &MPIR_Alltoall_pairwise_MVP},                         \
                    {128, &MPIR_Alltoall_pairwise_MVP},                        \
                    {256, &MPIR_Alltoall_Scatter_dest_MVP},                    \
                    {512, &MPIR_Alltoall_bruck_MVP},                           \
                    {1024, &MPIR_Alltoall_bruck_MVP},                          \
                    {2048, &MPIR_Alltoall_pairwise_MVP},                       \
                    {4096, &MPIR_Alltoall_inplace_MVP},                        \
                    {8192, &MPIR_Alltoall_pairwise_MVP},                       \
                    {16384, &MPIR_Alltoall_pairwise_MVP},                      \
                {                                                              \
                    32768, &MPIR_Alltoall_Scatter_dest_MVP                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__64PPN_CNT 6

#define test_table                                                             \
    {                                                                          \
        {8,                                                                    \
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                     \
         16,                                                                   \
         {{1, &MPIR_Alltoallv_intra_scatter_MVP},                              \
          {2, &MPIR_Alltoallv_intra_scatter_MVP},                              \
          {4, &MPIR_Alltoallv_intra_scatter_MVP},                              \
          {8, &MPIR_Alltoallv_intra_scatter_MVP},                              \
          {16, &MPIR_Alltoallv_intra_scatter_MVP},                             \
          {32, &MPIR_Alltoallv_intra_scatter_MVP},                             \
          {64, &MPIR_Alltoallv_intra_scatter_MVP},                             \
          {128, &MPIR_Alltoallv_intra_scatter_MVP},                            \
          {256, &MPIR_Alltoallv_intra_scatter_MVP},                            \
          {512, &MPIR_Alltoallv_intra_scatter_MVP},                            \
          {1024, &MPIR_Alltoallv_intra_scatter_MVP},                           \
          {2048, &MPIR_Alltoallv_intra_scatter_MVP},                           \
          {4096, &MPIR_Alltoallv_intra_scatter_MVP},                           \
          {8192, &MPIR_Alltoallv_intra_scatter_MVP},                           \
          {16384, &MPIR_Alltoallv_intra_scatter_MVP},                          \
          {32768, &MPIR_Alltoallv_intra_scatter_MVP}}},                        \
        {                                                                      \
            16, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 16,          \
            {                                                                  \
                {1, &MPIR_Alltoallv_intra_MVP},                                \
                    {2, &MPIR_Alltoallv_intra_MVP},                            \
                    {4, &MPIR_Alltoallv_intra_MVP},                            \
                    {8, &MPIR_Alltoallv_intra_MVP},                            \
                    {16, &MPIR_Alltoallv_intra_MVP},                           \
                    {32, &MPIR_Alltoallv_intra_MVP},                           \
                    {64, &MPIR_Alltoallv_intra_MVP},                           \
                    {128, &MPIR_Alltoallv_intra_MVP},                          \
                    {256, &MPIR_Alltoallv_intra_MVP},                          \
                    {512, &MPIR_Alltoallv_intra_MVP},                          \
                    {1024, &MPIR_Alltoallv_intra_MVP},                         \
                    {2048, &MPIR_Alltoallv_intra_MVP},                         \
                    {4096, &MPIR_Alltoallv_intra_MVP},                         \
                    {8192, &MPIR_Alltoallv_intra_MVP},                         \
                    {16384, &MPIR_Alltoallv_intra_MVP},                        \
                {                                                              \
                    32768, &MPIR_Alltoallv_intra_MVP                           \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define test_table_CNT 2

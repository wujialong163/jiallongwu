#define GEN2__ARM_GRACE_2S_144__MLX_CX_NDR__144PPN                             \
    {                                                                          \
        {144,                                                                  \
         {                                                                     \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
             0,                                                                \
         },                                                                    \
         16,                                                                   \
         {                                                                     \
             {1, &MPIR_Alltoall_bruck_MVP},                                    \
             {2, &MPIR_Alltoall_RD_MVP},                                       \
             {4, &MPIR_Alltoall_RD_MVP},                                       \
             {8, &MPIR_Alltoall_bruck_MVP},                                    \
             {16, &MPIR_Alltoall_bruck_MVP},                                   \
             {32, &MPIR_Alltoall_bruck_MVP},                                   \
             {64, &MPIR_Alltoall_Scatter_dest_MVP},                            \
             {128, &MPIR_Alltoall_Scatter_dest_MVP},                           \
             {256, &MPIR_Alltoall_Scatter_dest_MVP},                           \
             {512, &MPIR_Alltoall_Scatter_dest_MVP},                           \
             {1024, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {2048, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {4096, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {8192, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {16384, &MPIR_Alltoall_pairwise_MVP},                             \
             {32768, &MPIR_Alltoall_Scatter_dest_MVP},                         \
         }},                                                                   \
        {                                                                      \
            288,                                                               \
                {                                                              \
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,            \
                },                                                             \
                16,                                                            \
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
                    {1024, &MPIR_Alltoall_pairwise_MVP},                       \
                    {2048, &MPIR_Alltoall_inplace_MVP},                        \
                    {4096, &MPIR_Alltoall_inplace_MVP},                        \
                    {8192, &MPIR_Alltoall_inplace_MVP},                        \
                    {16384, &MPIR_Alltoall_inplace_MVP},                       \
                    {32768, &MPIR_Alltoall_inplace_MVP},                       \
            }                                                                  \
        }                                                                      \
    }

#define GEN2__ARM_GRACE_2S_144__MLX_CX_NDR__144PPN_CNT 2

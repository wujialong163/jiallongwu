#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__4PPN                              \
    {                                                                          \
        {4,                                                                    \
         {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},                     \
         16,                                                                   \
         {                                                                     \
             {1, &MPIR_Alltoall_RD_MVP},                                       \
             {2, &MPIR_Alltoall_RD_MVP},                                       \
             {4, &MPIR_Alltoall_RD_MVP},                                       \
             {8, &MPIR_Alltoall_Scatter_dest_MVP},                             \
             {16, &MPIR_Alltoall_Scatter_dest_MVP},                            \
             {32, &MPIR_Alltoall_Scatter_dest_MVP},                            \
             {64, &MPIR_Alltoall_Scatter_dest_MVP},                            \
             {128, &MPIR_Alltoall_Scatter_dest_MVP},                           \
             {256, &MPIR_Alltoall_Scatter_dest_MVP},                           \
             {512, &MPIR_Alltoall_Scatter_dest_MVP},                           \
             {1024, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {2048, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {4096, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {8192, &MPIR_Alltoall_Scatter_dest_MVP},                          \
             {16384, &MPIR_Alltoall_RD_MVP},                                   \
             {32768, &MPIR_Alltoall_Scatter_dest_MVP},                         \
         }},                                                                   \
            {8,                                                                \
             {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                 \
             16,                                                               \
             {                                                                 \
                 {1, &MPIR_Alltoall_RD_MVP},                                   \
                 {2, &MPIR_Alltoall_RD_MVP},                                   \
                 {4, &MPIR_Alltoall_RD_MVP},                                   \
                 {8, &MPIR_Alltoall_RD_MVP},                                   \
                 {16, &MPIR_Alltoall_RD_MVP},                                  \
                 {32, &MPIR_Alltoall_RD_MVP},                                  \
                 {64, &MPIR_Alltoall_Scatter_dest_MVP},                        \
                 {128, &MPIR_Alltoall_Scatter_dest_MVP},                       \
                 {256, &MPIR_Alltoall_Scatter_dest_MVP},                       \
                 {512, &MPIR_Alltoall_Scatter_dest_MVP},                       \
                 {1024, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {2048, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {4096, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {8192, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {16384, &MPIR_Alltoall_Scatter_dest_MVP},                     \
                 {32768, &MPIR_Alltoall_Scatter_dest_MVP},                     \
             }},                                                               \
            {16,                                                               \
             {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                 \
             16,                                                               \
             {                                                                 \
                 {1, &MPIR_Alltoall_RD_MVP},                                   \
                 {2, &MPIR_Alltoall_RD_MVP},                                   \
                 {4, &MPIR_Alltoall_RD_MVP},                                   \
                 {8, &MPIR_Alltoall_RD_MVP},                                   \
                 {16, &MPIR_Alltoall_RD_MVP},                                  \
                 {32, &MPIR_Alltoall_RD_MVP},                                  \
                 {64, &MPIR_Alltoall_Scatter_dest_MVP},                        \
                 {128, &MPIR_Alltoall_Scatter_dest_MVP},                       \
                 {256, &MPIR_Alltoall_Scatter_dest_MVP},                       \
                 {512, &MPIR_Alltoall_Scatter_dest_MVP},                       \
                 {1024, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {2048, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {4096, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {8192, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                 {16384, &MPIR_Alltoall_Scatter_dest_MVP},                     \
                 {32768, &MPIR_Alltoall_Scatter_dest_MVP},                     \
             }},                                                               \
        {                                                                      \
            32, {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1}, 16,          \
            {                                                                  \
                {1, &MPIR_Alltoall_RD_MVP}, {2, &MPIR_Alltoall_RD_MVP},        \
                    {4, &MPIR_Alltoall_RD_MVP}, {8, &MPIR_Alltoall_RD_MVP},    \
                    {16, &MPIR_Alltoall_RD_MVP},                               \
                    {32, &MPIR_Alltoall_bruck_MVP},                            \
                    {64, &MPIR_Alltoall_bruck_MVP},                            \
                    {128, &MPIR_Alltoall_bruck_MVP},                           \
                    {256, &MPIR_Alltoall_bruck_MVP},                           \
                    {512, &MPIR_Alltoall_Scatter_dest_MVP},                    \
                    {1024, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {2048, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {4096, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {8192, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {16384, &MPIR_Alltoall_Scatter_dest_MVP},                  \
                    {32768, &MPIR_Alltoall_Scatter_dest_MVP},                  \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__4PPN_CNT 4
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__4PPN_MAX 32
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__4PPN_CNT 4

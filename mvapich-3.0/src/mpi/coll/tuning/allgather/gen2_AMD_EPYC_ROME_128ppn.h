#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__128PPN                            \
    {                                                                          \
        {128,                                                                  \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Allgather_RD_MVP},                                      \
             {2, &MPIR_Allgather_RD_MVP},                                      \
             {4, &MPIR_Allgather_RD_MVP},                                      \
             {8, &MPIR_Allgather_RD_MVP},                                      \
             {16, &MPIR_Allgather_RD_MVP},                                     \
             {32, &MPIR_Allgather_RD_MVP},                                     \
             {64, &MPIR_Allgather_RD_MVP},                                     \
             {128, &MPIR_Allgather_RD_MVP},                                    \
             {256, &MPIR_Allgather_RD_MVP},                                    \
             {512, &MPIR_Allgather_RD_MVP},                                    \
             {1024, &MPIR_Allgather_RD_MVP},                                   \
             {2048, &MPIR_Allgather_RD_MVP},                                   \
             {4096, &MPIR_Allgather_RD_MVP},                                   \
             {8192, &MPIR_Allgather_RD_MVP},                                   \
             {16384, &MPIR_Allgather_Ring_MVP},                                \
             {32768, &MPIR_Allgather_Ring_MVP},                                \
             {65536, &MPIR_Allgather_Ring_MVP},                                \
             {131072, &MPIR_Allgather_Ring_MVP},                               \
             {262144, &MPIR_Allgather_Ring_MVP},                               \
             {524288, &MPIR_Allgather_Ring_MVP},                               \
             {1048576, &MPIR_Allgather_Ring_MVP},                              \
         }},                                                                   \
            {256,                                                              \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Allgather_RD_MVP},                                  \
                 {2, &MPIR_Allgather_RD_MVP},                                  \
                 {4, &MPIR_Allgather_RD_MVP},                                  \
                 {8, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
                 {16, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                 {32, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                 {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                 {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},                 \
                 {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},                 \
                 {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},                 \
                 {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {2048, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {4096, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {8192, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {16384, &MPIR_Allgather_Ring_MVP},                            \
                 {32768, &MPIR_Allgather_Ring_MVP},                            \
                 {65536, &MPIR_Allgather_Ring_MVP},                            \
                 {131072, &MPIR_Allgather_Ring_MVP},                           \
                 {262144, &MPIR_Allgather_Ring_MVP},                           \
                 {524288, &MPIR_Allgather_Ring_MVP},                           \
                 {1048576, &MPIR_Allgather_Ring_MVP},                          \
             }},                                                               \
            {512,                                                              \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Allgather_RD_MVP},                                  \
                 {2, &MPIR_Allgather_RD_MVP},                                  \
                 {4, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
                 {8, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
                 {16, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                 {32, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                 {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                 {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},                 \
                 {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},                 \
                 {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},                 \
                 {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {2048, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {4096, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                 {8192, &MPIR_Allgather_Ring_MVP},                             \
                 {16384, &MPIR_Allgather_Ring_MVP},                            \
                 {32768, &MPIR_Allgather_Ring_MVP},                            \
                 {65536, &MPIR_Allgather_Ring_MVP},                            \
                 {131072, &MPIR_Allgather_Ring_MVP},                           \
                 {262144, &MPIR_Allgather_Ring_MVP},                           \
                 {524288, &MPIR_Allgather_Ring_MVP},                           \
                 {1048576, &MPIR_Allgather_Ring_MVP},                          \
             }},                                                               \
        {                                                                      \
            1024, 20,                                                          \
            {                                                                  \
                {1, &MPIR_Allgather_RD_MVP},                                   \
                    {2, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                    {4, &MPIR_Allgather_RD_MVP},                               \
                    {8, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                    {16, &MPIR_Allgather_RD_Allgather_Comm_MVP},               \
                    {32, &MPIR_Allgather_RD_Allgather_Comm_MVP},               \
                    {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},               \
                    {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},              \
                    {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},              \
                    {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},              \
                    {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},             \
                    {2048, &MPIR_Allgather_Ring_MVP},                          \
                    {4096, &MPIR_Allgather_Ring_MVP},                          \
                    {8192, &MPIR_Allgather_RD_Allgather_Comm_MVP},             \
                    {16384, &MPIR_Allgather_Ring_MVP},                         \
                    {32768, &MPIR_Allgather_Ring_MVP},                         \
                    {65536, &MPIR_Allgather_Ring_MVP},                         \
                    {131072, &MPIR_Allgather_Ring_MVP},                        \
                    {262144, &MPIR_Allgather_Ring_MVP},                        \
                    {524288, &MPIR_Allgather_Ring_MVP},                        \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__AMD_EPYC_7742_128__MLX_CX_HDR__128PPN_CNT 4

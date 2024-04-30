#define gen2__AMD_EPYC_7763_128__MLX_CX_HDR__128PPN                            \
    {                                                                          \
        {128,                                                                  \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Scatter_MVP_Binomial},                                  \
             {2, &MPIR_Scatter_MVP_Binomial},                                  \
             {4, &MPIR_Scatter_MVP_Binomial},                                  \
             {8, &MPIR_Scatter_MVP_Binomial},                                  \
             {16, &MPIR_Scatter_MVP_Binomial},                                 \
             {32, &MPIR_Scatter_MVP_Binomial},                                 \
             {64, &MPIR_Scatter_MVP_Binomial},                                 \
             {128, &MPIR_Scatter_MVP_Binomial},                                \
             {256, &MPIR_Scatter_MVP_Binomial},                                \
             {512, &MPIR_Scatter_MVP_Binomial},                                \
             {1024, &MPIR_Scatter_MVP_two_level_Direct},                       \
             {2048, &MPIR_Scatter_MVP_Direct},                                 \
             {4096, &MPIR_Scatter_MVP_two_level_Direct},                       \
             {8192, &MPIR_Scatter_MVP_two_level_Direct},                       \
             {16384, &MPIR_Scatter_MVP_two_level_Direct},                      \
             {32768, &MPIR_Scatter_MVP_two_level_Binomial},                    \
             {65536, &MPIR_Scatter_MVP_two_level_Binomial},                    \
             {131072, &MPIR_Scatter_MVP_two_level_Binomial},                   \
             {262144, &MPIR_Scatter_MVP_two_level_Direct},                     \
             {524288, &MPIR_Scatter_MVP_two_level_Direct},                     \
             {1048576, &MPIR_Scatter_MVP_two_level_Direct},                    \
         },                                                                    \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Scatter_MVP_Direct},                                    \
             {2, &MPIR_Scatter_MVP_Direct},                                    \
             {4, &MPIR_Scatter_MVP_Direct},                                    \
             {8, &MPIR_Scatter_MVP_Direct},                                    \
             {16, &MPIR_Scatter_MVP_Direct},                                   \
             {32, &MPIR_Scatter_MVP_Direct},                                   \
             {64, &MPIR_Scatter_MVP_Direct},                                   \
             {128, &MPIR_Scatter_MVP_Direct},                                  \
             {256, &MPIR_Scatter_MVP_Direct},                                  \
             {512, &MPIR_Scatter_MVP_Direct},                                  \
             {1024, &MPIR_Scatter_MVP_Direct},                                 \
             {2048, &MPIR_Scatter_MVP_Direct},                                 \
             {4096, &MPIR_Scatter_MVP_Binomial},                               \
             {8192, &MPIR_Scatter_MVP_Direct},                                 \
             {16384, &MPIR_Scatter_MVP_Direct},                                \
             {32768, &MPIR_Scatter_MVP_Binomial},                              \
             {65536, &MPIR_Scatter_MVP_Binomial},                              \
             {131072, &MPIR_Scatter_MVP_Binomial},                             \
             {262144, &MPIR_Scatter_MVP_Binomial},                             \
             {524288, &MPIR_Scatter_MVP_Binomial},                             \
             {1048576, &MPIR_Scatter_MVP_Direct},                              \
         }},                                                                   \
            {256,                                                              \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Scatter_MVP_two_level_Binomial},                    \
                 {2, &MPIR_Scatter_MVP_Binomial},                              \
                 {4, &MPIR_Scatter_MVP_two_level_Binomial},                    \
                 {8, &MPIR_Scatter_MVP_Binomial},                              \
                 {16, &MPIR_Scatter_MVP_two_level_Direct},                     \
                 {32, &MPIR_Scatter_MVP_Binomial},                             \
                 {64, &MPIR_Scatter_MVP_Binomial},                             \
                 {128, &MPIR_Scatter_MVP_Binomial},                            \
                 {256, &MPIR_Scatter_MVP_Binomial},                            \
                 {512, &MPIR_Scatter_MVP_two_level_Binomial},                  \
                 {1024, &MPIR_Scatter_MVP_two_level_Binomial},                 \
                 {2048, &MPIR_Scatter_MVP_two_level_Binomial},                 \
                 {4096, &MPIR_Scatter_MVP_two_level_Binomial},                 \
                 {8192, &MPIR_Scatter_MVP_Direct},                             \
                 {16384, &MPIR_Scatter_MVP_Direct},                            \
                 {32768, &MPIR_Scatter_MVP_Direct},                            \
                 {65536, &MPIR_Scatter_MVP_Direct},                            \
                 {131072, &MPIR_Scatter_MVP_Direct},                           \
                 {262144, &MPIR_Scatter_MVP_Direct},                           \
                 {524288, &MPIR_Scatter_MVP_Direct},                           \
                 {1048576, &MPIR_Scatter_MVP_Direct},                          \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Scatter_MVP_Binomial},                              \
                 {2, &MPIR_Scatter_MVP_Direct},                                \
                 {4, &MPIR_Scatter_MVP_Binomial},                              \
                 {8, &MPIR_Scatter_MVP_Direct},                                \
                 {16, &MPIR_Scatter_MVP_Binomial},                             \
                 {32, &MPIR_Scatter_MVP_Direct},                               \
                 {64, &MPIR_Scatter_MVP_Direct},                               \
                 {128, &MPIR_Scatter_MVP_Direct},                              \
                 {256, &MPIR_Scatter_MVP_Direct},                              \
                 {512, &MPIR_Scatter_MVP_Direct},                              \
                 {1024, &MPIR_Scatter_MVP_Direct},                             \
                 {2048, &MPIR_Scatter_MVP_Direct},                             \
                 {4096, &MPIR_Scatter_MVP_Direct},                             \
                 {8192, &MPIR_Scatter_MVP_Direct},                             \
                 {16384, &MPIR_Scatter_MVP_Direct},                            \
                 {32768, &MPIR_Scatter_MVP_Direct},                            \
                 {65536, &MPIR_Scatter_MVP_Direct},                            \
                 {131072, &MPIR_Scatter_MVP_Direct},                           \
                 {262144, &MPIR_Scatter_MVP_Direct},                           \
                 {524288, &MPIR_Scatter_MVP_Direct},                           \
                 {1048576, &MPIR_Scatter_MVP_Direct},                          \
             }},                                                               \
            {512,                                                              \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Scatter_MVP_two_level_Direct},                      \
                 {2, &MPIR_Scatter_MVP_two_level_Direct},                      \
                 {4, &MPIR_Scatter_MVP_two_level_Direct},                      \
                 {8, &MPIR_Scatter_MVP_two_level_Direct},                      \
                 {16, &MPIR_Scatter_MVP_two_level_Direct},                     \
                 {32, &MPIR_Scatter_MVP_two_level_Direct},                     \
                 {64, &MPIR_Scatter_MVP_two_level_Direct},                     \
                 {128, &MPIR_Scatter_MVP_two_level_Direct},                    \
                 {256, &MPIR_Scatter_MVP_two_level_Direct},                    \
                 {512, &MPIR_Scatter_MVP_two_level_Direct},                    \
                 {1024, &MPIR_Scatter_MVP_two_level_Direct},                   \
                 {2048, &MPIR_Scatter_MVP_two_level_Direct},                   \
                 {4096, &MPIR_Scatter_MVP_two_level_Direct},                   \
                 {8192, &MPIR_Scatter_MVP_Direct},                             \
                 {16384, &MPIR_Scatter_MVP_Direct},                            \
                 {32768, &MPIR_Scatter_MVP_Direct},                            \
                 {65536, &MPIR_Scatter_MVP_Direct},                            \
                 {131072, &MPIR_Scatter_MVP_Direct},                           \
                 {262144, &MPIR_Scatter_MVP_Direct},                           \
                 {524288, &MPIR_Scatter_MVP_Direct},                           \
                 {1048576, &MPIR_Scatter_MVP_Direct},                          \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Scatter_MVP_Binomial},                              \
                 {2, &MPIR_Scatter_MVP_Binomial},                              \
                 {4, &MPIR_Scatter_MVP_Binomial},                              \
                 {8, &MPIR_Scatter_MVP_Binomial},                              \
                 {16, &MPIR_Scatter_MVP_Binomial},                             \
                 {32, &MPIR_Scatter_MVP_Binomial},                             \
                 {64, &MPIR_Scatter_MVP_Binomial},                             \
                 {128, &MPIR_Scatter_MVP_Binomial},                            \
                 {256, &MPIR_Scatter_MVP_Direct},                              \
                 {512, &MPIR_Scatter_MVP_Direct},                              \
                 {1024, &MPIR_Scatter_MVP_Direct},                             \
                 {2048, &MPIR_Scatter_MVP_Direct},                             \
                 {4096, &MPIR_Scatter_MVP_Direct},                             \
                 {8192, &MPIR_Scatter_MVP_Direct},                             \
                 {16384, &MPIR_Scatter_MVP_Direct},                            \
                 {32768, &MPIR_Scatter_MVP_Direct},                            \
                 {65536, &MPIR_Scatter_MVP_Direct},                            \
                 {131072, &MPIR_Scatter_MVP_Direct},                           \
                 {262144, &MPIR_Scatter_MVP_Direct},                           \
                 {524288, &MPIR_Scatter_MVP_Direct},                           \
                 {1048576, &MPIR_Scatter_MVP_Direct},                          \
             }},                                                               \
        {                                                                      \
            1024, 20,                                                          \
                {                                                              \
                    {1, &MPIR_Scatter_MVP_two_level_Direct},                   \
                    {2, &MPIR_Scatter_MVP_two_level_Direct},                   \
                    {4, &MPIR_Scatter_MVP_two_level_Direct},                   \
                    {8, &MPIR_Scatter_MVP_two_level_Direct},                   \
                    {16, &MPIR_Scatter_MVP_two_level_Direct},                  \
                    {32, &MPIR_Scatter_MVP_two_level_Direct},                  \
                    {64, &MPIR_Scatter_MVP_two_level_Direct},                  \
                    {128, &MPIR_Scatter_MVP_two_level_Direct},                 \
                    {256, &MPIR_Scatter_MVP_two_level_Direct},                 \
                    {512, &MPIR_Scatter_MVP_two_level_Direct},                 \
                    {1024, &MPIR_Scatter_MVP_two_level_Direct},                \
                    {2048, &MPIR_Scatter_MVP_two_level_Direct},                \
                    {4096, &MPIR_Scatter_MVP_two_level_Direct},                \
                    {8192, &MPIR_Scatter_MVP_two_level_Direct},                \
                    {16384, &MPIR_Scatter_MVP_two_level_Binomial},             \
                    {32768, &MPIR_Scatter_MVP_two_level_Binomial},             \
                    {65536, &MPIR_Scatter_MVP_Direct},                         \
                    {131072, &MPIR_Scatter_MVP_Direct},                        \
                    {262144, &MPIR_Scatter_MVP_Direct},                        \
                    {524288, &MPIR_Scatter_MVP_Direct},                        \
                },                                                             \
                20,                                                            \
            {                                                                  \
                {1, &MPIR_Scatter_MVP_Binomial},                               \
                    {2, &MPIR_Scatter_MVP_Binomial},                           \
                    {4, &MPIR_Scatter_MVP_Binomial},                           \
                    {8, &MPIR_Scatter_MVP_Binomial},                           \
                    {16, &MPIR_Scatter_MVP_Binomial},                          \
                    {32, &MPIR_Scatter_MVP_Binomial},                          \
                    {64, &MPIR_Scatter_MVP_Binomial},                          \
                    {128, &MPIR_Scatter_MVP_Direct},                           \
                    {256, &MPIR_Scatter_MVP_Direct},                           \
                    {512, &MPIR_Scatter_MVP_Direct},                           \
                    {1024, &MPIR_Scatter_MVP_Direct},                          \
                    {2048, &MPIR_Scatter_MVP_Direct},                          \
                    {4096, &MPIR_Scatter_MVP_Direct},                          \
                    {8192, &MPIR_Scatter_MVP_Direct},                          \
                    {16384, &MPIR_Scatter_MVP_Direct},                         \
                    {32768, &MPIR_Scatter_MVP_Direct},                         \
                    {65536, &MPIR_Scatter_MVP_Direct},                         \
                    {131072, &MPIR_Scatter_MVP_Direct},                        \
                    {262144, &MPIR_Scatter_MVP_Direct},                        \
                    {524288, &MPIR_Scatter_MVP_Direct},                        \
            }                                                                  \
        }                                                                      \
    }

#define gen2__AMD_EPYC_7763_128__MLX_CX_HDR__128PPN_CNT 4

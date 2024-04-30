#define PSM__INTEL_PLATINUM_8160_2S_48__INTEL_HFI_100__48PPN                   \
    {                                                                          \
        {48,                                                                   \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Gather_intra_binomial},                                 \
             {2, &MPIR_Gather_intra_binomial},                                 \
             {4, &MPIR_Gather_intra_binomial},                                 \
             {8, &MPIR_Gather_MVP_Direct},                                     \
             {16, &MPIR_Gather_MVP_Direct},                                    \
             {32, &MPIR_Gather_MVP_Direct},                                    \
             {64, &MPIR_Gather_MVP_two_level_Direct},                          \
             {128, &MPIR_Gather_MVP_Direct},                                   \
             {256, &MPIR_Gather_MVP_two_level_Direct},                         \
             {512, &MPIR_Gather_MVP_two_level_Direct},                         \
             {1024, &MPIR_Gather_intra_binomial},                              \
             {2048, &MPIR_Gather_intra_binomial},                              \
             {4096, &MPIR_Gather_MVP_Direct},                                  \
             {8192, &MPIR_Gather_MVP_two_level_Direct},                        \
             {16384, &MPIR_Gather_MVP_two_level_Direct},                       \
             {32768, &MPIR_Gather_MVP_Direct},                                 \
             {65536, &MPIR_Gather_MVP_Direct},                                 \
             {131072, &MPIR_Gather_MVP_two_level_Direct},                      \
             {262144, &MPIR_Gather_MVP_Direct},                                \
             {524288, &MPIR_Gather_MVP_Direct},                                \
             {1048576, &MPIR_Gather_intra_binomial},                           \
         },                                                                    \
         21,                                                                   \
         {                                                                     \
             {1, &MPIR_Gather_MVP_Direct},                                     \
             {2, &MPIR_Gather_MVP_Direct},                                     \
             {4, &MPIR_Gather_MVP_Direct},                                     \
             {8, &MPIR_Gather_MVP_Direct},                                     \
             {16, &MPIR_Gather_MVP_Direct},                                    \
             {32, &MPIR_Gather_MVP_Direct},                                    \
             {64, &MPIR_Gather_MVP_Direct},                                    \
             {128, &MPIR_Gather_MVP_Direct},                                   \
             {256, &MPIR_Gather_MVP_Direct},                                   \
             {512, &MPIR_Gather_MVP_Direct},                                   \
             {1024, &MPIR_Gather_MVP_Direct},                                  \
             {2048, &MPIR_Gather_MVP_Direct},                                  \
             {4096, &MPIR_Gather_MVP_Direct},                                  \
             {8192, &MPIR_Gather_MVP_Direct},                                  \
             {16384, &MPIR_Gather_intra_binomial},                             \
             {32768, &MPIR_Gather_MVP_Direct},                                 \
             {65536, &MPIR_Gather_MVP_Direct},                                 \
             {131072, &MPIR_Gather_MVP_Direct},                                \
             {262144, &MPIR_Gather_MVP_Direct},                                \
             {524288, &MPIR_Gather_MVP_Direct},                                \
             {1048576, &MPIR_Gather_MVP_Direct},                               \
         }},                                                                   \
            {96,                                                               \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {2, &MPIR_Gather_MVP_Direct},                                 \
                 {4, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {8, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {16, &MPIR_Gather_MVP_two_level_Direct},                      \
                 {32, &MPIR_Gather_MVP_Direct},                                \
                 {64, &MPIR_Gather_MVP_Direct},                                \
                 {128, &MPIR_Gather_MVP_Direct},                               \
                 {256, &MPIR_Gather_MVP_two_level_Direct},                     \
                 {512, &MPIR_Gather_MVP_two_level_Direct},                     \
                 {1024, &MPIR_Gather_MVP_Direct},                              \
                 {2048, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {4096, &MPIR_Gather_intra_binomial},                          \
                 {8192, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {16384, &MPIR_Gather_MVP_Direct},                             \
                 {32768, &MPIR_Gather_MVP_two_level_Direct},                   \
                 {65536, &MPIR_Gather_MVP_two_level_Direct},                   \
                 {131072, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {262144, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {524288, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {1048576, &MPIR_Gather_MVP_two_level_Direct},                 \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Gather_intra_binomial},                             \
                 {2, &MPIR_Gather_MVP_Direct},                                 \
                 {4, &MPIR_Gather_intra_binomial},                             \
                 {8, &MPIR_Gather_intra_binomial},                             \
                 {16, &MPIR_Gather_MVP_Direct},                                \
                 {32, &MPIR_Gather_MVP_Direct},                                \
                 {64, &MPIR_Gather_MVP_Direct},                                \
                 {128, &MPIR_Gather_MVP_Direct},                               \
                 {256, &MPIR_Gather_intra_binomial},                           \
                 {512, &MPIR_Gather_intra_binomial},                           \
                 {1024, &MPIR_Gather_MVP_Direct},                              \
                 {2048, &MPIR_Gather_intra_binomial},                          \
                 {4096, &MPIR_Gather_MVP_Direct},                              \
                 {8192, &MPIR_Gather_intra_binomial},                          \
                 {16384, &MPIR_Gather_MVP_Direct},                             \
                 {32768, &MPIR_Gather_MVP_Direct},                             \
                 {65536, &MPIR_Gather_intra_binomial},                         \
                 {131072, &MPIR_Gather_MVP_Direct},                            \
                 {262144, &MPIR_Gather_intra_binomial},                        \
                 {524288, &MPIR_Gather_intra_binomial},                        \
                 {1048576, &MPIR_Gather_MVP_Direct},                           \
             }},                                                               \
            {192,                                                              \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {2, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {4, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {8, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {16, &MPIR_Gather_intra_binomial},                            \
                 {32, &MPIR_Gather_MVP_two_level_Direct},                      \
                 {64, &MPIR_Gather_MVP_two_level_Direct},                      \
                 {128, &MPIR_Gather_intra_binomial},                           \
                 {256, &MPIR_Gather_MVP_Direct},                               \
                 {512, &MPIR_Gather_MVP_two_level_Direct},                     \
                 {1024, &MPIR_Gather_MVP_Direct},                              \
                 {2048, &MPIR_Gather_intra_binomial},                          \
                 {4096, &MPIR_Gather_MVP_Direct},                              \
                 {8192, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {16384, &MPIR_Gather_intra_binomial},                         \
                 {32768, &MPIR_Gather_intra_binomial},                         \
                 {65536, &MPIR_Gather_MVP_two_level_Direct},                   \
                 {131072, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {262144, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {524288, &MPIR_Gather_intra_binomial},                        \
                 {1048576, &MPIR_Gather_MVP_two_level_Direct},                 \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Gather_MVP_Direct},                                 \
                 {2, &MPIR_Gather_MVP_Direct},                                 \
                 {4, &MPIR_Gather_MVP_Direct},                                 \
                 {8, &MPIR_Gather_intra_binomial},                             \
                 {16, &MPIR_Gather_MVP_Direct},                                \
                 {32, &MPIR_Gather_intra_binomial},                            \
                 {64, &MPIR_Gather_MVP_Direct},                                \
                 {128, &MPIR_Gather_MVP_Direct},                               \
                 {256, &MPIR_Gather_MVP_Direct},                               \
                 {512, &MPIR_Gather_MVP_Direct},                               \
                 {1024, &MPIR_Gather_MVP_Direct},                              \
                 {2048, &MPIR_Gather_MVP_Direct},                              \
                 {4096, &MPIR_Gather_MVP_Direct},                              \
                 {8192, &MPIR_Gather_MVP_Direct},                              \
                 {16384, &MPIR_Gather_MVP_Direct},                             \
                 {32768, &MPIR_Gather_MVP_Direct},                             \
                 {65536, &MPIR_Gather_intra_binomial},                         \
                 {131072, &MPIR_Gather_MVP_Direct},                            \
                 {262144, &MPIR_Gather_MVP_Direct},                            \
                 {524288, &MPIR_Gather_MVP_Direct},                            \
                 {1048576, &MPIR_Gather_intra_binomial},                       \
             }},                                                               \
            {384,                                                              \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {2, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {4, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {8, &MPIR_Gather_MVP_two_level_Direct},                       \
                 {16, &MPIR_Gather_intra_binomial},                            \
                 {32, &MPIR_Gather_MVP_Direct},                                \
                 {64, &MPIR_Gather_MVP_two_level_Direct},                      \
                 {128, &MPIR_Gather_MVP_two_level_Direct},                     \
                 {256, &MPIR_Gather_MVP_two_level_Direct},                     \
                 {512, &MPIR_Gather_MVP_two_level_Direct},                     \
                 {1024, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {2048, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {4096, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {8192, &MPIR_Gather_MVP_two_level_Direct},                    \
                 {16384, &MPIR_Gather_MVP_two_level_Direct},                   \
                 {32768, &MPIR_Gather_MVP_two_level_Direct},                   \
                 {65536, &MPIR_Gather_MVP_two_level_Direct},                   \
                 {131072, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {262144, &MPIR_Gather_MVP_two_level_Direct},                  \
                 {524288, &MPIR_Gather_intra_binomial},                        \
                 {1048576, &MPIR_Gather_intra_binomial},                       \
             },                                                                \
             21,                                                               \
             {                                                                 \
                 {1, &MPIR_Gather_MVP_Direct},                                 \
                 {2, &MPIR_Gather_MVP_Direct},                                 \
                 {4, &MPIR_Gather_MVP_Direct},                                 \
                 {8, &MPIR_Gather_MVP_Direct},                                 \
                 {16, &MPIR_Gather_MVP_Direct},                                \
                 {32, &MPIR_Gather_MVP_Direct},                                \
                 {64, &MPIR_Gather_MVP_Direct},                                \
                 {128, &MPIR_Gather_MVP_Direct},                               \
                 {256, &MPIR_Gather_MVP_Direct},                               \
                 {512, &MPIR_Gather_MVP_Direct},                               \
                 {1024, &MPIR_Gather_MVP_Direct},                              \
                 {2048, &MPIR_Gather_MVP_Direct},                              \
                 {4096, &MPIR_Gather_MVP_Direct},                              \
                 {8192, &MPIR_Gather_MVP_Direct},                              \
                 {16384, &MPIR_Gather_MVP_Direct},                             \
                 {32768, &MPIR_Gather_MVP_Direct},                             \
                 {65536, &MPIR_Gather_MVP_Direct},                             \
                 {131072, &MPIR_Gather_MVP_Direct},                            \
                 {262144, &MPIR_Gather_MVP_Direct},                            \
                 {524288, &MPIR_Gather_MVP_Direct},                            \
                 {1048576, &MPIR_Gather_MVP_Direct},                           \
             }},                                                               \
        {                                                                      \
            768, 20,                                                           \
                {                                                              \
                    {1, &MPIR_Gather_MVP_two_level_Direct},                    \
                    {2, &MPIR_Gather_intra_binomial},                          \
                    {4, &MPIR_Gather_intra_binomial},                          \
                    {8, &MPIR_Gather_MVP_Direct},                              \
                    {16, &MPIR_Gather_MVP_two_level_Direct},                   \
                    {32, &MPIR_Gather_MVP_two_level_Direct},                   \
                    {64, &MPIR_Gather_MVP_two_level_Direct},                   \
                    {128, &MPIR_Gather_MVP_Direct},                            \
                    {256, &MPIR_Gather_intra_binomial},                        \
                    {512, &MPIR_Gather_intra_binomial},                        \
                    {1024, &MPIR_Gather_MVP_two_level_Direct},                 \
                    {2048, &MPIR_Gather_MVP_two_level_Direct},                 \
                    {4096, &MPIR_Gather_MVP_two_level_Direct},                 \
                    {8192, &MPIR_Gather_intra_binomial},                       \
                    {16384, &MPIR_Gather_MVP_Direct},                          \
                    {32768, &MPIR_Gather_MVP_two_level_Direct},                \
                    {65536, &MPIR_Gather_intra_binomial},                      \
                    {131072, &MPIR_Gather_MVP_two_level_Direct},               \
                    {262144, &MPIR_Gather_MVP_two_level_Direct},               \
                    {524288, &MPIR_Gather_MVP_two_level_Direct},               \
                },                                                             \
                20,                                                            \
            {                                                                  \
                {1, &MPIR_Gather_intra_binomial},                              \
                    {2, &MPIR_Gather_MVP_Direct},                              \
                    {4, &MPIR_Gather_MVP_Direct},                              \
                    {8, &MPIR_Gather_MVP_Direct},                              \
                    {16, &MPIR_Gather_MVP_Direct},                             \
                    {32, &MPIR_Gather_intra_binomial},                         \
                    {64, &MPIR_Gather_intra_binomial},                         \
                    {128, &MPIR_Gather_MVP_Direct},                            \
                    {256, &MPIR_Gather_MVP_Direct},                            \
                    {512, &MPIR_Gather_MVP_Direct},                            \
                    {1024, &MPIR_Gather_MVP_Direct},                           \
                    {2048, &MPIR_Gather_intra_binomial},                       \
                    {4096, &MPIR_Gather_MVP_Direct},                           \
                    {8192, &MPIR_Gather_MVP_Direct},                           \
                    {16384, &MPIR_Gather_MVP_Direct},                          \
                    {32768, &MPIR_Gather_intra_binomial},                      \
                    {65536, &MPIR_Gather_MVP_Direct},                          \
                    {131072, &MPIR_Gather_MVP_Direct},                         \
                    {262144, &MPIR_Gather_intra_binomial},                     \
                    {524288, &MPIR_Gather_intra_binomial},                     \
            }                                                                  \
        }                                                                      \
    }

#define PSM__INTEL_PLATINUM_8160_2S_48__INTEL_HFI_100__48PPN_CNT 5

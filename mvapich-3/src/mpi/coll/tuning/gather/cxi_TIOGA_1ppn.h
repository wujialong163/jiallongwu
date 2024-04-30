#define CXI__AMD_EPYC_7A53_64__CRAY_SS11__1PPN                                 \
    {                                                                          \
        {                                                                      \
            4, 21,                                                             \
                {                                                              \
                    {1, &MPIR_Gather_MVP_Direct},                              \
                    {2, &MPIR_Gather_MVP_Direct},                              \
                    {4, &MPIR_Gather_MVP_Direct},                              \
                    {8, &MPIR_Gather_MVP_Direct},                              \
                    {16, &MPIR_Gather_MVP_two_level_Direct},                   \
                    {32, &MPIR_Gather_MVP_Direct},                             \
                    {64, &MPIR_Gather_MVP_Direct},                             \
                    {128, &MPIR_Gather_MVP_two_level_Direct},                  \
                    {256, &MPIR_Gather_MVP_Direct},                            \
                    {512, &MPIR_Gather_MVP_Direct},                            \
                    {1024, &MPIR_Gather_MVP_Direct},                           \
                    {2048, &MPIR_Gather_MVP_two_level_Direct},                 \
                    {4096, &MPIR_Gather_MVP_Direct},                           \
                    {8192, &MPIR_Gather_MVP_Direct},                           \
                    {16384, &MPIR_Gather_MVP_Direct},                          \
                    {32768, &MPIR_Gather_intra_binomial},                      \
                    {65536, &MPIR_Gather_MVP_two_level_Direct},                \
                    {131072, &MPIR_Gather_MVP_Direct},                         \
                    {262144, &MPIR_Gather_MVP_Direct},                         \
                    {524288, &MPIR_Gather_MVP_Direct},                         \
                    {1048576, &MPIR_Gather_MVP_Direct},                        \
                },                                                             \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Gather_MVP_Direct}, {2, &MPIR_Gather_MVP_Direct},    \
                    {4, &MPIR_Gather_MVP_Direct},                              \
                    {8, &MPIR_Gather_MVP_Direct},                              \
                    {16, &MPIR_Gather_MVP_Direct},                             \
                    {32, &MPIR_Gather_MVP_Direct},                             \
                    {64, &MPIR_Gather_MVP_Direct},                             \
                    {128, &MPIR_Gather_MVP_Direct},                            \
                    {256, &MPIR_Gather_MVP_Direct},                            \
                    {512, &MPIR_Gather_MVP_Direct},                            \
                    {1024, &MPIR_Gather_MVP_Direct},                           \
                    {2048, &MPIR_Gather_MVP_Direct},                           \
                    {4096, &MPIR_Gather_MVP_Direct},                           \
                    {8192, &MPIR_Gather_MVP_Direct},                           \
                    {16384, &MPIR_Gather_MVP_Direct},                          \
                    {32768, &MPIR_Gather_MVP_Direct},                          \
                    {65536, &MPIR_Gather_MVP_Direct},                          \
                    {131072, &MPIR_Gather_MVP_Direct},                         \
                    {262144, &MPIR_Gather_MVP_Direct},                         \
                    {524288, &MPIR_Gather_MVP_Direct},                         \
                    {1048576, &MPIR_Gather_MVP_Direct},                        \
            }                                                                  \
        }                                                                      \
    }

#define CXI__AMD_EPYC_7A53_64__CRAY_SS11__1PPN_CNT 1

#define PSM__INTEL_XEON_E5_2695_V4_2S_36__INTEL_HFI_100__36PPN                 \
    {                                                                          \
        {36, 21, {{1, &MPIR_Allgather_Bruck_MVP},                              \
                  {2, &MPIR_Allgather_RD_Allgather_Comm_MVP},                  \
                  {4, &MPIR_Allgather_Bruck_MVP},                              \
                  {8, &MPIR_Allgather_Bruck_MVP},                              \
                  {16, &MPIR_Allgather_RD_MVP},                                \
                  {32, &MPIR_Allgather_Bruck_MVP},                             \
                  {64, &MPIR_Allgather_RD_MVP},                                \
                  {128, &MPIR_Allgather_RD_MVP},                               \
                  {256, &MPIR_Allgather_Bruck_MVP},                            \
                  {512, &MPIR_Allgather_RD_MVP},                               \
                  {1024, &MPIR_Allgather_Bruck_MVP},                           \
                  {2048, &MPIR_Allgather_Ring_MVP},                            \
                  {4096, &MPIR_Allgather_Ring_MVP},                            \
                  {8192, &MPIR_Allgather_Ring_MVP},                            \
                  {16384, &MPIR_Allgather_Ring_MVP},                           \
                  {32768, &MPIR_Allgather_Ring_MVP},                           \
                  {65536, &MPIR_Allgather_Ring_MVP},                           \
                  {131072, &MPIR_Allgather_Ring_MVP},                          \
                  {262144, &MPIR_Allgather_Ring_MVP},                          \
                  {524288, &MPIR_Allgather_Ring_MVP},                          \
                  {1048576, &MPIR_Allgather_Ring_MVP}}},                       \
            {72,                                                               \
             21,                                                               \
             {{1, &MPIR_Allgather_Bruck_MVP},                                  \
              {2, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {4, &MPIR_Allgather_Bruck_MVP},                                  \
              {8, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {16, &MPIR_Allgather_RD_MVP},                                    \
              {32, &MPIR_Allgather_Bruck_MVP},                                 \
              {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},                     \
              {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
              {2048, &MPIR_Allgather_Ring_MVP},                                \
              {4096, &MPIR_Allgather_Ring_MVP},                                \
              {8192, &MPIR_Allgather_Ring_MVP},                                \
              {16384, &MPIR_Allgather_Ring_MVP},                               \
              {32768, &MPIR_Allgather_Ring_MVP},                               \
              {65536, &MPIR_Allgather_Bruck_MVP},                              \
              {131072, &MPIR_Allgather_Ring_MVP},                              \
              {262144, &MPIR_Allgather_Ring_MVP},                              \
              {524288, &MPIR_Allgather_Ring_MVP},                              \
              {1048576, &MPIR_Allgather_Ring_MVP}}},                           \
            {144,                                                              \
             21,                                                               \
             {{1, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {2, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {4, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {8, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {16, &MPIR_Allgather_Bruck_MVP},                                 \
              {32, &MPIR_Allgather_RD_Allgather_Comm_MVP},                     \
              {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},                     \
              {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
              {2048, &MPIR_Allgather_Ring_MVP},                                \
              {4096, &MPIR_Allgather_Ring_MVP},                                \
              {8192, &MPIR_Allgather_Ring_MVP},                                \
              {16384, &MPIR_Allgather_Ring_MVP},                               \
              {32768, &MPIR_Allgather_Ring_MVP},                               \
              {65536, &MPIR_Allgather_Bruck_MVP},                              \
              {131072, &MPIR_Allgather_Ring_MVP},                              \
              {262144, &MPIR_Allgather_Ring_MVP},                              \
              {524288, &MPIR_Allgather_Ring_MVP},                              \
              {1048576, &MPIR_Allgather_Ring_MVP}}},                           \
            {288,                                                              \
             21,                                                               \
             {{1, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {2, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {4, &MPIR_Allgather_RD_Allgather_Comm_MVP},                      \
              {8, &MPIR_Allgather_Bruck_MVP},                                  \
              {16, &MPIR_Allgather_Bruck_MVP},                                 \
              {32, &MPIR_Allgather_RD_Allgather_Comm_MVP},                     \
              {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},                     \
              {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},                    \
              {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
              {2048, &MPIR_Allgather_RD_Allgather_Comm_MVP},                   \
              {4096, &MPIR_Allgather_Ring_MVP},                                \
              {8192, &MPIR_Allgather_Ring_MVP},                                \
              {16384, &MPIR_Allgather_Ring_MVP},                               \
              {32768, &MPIR_Allgather_Ring_MVP},                               \
              {65536, &MPIR_Allgather_Bruck_MVP},                              \
              {131072, &MPIR_Allgather_Ring_MVP},                              \
              {262144, &MPIR_Allgather_Ring_MVP},                              \
              {524288, &MPIR_Allgather_Ring_MVP},                              \
              {1048576, &MPIR_Allgather_Ring_MVP}}},                           \
        {                                                                      \
            576, 21,                                                           \
            {                                                                  \
                {1, &MPIR_Allgather_Bruck_MVP},                                \
                    {2, &MPIR_Allgather_Bruck_MVP},                            \
                    {4, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                    {8, &MPIR_Allgather_RD_Allgather_Comm_MVP},                \
                    {16, &MPIR_Allgather_RD_Allgather_Comm_MVP},               \
                    {32, &MPIR_Allgather_RD_Allgather_Comm_MVP},               \
                    {64, &MPIR_Allgather_RD_Allgather_Comm_MVP},               \
                    {128, &MPIR_Allgather_RD_Allgather_Comm_MVP},              \
                    {256, &MPIR_Allgather_RD_Allgather_Comm_MVP},              \
                    {512, &MPIR_Allgather_RD_Allgather_Comm_MVP},              \
                    {1024, &MPIR_Allgather_RD_Allgather_Comm_MVP},             \
                    {1024, &MPIR_Allgather_Bruck_MVP},                         \
                    {2048, &MPIR_Allgather_Ring_MVP},                          \
                    {4096, &MPIR_Allgather_Ring_MVP},                          \
                    {8192, &MPIR_Allgather_Ring_MVP},                          \
                    {16384, &MPIR_Allgather_Ring_MVP},                         \
                    {32768, &MPIR_Allgather_Ring_MVP},                         \
                    {65536, &MPIR_Allgather_Ring_MVP},                         \
                    {131072, &MPIR_Allgather_Ring_MVP},                        \
                    {262144, &MPIR_Allgather_Ring_MVP},                        \
                    {524288, &MPIR_Allgather_Ring_MVP},                        \
                {                                                              \
                    1048576, &MPIR_Allgather_Ring_MVP                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define PSM__INTEL_XEON_E5_2695_V4_2S_36__INTEL_HFI_100__36PPN_CNT 5

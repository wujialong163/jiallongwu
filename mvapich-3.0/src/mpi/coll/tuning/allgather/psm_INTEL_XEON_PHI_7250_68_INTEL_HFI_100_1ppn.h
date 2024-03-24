#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__1PPN                          \
    {                                                                          \
        {2,                                                                    \
         21,                                                                   \
         {{1, &MPIR_Allgather_Ring_MVP},    {2, &MPIR_Allgather_Ring_MVP},     \
          {4, &MPIR_Allgather_Ring_MVP},    {8, &MPIR_Allgather_Ring_MVP},     \
          {16, &MPIR_Allgather_Ring_MVP},   {32, &MPIR_Allgather_RD_MVP},      \
          {64, &MPIR_Allgather_Ring_MVP},   {128, &MPIR_Allgather_Ring_MVP},   \
          {256, &MPIR_Allgather_RD_MVP},    {512, &MPIR_Allgather_Ring_MVP},   \
          {1024, &MPIR_Allgather_RD_MVP},   {2048, &MPIR_Allgather_Ring_MVP},  \
          {4096, &MPIR_Allgather_Ring_MVP}, {8192, &MPIR_Allgather_RD_MVP},    \
          {16384, &MPIR_Allgather_RD_MVP},  {32768, &MPIR_Allgather_RD_MVP},   \
          {65536, &MPIR_Allgather_RD_MVP},  {131072, &MPIR_Allgather_RD_MVP},  \
          {262144, &MPIR_Allgather_RD_MVP}, {524288, &MPIR_Allgather_RD_MVP},  \
          {1048576, &MPIR_Allgather_RD_MVP}}},                                 \
            {4, 21, {{1, &MPIR_Allgather_Bruck_MVP},                           \
                     {2, &MPIR_Allgather_RD_MVP},                              \
                     {4, &MPIR_Allgather_Bruck_MVP},                           \
                     {8, &MPIR_Allgather_Ring_MVP},                            \
                     {16, &MPIR_Allgather_RD_MVP},                             \
                     {32, &MPIR_Allgather_Bruck_MVP},                          \
                     {64, &MPIR_Allgather_Bruck_MVP},                          \
                     {128, &MPIR_Allgather_RD_MVP},                            \
                     {256, &MPIR_Allgather_RD_MVP},                            \
                     {512, &MPIR_Allgather_Bruck_MVP},                         \
                     {1024, &MPIR_Allgather_RD_MVP},                           \
                     {2048, &MPIR_Allgather_RD_MVP},                           \
                     {4096, &MPIR_Allgather_Bruck_MVP},                        \
                     {8192, &MPIR_Allgather_Bruck_MVP},                        \
                     {16384, &MPIR_Allgather_RD_MVP},                          \
                     {32768, &MPIR_Allgather_RD_MVP},                          \
                     {65536, &MPIR_Allgather_RD_MVP},                          \
                     {131072, &MPIR_Allgather_RD_MVP},                         \
                     {262144, &MPIR_Allgather_RD_MVP},                         \
                     {524288, &MPIR_Allgather_RD_MVP},                         \
                     {1048576, &MPIR_Allgather_RD_MVP}}},                      \
            {8, 21, {{1, &MPIR_Allgather_RD_MVP},                              \
                     {2, &MPIR_Allgather_RD_MVP},                              \
                     {4, &MPIR_Allgather_Bruck_MVP},                           \
                     {8, &MPIR_Allgather_Bruck_MVP},                           \
                     {16, &MPIR_Allgather_RD_MVP},                             \
                     {32, &MPIR_Allgather_RD_MVP},                             \
                     {64, &MPIR_Allgather_Bruck_MVP},                          \
                     {128, &MPIR_Allgather_Bruck_MVP},                         \
                     {256, &MPIR_Allgather_Bruck_MVP},                         \
                     {512, &MPIR_Allgather_Bruck_MVP},                         \
                     {1024, &MPIR_Allgather_Bruck_MVP},                        \
                     {2048, &MPIR_Allgather_RD_MVP},                           \
                     {4096, &MPIR_Allgather_RD_MVP},                           \
                     {8192, &MPIR_Allgather_RD_MVP},                           \
                     {16384, &MPIR_Allgather_RD_MVP},                          \
                     {32768, &MPIR_Allgather_RD_MVP},                          \
                     {65536, &MPIR_Allgather_RD_MVP},                          \
                     {131072, &MPIR_Allgather_RD_MVP},                         \
                     {262144, &MPIR_Allgather_RD_MVP},                         \
                     {524288, &MPIR_Allgather_RD_MVP},                         \
                     {1048576, &MPIR_Allgather_RD_MVP}}},                      \
            {16, 21, {{1, &MPIR_Allgather_Bruck_MVP},                          \
                      {2, &MPIR_Allgather_RD_MVP},                             \
                      {4, &MPIR_Allgather_Bruck_MVP},                          \
                      {8, &MPIR_Allgather_RD_MVP},                             \
                      {16, &MPIR_Allgather_RD_MVP},                            \
                      {32, &MPIR_Allgather_RD_MVP},                            \
                      {64, &MPIR_Allgather_Bruck_MVP},                         \
                      {128, &MPIR_Allgather_RD_MVP},                           \
                      {256, &MPIR_Allgather_RD_MVP},                           \
                      {512, &MPIR_Allgather_RD_MVP},                           \
                      {1024, &MPIR_Allgather_Bruck_MVP},                       \
                      {2048, &MPIR_Allgather_Bruck_MVP},                       \
                      {4096, &MPIR_Allgather_RD_MVP},                          \
                      {8192, &MPIR_Allgather_RD_MVP},                          \
                      {16384, &MPIR_Allgather_RD_MVP},                         \
                      {32768, &MPIR_Allgather_RD_MVP},                         \
                      {65536, &MPIR_Allgather_RD_MVP},                         \
                      {131072, &MPIR_Allgather_RD_MVP},                        \
                      {262144, &MPIR_Allgather_RD_MVP},                        \
                      {524288, &MPIR_Allgather_RD_MVP},                        \
                      {1048576, &MPIR_Allgather_RD_MVP}}},                     \
        {                                                                      \
            32, 21,                                                            \
            {                                                                  \
                {1, &MPIR_Allgather_RD_MVP}, {2, &MPIR_Allgather_Bruck_MVP},   \
                    {4, &MPIR_Allgather_Bruck_MVP},                            \
                    {8, &MPIR_Allgather_RD_MVP},                               \
                    {16, &MPIR_Allgather_Bruck_MVP},                           \
                    {32, &MPIR_Allgather_RD_MVP},                              \
                    {64, &MPIR_Allgather_Bruck_MVP},                           \
                    {128, &MPIR_Allgather_RD_MVP},                             \
                    {256, &MPIR_Allgather_Bruck_MVP},                          \
                    {512, &MPIR_Allgather_Bruck_MVP},                          \
                    {1024, &MPIR_Allgather_Bruck_MVP},                         \
                    {2048, &MPIR_Allgather_RD_MVP},                            \
                    {4096, &MPIR_Allgather_RD_MVP},                            \
                    {8192, &MPIR_Allgather_RD_MVP},                            \
                    {16384, &MPIR_Allgather_RD_MVP},                           \
                    {32768, &MPIR_Allgather_RD_MVP},                           \
                    {65536, &MPIR_Allgather_RD_MVP},                           \
                    {131072, &MPIR_Allgather_RD_MVP},                          \
                    {262144, &MPIR_Allgather_RD_MVP},                          \
                    {524288, &MPIR_Allgather_RD_MVP},                          \
                {                                                              \
                    1048576, &MPIR_Allgather_RD_MVP                            \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__1PPN_CNT 5

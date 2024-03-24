#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__2PPN                          \
    {                                                                          \
        {2,                                                                    \
         8192,                                                                 \
         4,                                                                    \
         4,                                                                    \
         {0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},      \
         21,                                                                   \
         {{1, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {2, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {4, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {8, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {16, &MPIR_Bcast_binomial_MVP, -1},                                  \
          {32, &MPIR_Bcast_binomial_MVP, -1},                                  \
          {64, &MPIR_Bcast_binomial_MVP, -1},                                  \
          {128, &MPIR_Bcast_binomial_MVP, -1},                                 \
          {256, &MPIR_Bcast_binomial_MVP, -1},                                 \
          {512, &MPIR_Bcast_binomial_MVP, -1},                                 \
          {1024, &MPIR_Bcast_binomial_MVP, -1},                                \
          {2048, &MPIR_Bcast_binomial_MVP, -1},                                \
          {4096, &MPIR_Bcast_binomial_MVP, -1},                                \
          {8192, &MPIR_Bcast_binomial_MVP, -1},                                \
          {16384, &MPIR_Bcast_binomial_MVP, -1},                               \
          {32768, &MPIR_Bcast_binomial_MVP, -1},                               \
          {65536, &MPIR_Bcast_binomial_MVP, -1},                               \
          {131072, &MPIR_Bcast_binomial_MVP, -1},                              \
          {262144, &MPIR_Bcast_binomial_MVP, -1},                              \
          {524288, &MPIR_Bcast_binomial_MVP, -1},                              \
          {1048576, &MPIR_Bcast_binomial_MVP, 4}},                             \
         21,                                                                   \
         {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
          {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                          \
          {4, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                          \
          {8, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                         \
          {32, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                         \
          {128, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
          {256, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                        \
          {512, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                        \
          {1024, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                       \
          {4096, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                       \
          {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                      \
          {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                      \
          {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
          {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                     \
          {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                     \
          {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                     \
          {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 4}}},                  \
            {4,                                                                \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0},  \
             21,                                                               \
             {{1, &MPIR_Bcast_binomial_MVP, -1},                               \
              {2, &MPIR_Bcast_binomial_MVP, -1},                               \
              {4, &MPIR_Bcast_binomial_MVP, -1},                               \
              {8, &MPIR_Bcast_binomial_MVP, -1},                               \
              {16, &MPIR_Bcast_binomial_MVP, -1},                              \
              {32, &MPIR_Bcast_binomial_MVP, -1},                              \
              {64, &MPIR_Bcast_binomial_MVP, -1},                              \
              {128, &MPIR_Bcast_binomial_MVP, -1},                             \
              {256, &MPIR_Bcast_binomial_MVP, -1},                             \
              {512, &MPIR_Bcast_binomial_MVP, -1},                             \
              {1024, &MPIR_Bcast_binomial_MVP, -1},                            \
              {2048, &MPIR_Bcast_binomial_MVP, -1},                            \
              {4096, &MPIR_Bcast_binomial_MVP, -1},                            \
              {8192, &MPIR_Bcast_binomial_MVP, -1},                            \
              {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {32768, &MPIR_Bcast_binomial_MVP, -1},                           \
              {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {262144, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {1048576, &MPIR_Bcast_binomial_MVP, -1}},                        \
             21,                                                               \
             {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {2, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                      \
              {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {8, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                      \
              {16, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                     \
              {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                     \
              {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                     \
              {128, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {256, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                    \
              {1024, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
              {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
              {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {32768, &MPIR_Shmem_Bcast_MVP, -1},                              \
              {65536, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {131072, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                \
              {262144, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                \
              {524288, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                \
              {1048576, &MPIR_Shmem_Bcast_MVP, -1}}},                          \
            {8,                                                                \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {{1, &MPIR_Bcast_binomial_MVP, -1},                               \
              {2, &MPIR_Bcast_binomial_MVP, -1},                               \
              {4, &MPIR_Bcast_binomial_MVP, -1},                               \
              {8, &MPIR_Bcast_binomial_MVP, -1},                               \
              {16, &MPIR_Bcast_binomial_MVP, -1},                              \
              {32, &MPIR_Bcast_binomial_MVP, -1},                              \
              {64, &MPIR_Bcast_binomial_MVP, -1},                              \
              {128, &MPIR_Bcast_binomial_MVP, -1},                             \
              {256, &MPIR_Bcast_binomial_MVP, -1},                             \
              {512, &MPIR_Bcast_binomial_MVP, -1},                             \
              {1024, &MPIR_Bcast_binomial_MVP, -1},                            \
              {2048, &MPIR_Bcast_binomial_MVP, -1},                            \
              {4096, &MPIR_Bcast_binomial_MVP, -1},                            \
              {8192, &MPIR_Bcast_binomial_MVP, -1},                            \
              {16384, &MPIR_Bcast_binomial_MVP, -1},                           \
              {32768, &MPIR_Bcast_binomial_MVP, -1},                           \
              {65536, &MPIR_Bcast_binomial_MVP, -1},                           \
              {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {262144, &MPIR_Bcast_binomial_MVP, -1},                          \
              {524288, &MPIR_Bcast_binomial_MVP, -1},                          \
              {1048576, &MPIR_Bcast_binomial_MVP, 2}},                         \
             21,                                                               \
             {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                      \
              {2, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {4, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {8, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                     \
              {32, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                     \
              {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                    \
              {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                    \
              {512, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                    \
              {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                   \
              {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                   \
              {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                   \
              {8192, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {16384, &MPIR_Shmem_Bcast_MVP, -1},                              \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
              {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
              {131072, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                \
              {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                 \
              {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                 \
              {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 2}}},              \
            {16,                                                               \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {{1, &MPIR_Bcast_binomial_MVP, -1},                               \
              {2, &MPIR_Bcast_binomial_MVP, -1},                               \
              {4, &MPIR_Bcast_binomial_MVP, -1},                               \
              {8, &MPIR_Bcast_binomial_MVP, -1},                               \
              {16, &MPIR_Bcast_binomial_MVP, -1},                              \
              {32, &MPIR_Bcast_binomial_MVP, -1},                              \
              {64, &MPIR_Bcast_binomial_MVP, -1},                              \
              {128, &MPIR_Bcast_binomial_MVP, -1},                             \
              {256, &MPIR_Bcast_binomial_MVP, -1},                             \
              {512, &MPIR_Bcast_binomial_MVP, -1},                             \
              {1024, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {2048, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {16384, &MPIR_Bcast_binomial_MVP, -1},                           \
              {32768, &MPIR_Bcast_binomial_MVP, -1},                           \
              {65536, &MPIR_Bcast_binomial_MVP, -1},                           \
              {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {262144, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {524288, &MPIR_Bcast_binomial_MVP, -1},                          \
              {1048576, &MPIR_Bcast_binomial_MVP, 8}},                         \
             21,                                                               \
             {{1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                      \
              {4, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                      \
              {8, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                     \
              {32, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                     \
              {64, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                     \
              {128, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                    \
              {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                    \
              {512, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                    \
              {1024, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {2048, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                  \
              {4096, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {8192, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                  \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                  \
              {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},                  \
              {131072, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                \
              {262144, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                \
              {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                 \
              {1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 8}}},              \
        {                                                                      \
            32, 8192, 4, 4, {1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,                  \
                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                    \
                21,                                                            \
                {{1, &MPIR_Bcast_binomial_MVP, -1},                            \
                 {2, &MPIR_Bcast_binomial_MVP, -1},                            \
                 {4, &MPIR_Bcast_binomial_MVP, -1},                            \
                 {8, &MPIR_Bcast_binomial_MVP, -1},                            \
                 {16, &MPIR_Bcast_binomial_MVP, -1},                           \
                 {32, &MPIR_Bcast_binomial_MVP, -1},                           \
                 {64, &MPIR_Bcast_binomial_MVP, -1},                           \
                 {128, &MPIR_Bcast_binomial_MVP, -1},                          \
                 {256, &MPIR_Bcast_binomial_MVP, -1},                          \
                 {512, &MPIR_Bcast_binomial_MVP, -1},                          \
                 {1024, &MPIR_Bcast_binomial_MVP, -1},                         \
                 {2048, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},       \
                 {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},       \
                 {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},       \
                 {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},      \
                 {32768, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},      \
                 {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},      \
                 {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},     \
                 {262144, &MPIR_Bcast_binomial_MVP, -1},                       \
                 {524288, &MPIR_Bcast_binomial_MVP, -1},                       \
                 {1048576, &MPIR_Bcast_binomial_MVP, 4}},                      \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Knomial_Bcast_intra_node_MVP, 8},                    \
                    {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {4, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {8, &MPIR_Shmem_Bcast_MVP, -1},                            \
                    {16, &MPIR_Knomial_Bcast_intra_node_MVP, 8},               \
                    {32, &MPIR_Knomial_Bcast_intra_node_MVP, 2},               \
                    {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},              \
                    {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},              \
                    {512, &MPIR_Knomial_Bcast_intra_node_MVP, 8},              \
                    {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 2},             \
                    {2048, &MPIR_Shmem_Bcast_MVP, -1},                         \
                    {4096, &MPIR_Knomial_Bcast_intra_node_MVP, -1},            \
                    {8192, &MPIR_Shmem_Bcast_MVP, -1},                         \
                    {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},           \
                    {32768, &MPIR_Knomial_Bcast_intra_node_MVP, -1},           \
                    {65536, &MPIR_Knomial_Bcast_intra_node_MVP, -1},           \
                    {131072, &MPIR_Knomial_Bcast_intra_node_MVP, -1},          \
                    {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                    {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 8},           \
                {                                                              \
                    1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 4             \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__2PPN_CNT 5

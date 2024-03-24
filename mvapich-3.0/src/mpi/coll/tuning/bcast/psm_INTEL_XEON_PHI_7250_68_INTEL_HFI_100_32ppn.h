#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__32PPN                         \
    {                                                                          \
        {32,                                                                   \
         8192,                                                                 \
         4,                                                                    \
         4,                                                                    \
         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},      \
         21,                                                                   \
         {{1, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                     \
          {2, &MPIR_Bcast_binomial_MVP, -1},                                   \
          {4, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                     \
          {8, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                     \
          {16, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                    \
          {32, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                    \
          {64, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},                \
          {128, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},               \
          {256, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                   \
          {512, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                   \
          {1024, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                  \
          {2048, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                  \
          {4096, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                  \
          {8192, &MPIR_Bcast_binomial_MVP, -1},                                \
          {16384, &MPIR_Bcast_binomial_MVP, -1},                               \
          {32768, &MPIR_Bcast_binomial_MVP, -1},                               \
          {65536, &MPIR_Bcast_scatter_ring_allgather_MVP, -1},                 \
          {131072, &MPIR_Bcast_binomial_MVP, -1},                              \
          {262144, &MPIR_Bcast_binomial_MVP, -1},                              \
          {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},            \
          {1048576, &MPIR_Bcast_binomial_MVP, -1}},                            \
         21,                                                                   \
         {{1, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {2, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {4, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {8, &MPIR_Shmem_Bcast_MVP, -1},                                      \
          {16, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {32, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {64, &MPIR_Shmem_Bcast_MVP, -1},                                     \
          {128, &MPIR_Shmem_Bcast_MVP, -1},                                    \
          {256, &MPIR_Shmem_Bcast_MVP, -1},                                    \
          {512, &MPIR_Shmem_Bcast_MVP, -1},                                    \
          {1024, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {2048, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {4096, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {8192, &MPIR_Shmem_Bcast_MVP, -1},                                   \
          {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                     \
          {32768, &MPIR_Shmem_Bcast_MVP, -1},                                  \
          {65536, &MPIR_Shmem_Bcast_MVP, -1},                                  \
          {131072, &MPIR_Shmem_Bcast_MVP, -1},                                 \
          {262144, &MPIR_Shmem_Bcast_MVP, -1},                                 \
          {524288, &MPIR_Shmem_Bcast_MVP, -1},                                 \
          {1048576, &MPIR_Shmem_Bcast_MVP, -1}}},                              \
            {64,                                                               \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
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
              {32768, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {65536, &MPIR_Bcast_binomial_MVP, -1},                           \
              {131072, &MPIR_Bcast_binomial_MVP, -1},                          \
              {262144, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {1048576, &MPIR_Bcast_binomial_MVP, -1}},                        \
             21,                                                               \
             {{1, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {2, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {4, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {8, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {16, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {32, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {64, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {128, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {256, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {512, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {1024, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {2048, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {4096, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {8192, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {65536, &MPIR_Shmem_Bcast_MVP, -1},                              \
              {131072, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {262144, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {524288, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {1048576, &MPIR_Shmem_Bcast_MVP, -1}}},                          \
            {128,                                                              \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
             21,                                                               \
             {{1, &MPIR_Bcast_binomial_MVP, -1},                               \
              {2, &MPIR_Bcast_binomial_MVP, -1},                               \
              {4, &MPIR_Bcast_binomial_MVP, -1},                               \
              {8, &MPIR_Bcast_binomial_MVP, -1},                               \
              {16, &MPIR_Bcast_binomial_MVP, -1},                              \
              {32, &MPIR_Bcast_binomial_MVP, -1},                              \
              {64, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},            \
              {128, &MPIR_Bcast_binomial_MVP, -1},                             \
              {256, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},           \
              {512, &MPIR_Bcast_binomial_MVP, -1},                             \
              {1024, &MPIR_Bcast_binomial_MVP, -1},                            \
              {2048, &MPIR_Bcast_binomial_MVP, -1},                            \
              {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {32768, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {262144, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {1048576, &MPIR_Bcast_binomial_MVP, -1}},                        \
             21,                                                               \
             {{1, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {2, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {4, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {8, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {16, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {32, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {64, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {128, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {256, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {512, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {1024, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {2048, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {4096, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {8192, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {65536, &MPIR_Shmem_Bcast_MVP, -1},                              \
              {131072, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {262144, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {524288, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {1048576, &MPIR_Shmem_Bcast_MVP, -1}}},                          \
            {256,                                                              \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  \
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
              {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
              {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {32768, &MPIR_Bcast_binomial_MVP, -1},                           \
              {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
              {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {262144, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
              {1048576, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1}},      \
             21,                                                               \
             {{1, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {2, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {4, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {8, &MPIR_Shmem_Bcast_MVP, -1},                                  \
              {16, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {32, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {64, &MPIR_Shmem_Bcast_MVP, -1},                                 \
              {128, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {256, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {512, &MPIR_Shmem_Bcast_MVP, -1},                                \
              {1024, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {2048, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {4096, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {8192, &MPIR_Shmem_Bcast_MVP, -1},                               \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP, -1},                 \
              {65536, &MPIR_Shmem_Bcast_MVP, -1},                              \
              {131072, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {262144, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {524288, &MPIR_Shmem_Bcast_MVP, -1},                             \
              {1048576, &MPIR_Shmem_Bcast_MVP, -1}}},                          \
        {                                                                      \
            512, 8192, 4, 4, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                 \
                              1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                   \
                21,                                                            \
                {{1, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
                 {2, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},          \
                 {4, &MPIR_Bcast_binomial_MVP, -1},                            \
                 {8, &MPIR_Bcast_binomial_MVP, -1},                            \
                 {16, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},         \
                 {32, &MPIR_Bcast_binomial_MVP, -1},                           \
                 {64, &MPIR_Bcast_binomial_MVP, -1},                           \
                 {128, &MPIR_Bcast_binomial_MVP, -1},                          \
                 {256, &MPIR_Bcast_binomial_MVP, -1},                          \
                 {512, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},        \
                 {1024, &MPIR_Bcast_binomial_MVP, -1},                         \
                 {2048, &MPIR_Bcast_binomial_MVP, -1},                         \
                 {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},       \
                 {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},       \
                 {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},      \
                 {32768, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},      \
                 {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},      \
                 {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},     \
                 {262144, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},     \
                 {524288, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP, -1},     \
                 {1048576, &MPIR_Bcast_binomial_MVP, -1}},                     \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Shmem_Bcast_MVP, -1},                                \
                    {2, &MPIR_Shmem_Bcast_MVP, -1},                            \
                    {4, &MPIR_Shmem_Bcast_MVP, -1},                            \
                    {8, &MPIR_Shmem_Bcast_MVP, -1},                            \
                    {16, &MPIR_Shmem_Bcast_MVP, -1},                           \
                    {32, &MPIR_Shmem_Bcast_MVP, -1},                           \
                    {64, &MPIR_Shmem_Bcast_MVP, -1},                           \
                    {128, &MPIR_Shmem_Bcast_MVP, -1},                          \
                    {256, &MPIR_Shmem_Bcast_MVP, -1},                          \
                    {512, &MPIR_Shmem_Bcast_MVP, -1},                          \
                    {1024, &MPIR_Shmem_Bcast_MVP, -1},                         \
                    {2048, &MPIR_Shmem_Bcast_MVP, -1},                         \
                    {4096, &MPIR_Shmem_Bcast_MVP, -1},                         \
                    {8192, &MPIR_Shmem_Bcast_MVP, -1},                         \
                    {16384, &MPIR_Knomial_Bcast_intra_node_MVP, -1},           \
                    {32768, &MPIR_Knomial_Bcast_intra_node_MVP, -1},           \
                    {65536, &MPIR_Shmem_Bcast_MVP, -1},                        \
                    {131072, &MPIR_Shmem_Bcast_MVP, -1},                       \
                    {262144, &MPIR_Shmem_Bcast_MVP, -1},                       \
                    {524288, &MPIR_Shmem_Bcast_MVP, -1},                       \
                {                                                              \
                    1048576, &MPIR_Shmem_Bcast_MVP, -1                         \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define PSM__INTEL_XEON_PHI_7250__INTEL_HFI_100__32PPN_CNT 5

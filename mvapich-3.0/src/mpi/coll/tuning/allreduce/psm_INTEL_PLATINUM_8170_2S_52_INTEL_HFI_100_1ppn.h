#define PSM__INTEL_PLATINUM_8170_2S_52__INTEL_HFI_100__1PPN                    \
    {                                                                          \
        {2,                                                                    \
         0,                                                                    \
         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},            \
         19,                                                                   \
         {{4, &MPIR_Allreduce_pt2pt_rd_MVP},                                   \
          {8, &MPIR_Allreduce_pt2pt_rd_MVP},                                   \
          {16, &MPIR_Allreduce_pt2pt_rd_MVP},                                  \
          {32, &MPIR_Allreduce_pt2pt_rd_MVP},                                  \
          {64, &MPIR_Allreduce_pt2pt_rd_MVP},                                  \
          {128, &MPIR_Allreduce_pt2pt_rd_MVP},                                 \
          {256, &MPIR_Allreduce_pt2pt_rd_MVP},                                 \
          {512, &MPIR_Allreduce_pt2pt_rd_MVP},                                 \
          {1024, &MPIR_Allreduce_pt2pt_rd_MVP},                                \
          {2048, &MPIR_Allreduce_pt2pt_rd_MVP},                                \
          {4096, &MPIR_Allreduce_pt2pt_rd_MVP},                                \
          {8192, &MPIR_Allreduce_pt2pt_rd_MVP},                                \
          {16384, &MPIR_Allreduce_pt2pt_rs_MVP},                               \
          {32768, &MPIR_Allreduce_pt2pt_rd_MVP},                               \
          {65536, &MPIR_Allreduce_pt2pt_rd_MVP},                               \
          {131072, &MPIR_Allreduce_pt2pt_rd_MVP},                              \
          {262144, &MPIR_Allreduce_pt2pt_rd_MVP},                              \
          {524288, &MPIR_Allreduce_pt2pt_rs_MVP},                              \
          {1048576, &MPIR_Allreduce_pt2pt_rs_MVP}},                            \
         19,                                                                   \
         {{4, &MPIR_Allreduce_reduce_p2p_MVP},                                 \
          {8, &MPIR_Allreduce_reduce_p2p_MVP},                                 \
          {16, &MPIR_Allreduce_reduce_p2p_MVP},                                \
          {32, &MPIR_Allreduce_reduce_p2p_MVP},                                \
          {64, &MPIR_Allreduce_reduce_p2p_MVP},                                \
          {128, &MPIR_Allreduce_reduce_p2p_MVP},                               \
          {256, &MPIR_Allreduce_reduce_p2p_MVP},                               \
          {512, &MPIR_Allreduce_reduce_p2p_MVP},                               \
          {1024, &MPIR_Allreduce_reduce_p2p_MVP},                              \
          {2048, &MPIR_Allreduce_reduce_p2p_MVP},                              \
          {4096, &MPIR_Allreduce_reduce_p2p_MVP},                              \
          {8192, &MPIR_Allreduce_reduce_p2p_MVP},                              \
          {16384, &MPIR_Allreduce_reduce_p2p_MVP},                             \
          {32768, &MPIR_Allreduce_reduce_p2p_MVP},                             \
          {65536, &MPIR_Allreduce_reduce_p2p_MVP},                             \
          {131072, &MPIR_Allreduce_reduce_p2p_MVP},                            \
          {262144, &MPIR_Allreduce_reduce_p2p_MVP},                            \
          {524288, &MPIR_Allreduce_reduce_p2p_MVP},                            \
          {1048576, &MPIR_Allreduce_reduce_p2p_MVP}}},                         \
            {4,                                                                \
             0,                                                                \
             {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        \
             19,                                                               \
             {{4, &MPIR_Allreduce_pt2pt_rs_MVP},                               \
              {8, &MPIR_Allreduce_pt2pt_rd_MVP},                               \
              {16, &MPIR_Allreduce_pt2pt_rd_MVP},                              \
              {32, &MPIR_Allreduce_pt2pt_rd_MVP},                              \
              {64, &MPIR_Allreduce_pt2pt_rd_MVP},                              \
              {128, &MPIR_Allreduce_pt2pt_rd_MVP},                             \
              {256, &MPIR_Allreduce_pt2pt_rd_MVP},                             \
              {512, &MPIR_Allreduce_pt2pt_rd_MVP},                             \
              {1024, &MPIR_Allreduce_pt2pt_rd_MVP},                            \
              {2048, &MPIR_Allreduce_pt2pt_rd_MVP},                            \
              {4096, &MPIR_Allreduce_pt2pt_rd_MVP},                            \
              {8192, &MPIR_Allreduce_pt2pt_rd_MVP},                            \
              {16384, &MPIR_Allreduce_pt2pt_rs_MVP},                           \
              {32768, &MPIR_Allreduce_pt2pt_rs_MVP},                           \
              {65536, &MPIR_Allreduce_pt2pt_rs_MVP},                           \
              {131072, &MPIR_Allreduce_pt2pt_rs_MVP},                          \
              {262144, &MPIR_Allreduce_pt2pt_rs_MVP},                          \
              {524288, &MPIR_Allreduce_pt2pt_rs_MVP},                          \
              {1048576, &MPIR_Allreduce_pt2pt_rs_MVP}},                        \
             19,                                                               \
             {{4, &MPIR_Allreduce_reduce_p2p_MVP},                             \
              {8, &MPIR_Allreduce_reduce_p2p_MVP},                             \
              {16, &MPIR_Allreduce_reduce_p2p_MVP},                            \
              {32, &MPIR_Allreduce_reduce_p2p_MVP},                            \
              {64, &MPIR_Allreduce_reduce_shmem_MVP},                          \
              {128, &MPIR_Allreduce_reduce_p2p_MVP},                           \
              {256, &MPIR_Allreduce_reduce_p2p_MVP},                           \
              {512, &MPIR_Allreduce_reduce_p2p_MVP},                           \
              {1024, &MPIR_Allreduce_reduce_p2p_MVP},                          \
              {2048, &MPIR_Allreduce_reduce_p2p_MVP},                          \
              {4096, &MPIR_Allreduce_reduce_p2p_MVP},                          \
              {8192, &MPIR_Allreduce_reduce_p2p_MVP},                          \
              {16384, &MPIR_Allreduce_reduce_p2p_MVP},                         \
              {32768, &MPIR_Allreduce_reduce_p2p_MVP},                         \
              {65536, &MPIR_Allreduce_reduce_p2p_MVP},                         \
              {131072, &MPIR_Allreduce_reduce_p2p_MVP},                        \
              {262144, &MPIR_Allreduce_reduce_p2p_MVP},                        \
              {524288, &MPIR_Allreduce_reduce_p2p_MVP},                        \
              {1048576, &MPIR_Allreduce_reduce_p2p_MVP}}},                     \
        {                                                                      \
            8, 0, {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},   \
                19, {{4, &MPIR_Allreduce_pt2pt_rd_MVP},                        \
                     {8, &MPIR_Allreduce_pt2pt_rs_MVP},                        \
                     {16, &MPIR_Allreduce_pt2pt_rs_MVP},                       \
                     {32, &MPIR_Allreduce_pt2pt_rd_MVP},                       \
                     {64, &MPIR_Allreduce_pt2pt_rd_MVP},                       \
                     {128, &MPIR_Allreduce_pt2pt_rd_MVP},                      \
                     {256, &MPIR_Allreduce_pt2pt_rd_MVP},                      \
                     {512, &MPIR_Allreduce_pt2pt_rd_MVP},                      \
                     {1024, &MPIR_Allreduce_pt2pt_rd_MVP},                     \
                     {2048, &MPIR_Allreduce_pt2pt_rd_MVP},                     \
                     {4096, &MPIR_Allreduce_pt2pt_rd_MVP},                     \
                     {8192, &MPIR_Allreduce_pt2pt_rd_MVP},                     \
                     {16384, &MPIR_Allreduce_pt2pt_rs_MVP},                    \
                     {32768, &MPIR_Allreduce_pt2pt_rs_MVP},                    \
                     {65536, &MPIR_Allreduce_pt2pt_rs_MVP},                    \
                     {131072, &MPIR_Allreduce_pt2pt_rs_MVP},                   \
                     {262144, &MPIR_Allreduce_pt2pt_rs_MVP},                   \
                     {524288, &MPIR_Allreduce_pt2pt_rs_MVP},                   \
                     {1048576, &MPIR_Allreduce_pt2pt_rs_MVP}},                 \
                19,                                                            \
            {                                                                  \
                {4, &MPIR_Allreduce_reduce_p2p_MVP},                           \
                    {8, &MPIR_Allreduce_reduce_p2p_MVP},                       \
                    {16, &MPIR_Allreduce_reduce_p2p_MVP},                      \
                    {32, &MPIR_Allreduce_reduce_shmem_MVP},                    \
                    {64, &MPIR_Allreduce_reduce_shmem_MVP},                    \
                    {128, &MPIR_Allreduce_reduce_p2p_MVP},                     \
                    {256, &MPIR_Allreduce_reduce_p2p_MVP},                     \
                    {512, &MPIR_Allreduce_reduce_p2p_MVP},                     \
                    {1024, &MPIR_Allreduce_reduce_shmem_MVP},                  \
                    {2048, &MPIR_Allreduce_reduce_p2p_MVP},                    \
                    {4096, &MPIR_Allreduce_reduce_p2p_MVP},                    \
                    {8192, &MPIR_Allreduce_pt2pt_rd_MVP},                      \
                    {16384, &MPIR_Allreduce_reduce_p2p_MVP},                   \
                    {32768, &MPIR_Allreduce_reduce_shmem_MVP},                 \
                    {65536, &MPIR_Allreduce_reduce_p2p_MVP},                   \
                    {131072, &MPIR_Allreduce_reduce_p2p_MVP},                  \
                    {262144, &MPIR_Allreduce_reduce_p2p_MVP},                  \
                    {524288, &MPIR_Allreduce_reduce_p2p_MVP},                  \
                {                                                              \
                    1048576, &MPIR_Allreduce_reduce_p2p_MVP                    \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define PSM__INTEL_PLATINUM_8170_2S_52__INTEL_HFI_100__1PPN_CNT 3

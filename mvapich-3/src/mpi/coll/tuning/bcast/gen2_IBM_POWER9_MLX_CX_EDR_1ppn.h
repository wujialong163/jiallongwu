#define GEN2__IBM_POWER9__MLX_CX_EDR__1PPN                                     \
    {                                                                          \
        {2,                                                                    \
         8192,                                                                 \
         4,                                                                    \
         4,                                                                    \
         {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},      \
         21,                                                                   \
         {{1, &MPIR_Bcast_binomial_MVP},                                       \
          {2, &MPIR_Bcast_binomial_MVP},                                       \
          {4, &MPIR_Bcast_binomial_MVP},                                       \
          {8, &MPIR_Bcast_binomial_MVP},                                       \
          {16, &MPIR_Bcast_binomial_MVP},                                      \
          {32, &MPIR_Bcast_binomial_MVP},                                      \
          {64, &MPIR_Bcast_binomial_MVP},                                      \
          {128, &MPIR_Bcast_binomial_MVP},                                     \
          {256, &MPIR_Bcast_binomial_MVP},                                     \
          {512, &MPIR_Bcast_binomial_MVP},                                     \
          {1024, &MPIR_Bcast_binomial_MVP},                                    \
          {2048, &MPIR_Bcast_binomial_MVP},                                    \
          {4096, &MPIR_Bcast_binomial_MVP},                                    \
          {8192, &MPIR_Bcast_binomial_MVP},                                    \
          {16384, &MPIR_Bcast_binomial_MVP},                                   \
          {32768, &MPIR_Bcast_binomial_MVP},                                   \
          {65536, &MPIR_Bcast_binomial_MVP},                                   \
          {131072, &MPIR_Bcast_binomial_MVP},                                  \
          {262144, &MPIR_Bcast_binomial_MVP},                                  \
          {524288, &MPIR_Bcast_binomial_MVP},                                  \
          {1048576, &MPIR_Bcast_binomial_MVP}},                                \
         21,                                                                   \
         {{1, &MPIR_Shmem_Bcast_MVP},      {2, &MPIR_Shmem_Bcast_MVP},         \
          {4, &MPIR_Shmem_Bcast_MVP},      {8, &MPIR_Shmem_Bcast_MVP},         \
          {16, &MPIR_Shmem_Bcast_MVP},     {32, &MPIR_Shmem_Bcast_MVP},        \
          {64, &MPIR_Shmem_Bcast_MVP},     {128, &MPIR_Shmem_Bcast_MVP},       \
          {256, &MPIR_Shmem_Bcast_MVP},    {512, &MPIR_Shmem_Bcast_MVP},       \
          {1024, &MPIR_Shmem_Bcast_MVP},   {2048, &MPIR_Shmem_Bcast_MVP},      \
          {4096, &MPIR_Shmem_Bcast_MVP},   {8192, &MPIR_Shmem_Bcast_MVP},      \
          {16384, &MPIR_Shmem_Bcast_MVP},  {32768, &MPIR_Shmem_Bcast_MVP},     \
          {65536, &MPIR_Shmem_Bcast_MVP},  {131072, &MPIR_Shmem_Bcast_MVP},    \
          {262144, &MPIR_Shmem_Bcast_MVP}, {524288, &MPIR_Shmem_Bcast_MVP},    \
          {1048576, &MPIR_Shmem_Bcast_MVP}}},                                  \
            {4,                                                                \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},  \
             21,                                                               \
             {{1, &MPIR_Bcast_binomial_MVP},                                   \
              {2, &MPIR_Bcast_binomial_MVP},                                   \
              {4, &MPIR_Bcast_binomial_MVP},                                   \
              {8, &MPIR_Bcast_binomial_MVP},                                   \
              {16, &MPIR_Bcast_binomial_MVP},                                  \
              {32, &MPIR_Bcast_binomial_MVP},                                  \
              {64, &MPIR_Bcast_binomial_MVP},                                  \
              {128, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},               \
              {256, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},               \
              {512, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},               \
              {1024, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {2048, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},             \
              {32768, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},             \
              {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},             \
              {131072, &MPIR_Bcast_binomial_MVP},                              \
              {262144, &MPIR_Bcast_binomial_MVP},                              \
              {524288, &MPIR_Bcast_binomial_MVP},                              \
              {1048576, &MPIR_Bcast_binomial_MVP}},                            \
             21,                                                               \
             {{1, &MPIR_Shmem_Bcast_MVP},                                      \
              {2, &MPIR_Shmem_Bcast_MVP},                                      \
              {4, &MPIR_Shmem_Bcast_MVP},                                      \
              {8, &MPIR_Shmem_Bcast_MVP},                                      \
              {16, &MPIR_Shmem_Bcast_MVP},                                     \
              {32, &MPIR_Shmem_Bcast_MVP},                                     \
              {64, &MPIR_Shmem_Bcast_MVP},                                     \
              {128, &MPIR_Shmem_Bcast_MVP},                                    \
              {256, &MPIR_Shmem_Bcast_MVP},                                    \
              {512, &MPIR_Shmem_Bcast_MVP},                                    \
              {1024, &MPIR_Shmem_Bcast_MVP},                                   \
              {2048, &MPIR_Shmem_Bcast_MVP},                                   \
              {4096, &MPIR_Shmem_Bcast_MVP},                                   \
              {8192, &MPIR_Shmem_Bcast_MVP},                                   \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP},                     \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP},                     \
              {65536, &MPIR_Knomial_Bcast_intra_node_MVP},                     \
              {131072, &MPIR_Shmem_Bcast_MVP},                                 \
              {262144, &MPIR_Shmem_Bcast_MVP},                                 \
              {524288, &MPIR_Shmem_Bcast_MVP},                                 \
              {1048576, &MPIR_Shmem_Bcast_MVP}}},                              \
            {8,                                                                \
             8192,                                                             \
             4,                                                                \
             4,                                                                \
             {1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},  \
             21,                                                               \
             {{1, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},                 \
              {2, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},                 \
              {4, &MPIR_Bcast_binomial_MVP},                                   \
              {8, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},                 \
              {16, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},                \
              {32, &MPIR_Bcast_binomial_MVP},                                  \
              {64, &MPIR_Bcast_binomial_MVP},                                  \
              {128, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},               \
              {256, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},               \
              {512, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},               \
              {1024, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {2048, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {4096, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {8192, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},              \
              {16384, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},             \
              {32768, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},             \
              {65536, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},             \
              {131072, &MPIR_Knomial_Bcast_inter_node_wrapper_MVP},            \
              {262144, &MPIR_Bcast_binomial_MVP},                              \
              {524288, &MPIR_Bcast_binomial_MVP},                              \
              {1048576, &MPIR_Bcast_binomial_MVP}},                            \
             21,                                                               \
             {{1, &MPIR_Shmem_Bcast_MVP},                                      \
              {2, &MPIR_Shmem_Bcast_MVP},                                      \
              {4, &MPIR_Shmem_Bcast_MVP},                                      \
              {8, &MPIR_Shmem_Bcast_MVP},                                      \
              {16, &MPIR_Shmem_Bcast_MVP},                                     \
              {32, &MPIR_Shmem_Bcast_MVP},                                     \
              {64, &MPIR_Shmem_Bcast_MVP},                                     \
              {128, &MPIR_Knomial_Bcast_intra_node_MVP},                       \
              {256, &MPIR_Shmem_Bcast_MVP},                                    \
              {512, &MPIR_Shmem_Bcast_MVP},                                    \
              {1024, &MPIR_Shmem_Bcast_MVP},                                   \
              {2048, &MPIR_Shmem_Bcast_MVP},                                   \
              {4096, &MPIR_Knomial_Bcast_intra_node_MVP},                      \
              {8192, &MPIR_Shmem_Bcast_MVP},                                   \
              {16384, &MPIR_Knomial_Bcast_intra_node_MVP},                     \
              {32768, &MPIR_Knomial_Bcast_intra_node_MVP},                     \
              {65536, &MPIR_Knomial_Bcast_intra_node_MVP},                     \
              {131072, &MPIR_Knomial_Bcast_intra_node_MVP},                    \
              {262144, &MPIR_Knomial_Bcast_intra_node_MVP},                    \
              {524288, &MPIR_Shmem_Bcast_MVP},                                 \
              {1048576, &MPIR_Knomial_Bcast_intra_node_MVP}}},                 \
        {                                                                      \
            16, 8192, 4, 4, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                  \
                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1},                    \
                21,                                                            \
                {{1, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {2, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {4, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {8, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                       \
                 {16, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                      \
                 {32, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                      \
                 {64, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                      \
                 {128, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {256, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {512, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                     \
                 {1024, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {2048, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {4096, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {8192, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                    \
                 {16384, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                 {32768, &MPIR_Pipelined_Bcast_Zcpy_MVP, 4},                   \
                 {65536, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                   \
                 {131072, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {262144, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {524288, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2},                  \
                 {1048576, &MPIR_Pipelined_Bcast_Zcpy_MVP, 2}},                \
                21,                                                            \
            {                                                                  \
                {1, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                    \
                    {2, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {4, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {8, &MPIR_Knomial_Bcast_intra_node_MVP, 4},                \
                    {16, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {32, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {64, &MPIR_Knomial_Bcast_intra_node_MVP, 4},               \
                    {128, &MPIR_Knomial_Bcast_intra_node_MVP, 4},              \
                    {256, &MPIR_Knomial_Bcast_intra_node_MVP, 4},              \
                    {512, &MPIR_Knomial_Bcast_intra_node_MVP, 4},              \
                    {1024, &MPIR_Knomial_Bcast_intra_node_MVP, 4},             \
                    {2048, &MPIR_Knomial_Bcast_intra_node_MVP, 4},             \
                    {4096, &MPIR_Knomial_Bcast_intra_node_MVP, 4},             \
                    {8192, &MPIR_Knomial_Bcast_intra_node_MVP, 4},             \
                    {16384, &MPIR_Knomial_Bcast_intra_node_MVP, 4},            \
                    {32768, &MPIR_Knomial_Bcast_intra_node_MVP, 4},            \
                    {65536, &MPIR_Knomial_Bcast_intra_node_MVP, 2},            \
                    {131072, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                    {262144, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                    {524288, &MPIR_Knomial_Bcast_intra_node_MVP, 2},           \
                {                                                              \
                    1048576, &MPIR_Knomial_Bcast_intra_node_MVP, 2             \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__IBM_POWER9__MLX_CX_EDR__1PPN_CNT 4
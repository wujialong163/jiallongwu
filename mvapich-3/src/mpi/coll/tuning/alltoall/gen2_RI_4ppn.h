#define GEN2__INTEL_XEON_E5630_8__MLX_CX_QDR__4PPN                             \
    {                                                                          \
        {                                                                      \
            4, {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 16,           \
            {                                                                  \
                {1, &MPIR_Alltoall_RD_MVP}, {2, &MPIR_Alltoall_RD_MVP},        \
                    {4, &MPIR_Alltoall_RD_MVP},                                \
                    {8, &MPIR_Alltoall_Scatter_dest_MVP},                      \
                    {16, &MPIR_Alltoall_Scatter_dest_MVP},                     \
                    {32, &MPIR_Alltoall_Scatter_dest_MVP},                     \
                    {64, &MPIR_Alltoall_Scatter_dest_MVP},                     \
                    {128, &MPIR_Alltoall_Scatter_dest_MVP},                    \
                    {256, &MPIR_Alltoall_Scatter_dest_MVP},                    \
                    {512, &MPIR_Alltoall_Scatter_dest_MVP},                    \
                    {1024, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {2048, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {4096, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {8192, &MPIR_Alltoall_Scatter_dest_MVP},                   \
                    {16384, &MPIR_Alltoall_Scatter_dest_MVP},                  \
                {                                                              \
                    32768, &MPIR_Alltoall_Scatter_dest_MVP                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
    }
#define GEN2__INTEL_XEON_E5630_8__MLX_CX_QDR__4PPN_CNT 1

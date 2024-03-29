#include "mpiPi.h"
#include <stdio.h>
#include "mpiP-pvar.h"

void mpiP_pvar_start(int op){
    int i=0,count,pvar_index;
    switch (op)
    {
    case mpiPi_MPI_Allreduce:
        get_pvar_name();
        break;
    
    default:
        break;
    }
    while (pvar_name[i])
    {
        MPI_T_pvar_get_index(pvar_name[i],pvar_class,&pvar_index);
        MPI_T_pvar_handle_alloc(mpiPi.session,pvar_index,&mpiPi.comm,&mpiPi.handle[i],&count);
        MPI_T_pvar_start(mpiPi.session,mpiPi.handle[i]);
        if(pvar_class == MPI_T_PVAR_CLASS_TIMER){

        }

    }
    
}
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
/*
要读取变量内的值，如果用户有确定的变量名称则进行针对性的读取，如果没有，则读取有数据记录的变量的值。

*/
void mpiP_pvar_stop(int op){
    int i=0;
    while (i)
    {
        /* code */
        MPI_T_pvar_stop(mpiPi.session,mpiPi.handle[i]);
        MPI_T_pvar_read(mpiPi.session,mpiPi.handle[i],&buf);
    }

}
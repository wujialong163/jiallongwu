#include<stdio.h>
#include<string.h>
#include<mpi.h>

void foo(){
	int a ;
	for(int i =0; i < 1000 ; i++){
	  a = a+1;
	}
}

void main(int argc , char ** argv){
int a,b,c;
int messg=1000;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&a);
	MPI_Comm_size(MPI_COMM_WORLD,&b);
	
	if(a == 0){
	MPI_Send(&messg,1,MPI_INT,1,99,MPI_COMM_WORLD);
	}
	
	if(a == 1)
	{
	MPI_Recv(&c,1,MPI_INT,0,99,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	printf("%d" , c);
	if(c == 1000){
	printf("\n hellow");
	}
	else if(a == 10){
	printf("da ");
	}
	else{
	printf("ad");
	}
	foo();
	printf("\n a " );
	}
	
	MPI_Finalize();

}

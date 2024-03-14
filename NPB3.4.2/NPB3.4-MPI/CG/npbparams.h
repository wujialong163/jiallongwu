! CLASS = A
!  
!  
!  This file is generated automatically by the setparams utility.
!  It sets the number of processors and the class of the NPB
!  in this directory. Do not modify it by hand.
!  
        integer            na, nonzer, niter
        double precision   shift, rcond
        parameter(  na=14000, &
     &              nonzer=11, &
     &              niter=15, &
     &              shift=20., &
     &              rcond=1.0d-1 )
        logical  convertdouble
        parameter (convertdouble = .false.)
        character*11 compiletime
        parameter (compiletime='26 Oct 2023')
        character*5 npbversion
        parameter (npbversion='3.4.2')
        character*6 cs1
        parameter (cs1='mpif90')
        character*8 cs2
        parameter (cs2='$(MPIFC)')
        character*43 cs3
        parameter (cs3='-L /home/wujialong/software/mpiP-3.5 -lmpiP')
        character*45 cs4
        parameter (cs4='-I /home/wujialong/software/mpich-4.1/include')
        character*29 cs5
        parameter (cs5='-O3 -fallow-argument-mismatch')
        character*9 cs6
        parameter (cs6='$(FFLAGS)')
        character*6 cs7
        parameter (cs7='randi8')

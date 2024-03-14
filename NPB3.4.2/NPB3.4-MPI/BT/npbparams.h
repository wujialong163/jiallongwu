! CLASS = C
!  
!  
!  This file is generated automatically by the setparams utility.
!  It sets the number of processors and the class of the NPB
!  in this directory. Do not modify it by hand.
!  
        integer problem_size, niter_default
        parameter (problem_size=162, niter_default=200)
        double precision dt_default
        parameter (dt_default = 0.0001d0)
        integer wr_default
        parameter (wr_default = 5)
        integer iotype
        parameter (iotype = 0)
        logical  convertdouble
        parameter (convertdouble = .false.)
        character*11 compiletime
        parameter (compiletime='11 Sep 2023')
        character*5 npbversion
        parameter (npbversion='3.4.2')
        character*6 cs1
        parameter (cs1='mpif90')
        character*8 cs2
        parameter (cs2='$(MPIFC)')
        character*6 cs3
        parameter (cs3='(none)')
        character*41 cs4
        parameter (cs4='-I /home/wujialong/software/mpich/include')
        character*29 cs5
        parameter (cs5='-O3 -fallow-argument-mismatch')
        character*9 cs6
        parameter (cs6='$(FFLAGS)')
        character*6 cs7
        parameter (cs7='randi8')

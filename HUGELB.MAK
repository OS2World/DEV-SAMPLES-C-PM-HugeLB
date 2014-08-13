# ======================================================================== #
#                                                                          #
#    Makefile for HugeLB                                                   #
#                                                                          #
# ======================================================================== #
   

HugeLB.exe : HugeLB.obj  HugeLB.def HugeLB.res
     link386  /NOE /NOI /DE HugeLB,,,, HugeLB.def
     rc HugeLB.res  HugeLB.exe

HugeLB.obj : HugeLB.c HugeLB.h
     icc /Ss /C /Ti HugeLB.c

HugeLB.res : HugeLB.rc HugeLB.h 
     rc -r HugeLB  


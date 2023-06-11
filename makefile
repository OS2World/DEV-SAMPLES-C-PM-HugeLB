# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::GNU C
#  Make: GNU make
all : HugeLB.exe 

HugeLB.exe : HugeLB.obj  HugeLB.def HugeLB.res 
	gcc -Zomf HugeLB.obj  HugeLB.def HugeLB.res -o HugeLB.exe
	wrc HugeLB.res

HugeLB.obj : HugeLB.c HugeLB.h 
	gcc -Wall -Zomf -c -O2 HugeLB.c -o HugeLB.obj

HugeLB.res : HugeLB.rc 
	wrc -r HugeLB.rc

clean :
	rm -rf *exe *res *obj *dll *lib

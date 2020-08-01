ganesha1: ganesha1.o elephant1.o curtime.o wait.o
	gcc -o ganesha1 ganesha1.o curtime.o
	gcc -o elephant1 elephant1.o curtime.o wait.o

curtime.o: ../common/curtime.c ../common/curtime.h
	gcc -c ../common/curtime.c -o curtime.o

wait.o: ../common/wait.c ../common/wait.h
	gcc -c ../common/wait.c -o wait.o

ganesha1.o: ganesha1.c ../common/ganesha.h ../common/elephant.h ../common/curtime.h
	gcc -c ganesha1.c

elephant1.o: elephant1.c ../common/elephant.h ../common/wait.h ../common/curtime.h 
	gcc -c elephant1.c

clean:
	rm -f elephant1 ganesha1 elephant1.o ganesha1.o curtime.o wait.o 

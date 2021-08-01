CC = gcc
#CHANGEME: Wherever you installed Gaul, this should point to that location. 
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -L${GAUL_BASE}/lib -I${GAUL_BASE}/include 
CFLAGS = -g -O3
CLIBS = -lm
GAUL_LIBS = -lgaul -lgaul_util

test_simulation: run_simulation.o plane_physics.o test_simulation.c
	${CC} ${CFLAGS} -otest_simulation run_simulation.o plane_physics.o test_simulation.c ${CLIBS}

run_simulation.o: run_simulation.c run_simulation.h plane_physics.h plane_physics.o
	${CC} ${CFLAGS} -c -o run_simulation.o run_simulation.c ${CLIBS}
plane_physics.o: plane_physics.h plane_physics.c
	${CC} ${CFLAGS} -c -o plane_physics.o plane_physics.c ${CLIBS}

run_ga.o: run_ga.c run_simulation.o plane_physics.o boring_callbacks.c mixing_callbacks.c mutate_callbacks.c util_callbacks.c
	${CC} ${CFLAGS} ${GAUL_FLAGS} -c -o run_ga.o run_simulation.o plane_physics.o run_ga.c ${CLIBS} ${GAUL_LIBS}

ga_main: ga_main.c run_ga.o run_ga.h
	${CC} ${CFLAGS} ${GAUL_FLAGS} -oga_main ga_main.c run_ga.o run_simulation.o plane_physics.o ${CLIBS} ${GAUL_LIBS}

flurrie: ga_main playerdats/flurrie.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=4 ./ga_main --player-dat=playerdats/flurrie.dat --output-file=test_flurrie.txt --pop-size=100 --max-frames=300 --num-generations=100


gloomtail: ga_main playerdats/gloomtail.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=4 ./ga_main --player-dat=playerdats/gloomtail.dat --output-file=test_gloomtail.txt --pop-size=3000 --max-frames=900 --num-generations=100000

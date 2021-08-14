CC = gcc
#CHANGEME: Wherever you installed Gaul, this should point to that location. 
#Malleo's version:
#GAUL_BASE = /malleo/somewhere/i/dont/know/where/gaul-install
#MMTrebuchet's version:
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -L${GAUL_BASE}/lib -I${GAUL_BASE}/include 
CFLAGS = -g -O3
CLIBS = -lm
GAUL_LIBS = -lgaul -lgaul_util
GAUL_THREADS = 4

test_simulation: run_simulation.o plane_physics.o test_simulation.c
	${CC} ${CFLAGS} -otest_simulation run_simulation.o plane_physics.o test_simulation.c ${CLIBS}

run_simulation.o: run_simulation.c run_simulation.h plane_physics.h plane_physics.o
	${CC} ${CFLAGS} -c -o run_simulation.o run_simulation.c ${CLIBS}
plane_physics.o: plane_physics.h plane_physics.c
	${CC} ${CFLAGS} -c -o plane_physics.o plane_physics.c ${CLIBS}

run_ga.o: run_ga.c run_simulation.o plane_physics.o boring_callbacks.c mixing_callbacks.c mutate_callbacks.c util_callbacks.c
	${CC} ${CFLAGS} ${GAUL_FLAGS} -c -o run_ga.o run_ga.c ${CLIBS} ${GAUL_LIBS}

ga_main: ga_main.c run_ga.o run_ga.h
	${CC} ${CFLAGS} ${GAUL_FLAGS} -oga_main ga_main.c run_ga.o run_simulation.o plane_physics.o ${CLIBS} ${GAUL_LIBS}

flurrie: ga_main playerdats/flurrie.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/flurrie.dat \
		--output-file=test_flurrie.txt \
		--pop-size=100 \
		--max-frames=300 \
		--num-generations=100


gloomtail: ga_main playerdats/gloomtail.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/gloomtail.dat \
		--output-file=test_gloomtail.txt \
		--pop-size=3000 \
		--max-frames=900 \
		--num-generations=100000

grodus: ga_main playerdats/grodus.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/grodus.dat \
		--output-file=test_grodus.txt \
		--pop-size=2000 \
		--max-frames=300 \
		--num-generations=10000

blooper: ga_main playerdats/blooper.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/blooper.dat \
		--output-file=test_blooper.txt \
		--pop-size=3000 \
		--max-frames=100 \
		--num-generations=1000

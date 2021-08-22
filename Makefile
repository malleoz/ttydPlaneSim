CC = gcc
#CHANGEME: Wherever you installed Gaul, this should point to that location. 
#Malleo's version:
#GAUL_BASE = /malleo/somewhere/i/dont/know/where/gaul-install
#MMTrebuchet's version:
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -L${GAUL_BASE}/lib -I${GAUL_BASE}/include 
CFLAGS = -g -O3 -D ABOVE_PANEL
CLIBS = -lm
GAUL_LIBS = -lgaul -lgaul_util
GAUL_THREADS = 4
OBJ_DIR = obj
SRC_DIR = src

test_simulation: ${OBJ_DIR}/run_simulation.o ${OBJ_DIR}/plane_physics.o ${SRC_DIR}/test_simulation.c
	${CC} ${CFLAGS} -o$@ $^ ${CLIBS}

${OBJ_DIR}/run_simulation.o: ${SRC_DIR}/run_simulation.c ${SRC_DIR}/run_simulation.h ${SRC_DIR}/plane_physics.h ${OBJ_DIR}/plane_physics.o
	${CC} ${CFLAGS} -c -o$@ ${SRC_DIR}/run_simulation.c ${CLIBS}

${OBJ_DIR}/plane_physics.o: ${SRC_DIR}/plane_physics.h ${SRC_DIR}/plane_physics.c
	${CC} ${CFLAGS} -c -o$@ ${SRC_DIR}/plane_physics.c ${CLIBS}

${OBJ_DIR}/run_ga.o: ${SRC_DIR}/run_ga.c ${SRC_DIR}/run_ga.h ${OBJ_DIR}/run_simulation.o ${OBJ_DIR}/plane_physics.o ${SRC_DIR}/boring_callbacks.c ${SRC_DIR}/mixing_callbacks.c ${SRC_DIR}/mutate_callbacks.c ${SRC_DIR}/util_callbacks.c
	${CC} ${CFLAGS} ${GAUL_FLAGS} -c -o$@ ${SRC_DIR}/run_ga.c ${CLIBS} ${GAUL_LIBS}

ga_main: ${SRC_DIR}/ga_main.c ${OBJ_DIR}/run_ga.o ${OBJ_DIR}/run_simulation.o ${OBJ_DIR}/plane_physics.o
	${CC} ${CFLAGS} ${GAUL_FLAGS} -o$@ $^ ${CLIBS} ${GAUL_LIBS}

flurrie: ga_main playerdats/flurrie.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/flurrie.dat \
		--output-file=test_flurrie.txt \
		--pop-size=3000 \
		--max-frames=340 \
		--num-generations=50000

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

gloomtail_above_panel: ga_main playerdats/gloomtail_above_panel.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/gloomtail_above_panel.dat \
		--output-file=test_gloomtail_above_panel.txt \
		--pop-size=3000 \
		--max-frames=100 \
		--num-generations=1000


plane_game: ga_main playerdats/plane_game.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/plane_game.dat \
		--output-file=test_plane_game.txt \
		--pop-size=1000 \
		--max-frames=2000 \
		--num-generations=1000

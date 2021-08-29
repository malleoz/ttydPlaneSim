CC = gcc
#CHANGEME: Wherever you installed Gaul, this should point to that location. 
#Malleo's version:
#GAUL_BASE = /usr/local
#MMTrebuchet's version:
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -L${GAUL_BASE}/lib -I${GAUL_BASE}/include 
CFLAGS = -g -O3 -Wall -Wextra 
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

simulate_single: ${SRC_DIR}/simulate_single.c ${OBJ_DIR}/run_simulation.o ${OBJ_DIR}/plane_physics.o ${SRC_DIR}/run_simulation.h ${SRC_DIR}/plane_physics.h
	${CC} ${CFLAGS} -o$@ ${SRC_DIR}/simulate_single.c ${OBJ_DIR}/run_simulation.o ${OBJ_DIR}/plane_physics.o ${CLIBS}



# Prologue
tyd_east_room: ga_main playerdats/tyd_east_room.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/tyd_east_room.dat \
		--output-file=SimRes_tyd_east_room.txt \
		--pop-size=3000 \
		--max-frames=125 \
		--num-generations=200000

tyd_east_room_no_yellow_block: ga_main playerdats/tyd_east_room_no_yellow_block.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/tyd_east_room_no_yellow_block.dat \
		--output-file=SimRes_tyd_east_room_no_yellow_block.txt \
		--pop-size=3000 \
		--max-frames=125 \
		--num-generations=200000

tyd_east_room_tas: ga_main playerdats/tyd_east_room_tas.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/tyd_east_room_tas.dat \
		--output-file=SimRes_tyd_east_room_tas.txt \
		--pop-size=3000 \
		--max-frames=125 \
		--num-generations=200000

blooper: ga_main playerdats/blooper.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/blooper.dat \
		--output-file=simRes_blooper.txt \
		--pop-size=3000 \
		--max-frames=100 \
		--num-generations=1000


flurrie_unraised: ga_main playerdats/flurrie_unraised.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/flurrie_unraised.dat \
		--output-file=simRes_flurrie_unraised.txt \
		--pop-size=1500 \
		--max-frames=450 \
		--num-generations=50000

flurrie_raised: ga_main playerdats/flurrie_raised.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/flurrie_raised.dat \
		--output-file=simRes_flurrie_raised.txt \
		--pop-size=3000 \
		--max-frames=250 \
		--num-generations=50000

glitzville: ga_main playerdats/glitzville.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/glitzville.dat \
		--output-file=simRes_glitzville.txt \
		--pop-size=2000 \
		--max-frames=340 \
		--num-generations=200000

gloomtail: ga_main playerdats/gloomtail.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/gloomtail.dat \
		--output-file=simRes_gloomtail.txt \
		--pop-size=3000 \
		--max-frames=900 \
		--num-generations=100000

grodus: ga_main playerdats/grodus.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/grodus.dat \
		--output-file=simRes_grodus.txt \
		--pop-size=2000 \
		--max-frames=300 \
		--num-generations=10000

plane_game: ga_main playerdats/plane_game.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/plane_game.dat \
		--output-file=test_plane_game.txt \
		--pop-size=1000 \
		--max-frames=1500 \
		--num-generations=2000


blooper_tall: ga_main playerdats/blooper_tall.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=${GAUL_THREADS} ./ga_main \
		--player-dat=playerdats/blooper_tall.dat \
		--output-file=simRes_blooper_tall.txt \
		--pop-size=1000 \
		--max-frames=200 \
		--num-generations=1000

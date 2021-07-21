CC = gcc
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -lgaul -L${GAUL_BASE}/lib -I${GAUL_BASE}/include -lgaul_util -lm
#CFLAGS = -Wall -lm -g -Wextra -Werror -Wpedantic ${GAUL_FLAGS}
CFLAGS = -lm -g -O3 -fopenmp

test_simulation: run_simulation.o plane_physics.o test_simulation.c
	${CC} ${CFLAGS} -otest_simulation run_simulation.o plane_physics.o test_simulation.c

run_simulation.o: run_simulation.c run_simulation.h plane_physics.h plane_physics.o
	${CC} ${CFLAGS} -c -o run_simulation.o run_simulation.c 
plane_physics.o: plane_physics.h plane_physics.c
	${CC} ${CFLAGS} -c -o plane_physics.o plane_physics.c

runTestSimulation: test_simulation player.dat
	valgrind ./test_simulation < player.dat

player.dat: export_player.py ram.raw
	python3 export_player.py > player.dat

run_ga: run_ga.c run_simulation.o plane_physics.o boring_callbacks.c mixing_callbacks.c mutate_callbacks.c util_callbacks.c
	${CC} ${CFLAGS} ${GAUL_FLAGS} -orun_ga run_simulation.o plane_physics.o run_ga.c

test_run: run_ga player.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=2 ./run_ga sim_results_5.txt < player.dat

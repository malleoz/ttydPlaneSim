CC = gcc
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -lgaul -L${GAUL_BASE}/lib -I${GAUL_BASE}/include -lgaul_util -lm
#CFLAGS = -Wall -lm -g -Wextra -Werror -Wpedantic ${GAUL_FLAGS}
CFLAGS = -lm -g -O3

test_simulation: run_simulation.o plane_physics.o test_simulation.c
	${CC} ${CFLAGS} -otest_simulation run_simulation.o plane_physics.o test_simulation.c

run_simulation.o: run_simulation.c run_simulation.h plane_physics.h plane_physics.o
	${CC} ${CFLAGS} -c -o run_simulation.o run_simulation.c 
plane_physics.o: plane_physics.h plane_physics.c
	${CC} ${CFLAGS} -c -o plane_physics.o plane_physics.c

player.dat: export_player.py ram.raw
	python3 export_player.py > player.dat

run_ga.o: run_ga.c run_simulation.o plane_physics.o boring_callbacks.c mixing_callbacks.c mutate_callbacks.c util_callbacks.c
	${CC} ${CFLAGS} ${GAUL_FLAGS} -c -o run_ga.o run_simulation.o plane_physics.o run_ga.c

test_run: run_ga player.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=4 ./run_ga sim_results.txt < player.dat

ga_main: ga_main.c run_ga.o run_ga.h
	${CC} ${CFLAGS} ${GAUL_FLAGS} -oga_main ga_main.c run_ga.o run_simulation.o plane_physics.o

run_main: ga_main player.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=4 ./ga_main --player-dat=player.dat --output-file=sim_results_ref.csv --pop-size=10 --max-frames=300 --num-generations=10


flurrie: ga_main playerdats/flurrie.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=4 ./ga_main --player-dat=playerdats/flurrie.dat --output-file=test_flurrie.txt --pop-size=100 --max-frames=300 --num-generations=100


gloomtail: ga_main playerdats/gloomtail.dat
	LD_LIBRARY_PATH=${GAUL_BASE}/lib GAUL_NUM_THREADS=4 ./ga_main --player-dat=playerdats/gloomtail.dat --output-file=test_gloomtail.txt --pop-size=3000 --max-frames=900 --num-generations=100000

CC = gcc
GAUL_BASE = /home/pi/gaul-install
GAUL_FLAGS = -lgaul -L${GAUL_BASE}/lib -I${GAUL_BASE}/include -lgaul_util -lm
#CFLAGS = -Wall -lm -g -Wextra -Werror -Wpedantic ${GAUL_FLAGS}
CFLAGS = -lm -g  

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

basic_struggle: basic_test.c run_simulation.o plane_physics.o
	${CC} ${CFLAGS} ${GAUL_FLAGS} -obasic_struggle run_simulation.o plane_physics.o basic_test.c

test_basic: basic_struggle
	LD_LIBRARY_PATH=${GAUL_BASE}/lib ./basic_struggle < player.dat

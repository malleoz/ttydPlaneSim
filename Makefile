CC = gcc
CFLAGS = -Wall -lm -g -Wextra -Werror -Wpedantic

test_simulation: run_simulation.o plane_physics.o test_simulation.c
	${CC} ${CFLAGS} -otest_simulation run_simulation.o plane_physics.o test_simulation.c

run_simulation.o: run_simulation.c run_simulation.h plane_physics.h plane_physics.o
	${CC} ${CFLAGS} -c -o run_simulation.o run_simulation.c 
plane_physics.o: plane_physics.h plane_physics.c
	${CC} ${CFLAGS} -c -o plane_physics.o plane_physics.c

runTestSimulation: test_simulation player.dat
	./test_simulation < player.dat

player.dat: export_player.py ram.raw
	python3 export_player.py > player.dat

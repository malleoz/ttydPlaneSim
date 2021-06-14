GAUL_BASE = /home/pi/gaul-install
GAULFLAGS = -lgaul -L${GAUL_BASE}/lib -I${GAUL_BASE}/include -lgaul_util -lm

testSimulation: run_simulation.o sample_physics.o test_simulation.c
	cc -otest_simulation run_simulation.o sample_physics.o test_simulation.c

run_simulation.o: run_simulation.c run_simulation.h sample_physics.h sample_physics.o
	cc -c -o run_simulation.o run_simulation.c 
sample_physics.o: sample_physics.h sample_physics.c
	cc -c -o sample_physics.o sample_physics.c

struggle: struggle.c
	cc ${GAULFLAGS} -ostruggle struggle.c
runStruggle: struggle
	LD_LIBRARY_PATH=${GAUL_BASE}/lib ./struggle

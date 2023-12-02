clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-rm -rf ./build

plot:
	gnuplot CAdfinitasExample1.1.plt
	gnuplot CAdfinitasExample1.2.plt
	gnuplot CAdfinitasExample1.3.plt
	-mv ./*.gif build/
	-mv ./*.png build/

build: clear
	gcc CAdfinitasExample1.c ../../libCAdfinitas.c ../../libXi.c -o CAdfinitasExample1.run -lm -L../../
	mkdir -p build/data
	-mv ./*.run build/

run: build
	./build/CAdfinitasExample1.run
	-mv ./*.dat build/data/

all: clear run plot

build_mpi: clear
	mpicc -D_ADFINITAS_MPI CAdfinitasExample1.c ../../libCAdfinitas.c ../../libXi.c -o CAdfinitasExample1MPI.run -lm -L../../
	mkdir -p build/data
	-mv ./*.run build/

run_mpi: build_mpi
	mpirun ./build/CAdfinitasExample1MPI.run
	-mv ./*.dat build/data/

all_mpi: clear run_mpi plot

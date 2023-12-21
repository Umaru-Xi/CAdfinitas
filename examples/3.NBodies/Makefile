clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-rm -rf ./build

build: clear
	gcc -lm -I../../ -I../../../LibXi/ CAdfinitasExample3.c ../../libCAdfinitas.c ../../../LibXi/libXi.c -o CAdfinitasExample3.run
	mkdir -p build/data
	-mv ./*.run build/

run: build
	./build/CAdfinitasExample3.run
	-mv ./*.dat build/data/

plot:
	gnuplot CAdfinitasExample3.1.plt
	gnuplot CAdfinitasExample3.2.plt
	-mv ./*.gif build/
	-mv ./*.png build/

all: clear run plot

build_mpi: clear
	mpicc -D_XI_MPI -lm -I../../ -I../../../LibXi/ CAdfinitasExample3.c ../../libCAdfinitas.c ../../../LibXi/libXi.c -o CAdfinitasExample3MPI.run
	mkdir -p build/data
	-mv ./*.run build/

run_mpi: build_mpi
	mpirun ./build/CAdfinitasExample3MPI.run
	-mv ./*.dat build/data/

all_mpi: clear run_mpi plot

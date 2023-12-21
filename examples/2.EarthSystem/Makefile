clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-rm -rf ./build

build: clear
	gcc -lm -I../../ -I../../../LibXi/ CAdfinitasExample2.c ../../libCAdfinitas.c ../../../LibXi/libXi.c -o CAdfinitasExample2.run 
	mkdir -p build/data
	-mv ./*.run build/

run: build
	./build/CAdfinitasExample2.run
	-mv ./*.dat build/data/

plot:
	gnuplot CAdfinitasExample2.1.plt
	-mv ./*.gif build/
	-mv ./*.png build/

all: clear run plot

build_mpi: clear
	mpicc -D_XI_MPI -lm -I../../ -I../../../LibXi/ CAdfinitasExample2.c ../../libCAdfinitas.c ../../../LibXi/libXi.c -o CAdfinitasExample2MPI.run
	mkdir -p build/data
	-mv ./*.run build/

run_mpi: build_mpi
	mpirun ./build/CAdfinitasExample2MPI.run
	-mv ./*.dat build/data/

all_mpi: clear run_mpi plot

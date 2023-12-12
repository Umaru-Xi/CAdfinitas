clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-mv ./adfinitasDump_* ./build/
	-rm -rf ./build

plot:
	gnuplot CAdfinitasExample1.1.plt
	gnuplot CAdfinitasExample1.2.plt
	gnuplot CAdfinitasExample1.3.plt
	-mv ./*.gif build/
	-mv ./*.png build/

build: clear
	gcc -I../../ -I../../../LibXi/ CAdfinitasExample1.c ../../libCAdfinitas.c ../../../LibXi/libXi.c -o CAdfinitasExample1.run -lm
	mkdir -p build/data
	-mv ./*.run build/

run: build
	./build/CAdfinitasExample1.run
	-mv ./*.dat build/data/
	-mv ./adfinitasDump_* ./build/

all: clear run plot

build_mpi: clear
	mpicc -D_ADFINITAS_MPI -I../../ -I../../../LibXi/ -lm CAdfinitasExample1.c ../../libCAdfinitas.c ../../../LibXi/libXi.c -o CAdfinitasExample1MPI.run
	mkdir -p build/data
	-mv ./*.run build/

run_mpi: build_mpi
	mpirun ./build/CAdfinitasExample1MPI.run
	-mv ./*.dat build/data/
	-mv ./adfinitasDump_* ./build/

all_mpi: clear run_mpi plot

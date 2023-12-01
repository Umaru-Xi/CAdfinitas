clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-rm -rf ./build

build: clear
	gcc CAdfinitasExample1.c ../../libCAdfinitas.c ../../libXi.c -o CAdfinitasExample1.run -lm -L../../
	mkdir -p build/data
	-mv ./*.run build/

run: build
	./build/CAdfinitasExample1.run
	-mv ./*.dat build/data/

plot:
	gnuplot CAdfinitasExample1.1.plt
	gnuplot CAdfinitasExample1.2.plt
	gnuplot CAdfinitasExample1.3.plt
	-mv ./*.gif build/
	-mv ./*.png build/

all: clear run plot

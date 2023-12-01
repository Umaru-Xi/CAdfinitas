clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-rm -rf ./build

build: clear
	gcc CAdfinitasExample3.c ../../libCAdfinitas.c ../../libXi.c -o CAdfinitasExample3.run -lm -L../../
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

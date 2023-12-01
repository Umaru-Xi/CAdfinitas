clear:
	-mv ./*.run build/
	-mv ./*.gif build/
	-mv ./*.png build/
	-mv ./*.dat build/data/
	-rm -rf ./build

build: clear
	gcc CAdfinitasExample2.c ../../libCAdfinitas.c ../../libXi.c -o CAdfinitasExample2.run -lm -L../../
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

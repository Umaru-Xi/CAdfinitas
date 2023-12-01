set terminal gif animate delay 1 enhanced font "Times New Roman,20.0" size 1920,1080
set output 'CAdfinitasExample2.1.gif'

set xrange [-1.5e11 : 1.5e11]
set yrange [-1.5e11 : 1.5e11]
set zrange [-3.9e8 : 3.9e8]
set title "C Adfinitas Example 2.1"

set xlabel "x(m)"
set ylabel "y(m)"
set zlabel "z(m)"

do for [i = 1 : 17532 : 200] {
    splot \
    "./build/data/adfinitas-Example2-Moon.dat" using 2 : 3 : 4 every ::::i with lines title "Mercury"\
    , "./build/data/adfinitas-Example2-Moon.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle\
    , "./build/data/adfinitas-Example2-Solar.dat" using 2 : 3 : 4 every ::::i with lines  title "Solar"\
    , "./build/data/adfinitas-Example2-Solar.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle \
    , "./build/data/adfinitas-Example2-Earth.dat" using 2 : 3 : 4 every ::::i with lines  title "Solar"\
    , "./build/data/adfinitas-Example2-Earth.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle
}

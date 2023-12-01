set terminal gif animate delay 1 enhanced font "Times New Roman,20.0" size 1920,1080
set output 'CAdfinitasExample1.1.gif'

set xrange [-2.27939366000e11 : 2.27939366000e11]
set yrange [-2.27939366000e11 : 2.27939366000e11]
set zrange [-1 : 1]
set title "C Adfinitas Example 1.1"
set xlabel "x(m)"
set ylabel "y(m)"
set zlabel "z(m)"

do for [i = 1 : 17532 : 200] {
    splot \
    "./build/data/adfinitas-Example1-Mercury.dat" using 2 : 3 : 4 every ::::i with lines title "Mercury"\
    , "./build/data/adfinitas-Example1-Mercury.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle\
    , "./build/data/adfinitas-Example1-Venus.dat" using 2 : 3 : 4 every ::::i with lines title "Venus"\
    , "./build/data/adfinitas-Example1-Venus.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle\
    , "./build/data/adfinitas-Example1-Earth.dat" using 2 : 3 : 4 every ::::i with lines title "Earth"\
    , "./build/data/adfinitas-Example1-Earth.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle\
    , "./build/data/adfinitas-Example1-Mars.dat" using 2 : 3 : 4 every ::::i with lines title "Mars"\
    , "./build/data/adfinitas-Example1-Mars.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle\
    , "./build/data/adfinitas-Example1-Solar.dat" using 2 : 3 : 4 every ::::i with lines  title "Solar"\
    , "./build/data/adfinitas-Example1-Solar.dat" using 2 : 3 : 4 every ::i::i pt 7 notitle
}

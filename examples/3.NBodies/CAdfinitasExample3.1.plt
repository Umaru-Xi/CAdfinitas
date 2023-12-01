set terminal gif animate delay 1 enhanced font "Times New Roman,20.0" size 1920,1080
set output 'CAdfinitasExample3.1.gif'

set xrange [-5e16 : 15e16]
set yrange [-5e16 : 15e16]
set zrange [-5e16 : 15e16]
set title "C Adfinitas Example 3.1"

set xlabel "x(m)"
set ylabel "y(m)"
set zlabel "z(m)"

list = system('ls -1B ./build/data/adfinitas-Example3-*.dat')

do for [i = 1 : 360 : 1] {
    splot for [file in list] file using 2 : 3 : 4 every ::i::i pt 7 linecolor rgb 'black' notitle
}

set terminal pngcairo enhanced font "Times New Roman,20.0" size 1920,1080
set output 'CAdfinitasExample3.2.png'

set title "C Adfinitas Example 3.2"
set xlabel "time(s)"
set ylabel "hamilton(J)"

plot "./build/data/adfinitas-hamilton-Example3.dat" using 1 : 2 with lines title "System Hamilton"

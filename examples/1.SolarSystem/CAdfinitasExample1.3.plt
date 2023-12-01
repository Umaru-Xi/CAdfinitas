set terminal pngcairo enhanced font "Times New Roman,20.0" size 1920,1080
set output 'CAdfinitasExample1.3.png'

set title "C Adfinitas Example 1.3"

set multiplot layout 1,2 

set xlabel "time(s)"
set ylabel "radial velocity(m/s)"
plot "./build/data/adfinitas-Example1-Mercury.dat" using 1 : 6 with lines title "Mercury Radial Velociry"
plot "./build/data/adfinitas-Example1-Solar.dat" using 1 : 6 with lines title "Solar Radial Velociry"

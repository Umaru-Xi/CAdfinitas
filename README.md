# Adfīnitās

C N-body simulation program with distributed support. It is the new edition of [Juila Adfīnitās package](https://github.com/Umaru-Xi/Adfinitas "Julia Adfīnitās"). Fixed memory consumpting problem and speed up. 

Now distributed computing support is comming back with MPI.

All integrators are sympletic integrator, but Verlet method have some known problem on Hamilton conservation(To be continued).

This simulator does not solve collision. So in the example 3.2, the Hamilton of system is not conservated.

* * *  

Dependency: openMP(not now);

Example dependency: GNU Plot, make, cc;  

* * * 

There are some examples just same as Julia edition.

Examples in examples/, just run: # make all, or using MPI with # make all_mpi 

1. Solar system up to Jupiter;  
    a. Orbits;  
        ![solar system orbits](figures/CAdfinitasExample1.1.gif)  
    b. System Hamilton;  
        ![system hamilton](figures/CAdfinitasExample1.2.png)  
    c. Radial velocity of Mercury and Solar;  
        ![radial velosity](figures/CAdfinitasExample1.3.png)  
2. Sun-Earth-Moon system, but Moon has velocity on z-axis;  
    a. orbits;  
        ![Sun-Earth-Moon system orbits](figures/CAdfinitasExample2.1.gif)  
3. N-Body simulation with 1000 bodies; 
    a. 1000-bodies animation;  
        ![1000-bodies animation](figures/CAdfinitasExample3.1.gif)  
    a. system Hamilton;  
        ![system Hamilton](figures/CAdfinitasExample3.2.png)  
    
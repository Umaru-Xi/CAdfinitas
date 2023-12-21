#include "libXi.h"
#include "libCAdfinitas.h"

#include <stdio.h>
#include <time.h>

#ifdef _XI_MPI
    extern int _xiMPI_PID, _xiMPI_RankSize;
#endif

int main(int argc, char* argv[]){
    xiReturnCode returnCode;
    adfinitasSystem system;
    xiVector3 positionVector, velocityVector;
    char systemName[] = "Example1";
    long double maxHamilton = 0., minHamilton = 0.;

    long double simulationTime = 2 * 365.25 * 24 * 60 * 60; // 2 years
    long double timeStep = 60 * 60; // 1 hour

    clock_t startTime, endTime;
    long double secondsTime = 0;

    startTime = clock();

    #ifdef _XI_MPI
        xiMPIInit();

        startTime = clock();
        returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasMPIIntegratorSemiImplicitEuler);
    #else
        // returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorVerlet);
        returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorSemiImplicitEuler);
    #endif
    if(returnCode != _XI_RETURN_OK) printf("Init System Faild.\n");

    xiInitVector3(&positionVector, 0., 0., 0.);
    xiInitVector3(&velocityVector, 0., 0., 0.);
    returnCode = adfinitasAddBody(&system, "Solar", 1.98847e30, 6.95700e8, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Solar Faild.\n");

    xiInitVector3(&positionVector, 5.791e10, 0., 0.);
    xiInitVector3(&velocityVector, 0., 47360., 0.);
    returnCode = adfinitasAddBody(&system, "Mercury", 3.3011e23, 2.4397e6 , 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Mercury Faild.\n");

    xiInitVector3(&positionVector, 1.0821e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 35020., 0.);
    returnCode = adfinitasAddBody(&system, "Venus", 4.8675e24, 6.0518e6, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Venus Faild.\n");

    xiInitVector3(&positionVector, 1.495978707e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 29780., 0.);
    returnCode = adfinitasAddBody(&system, "Earth", 5.9722e24, 6.3710e6, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Earth Faild.\n");

    xiInitVector3(&positionVector, 2.27939366000e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 24070., 0.);
    returnCode = adfinitasAddBody(&system, "Mars", 6.4171e23, 3.3895e6, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Mars Faild.\n");

    xiInitVector3(&positionVector, 7.78479e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 13070., 0.);
    returnCode = adfinitasAddBody(&system, "Jupiter", 1.8982e27, 6.9911e7, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Jupiter Faild.\n");

    #ifdef _XI_MPI
        returnCode = adfinitasMPIRunSystem(&system);
    #else
        returnCode = adfinitasRunSystem(&system);
    #endif
    if(returnCode != _XI_RETURN_OK) printf("Running Faild.\n");

    returnCode = adfinitasExportDump(&system);
    if(returnCode != _XI_RETURN_OK) printf("Dump System Faild.\n");

    adfinitasClearSystem(&system);

    endTime = clock();
    secondsTime = (float)(endTime - startTime) / CLOCKS_PER_SEC;

    returnCode = adfinitasInitSystemFromDump(&system, "adfinitasDump_Example1", adfinitasIntegratorSemiImplicitEuler);
    if(returnCode != _XI_RETURN_OK) printf("Load Dump File Faild.\n");

    returnCode = adfinitasExportSystem(&system);
    if(returnCode != _XI_RETURN_OK) printf("Exporting Faild.\n");

    returnCode = adfinitasExportSystemHamilton(&system, &minHamilton, &maxHamilton);
    if(returnCode != _XI_RETURN_OK) printf("Exporting Hamilton Faild.\n");

    adfinitasClearSystem(&system);

    #ifdef _XI_MPI
        xiMPIStop();
        if(_xiMPI_PID == 0)
            printf("Total Time: %Les\n", secondsTime);
    #else
        printf("Total Time: %Les\n", secondsTime);
    #endif

    return 0;
}

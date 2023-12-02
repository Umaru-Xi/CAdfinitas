#include "../../libXi.h"
#include "../../libCAdfinitas.h"

#include <stdio.h>
#include <time.h>

#ifdef _ADFINITAS_MPI
    extern int _adfinitasMPI_PID, _adfinitasMPI_RankSize;
#endif

int main(int argc, char* argv[]){
    xiReturnCode returnCode;
    adfinitasSystem system;
    xiVector3 positionVector, velocityVector;
    char systemName[] = "Example2";
    long double maxHamilton = 0., minHamilton = 0.;

    long double simulationTime = 365.25 * 24 * 60 * 60; // 1 year
    long double timeStep = 30 * 60; // 30 mins

    clock_t startTime, endTime;
    long double secondsTime = 0;

    startTime = clock();

    #ifdef _ADFINITAS_MPI
        adfinitasMPIInit();

        startTime = clock();
        returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasMPIIntegratorSemiImplicitEuler);
    #else
        returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorVerlet);
        // returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorSemiImplicitEuler);
    #endif
    if(returnCode != _XI_RETURN_OK) printf("Init System Faild.\n");

    xiInitVector3(&positionVector, 0., 0., 0.);
    xiInitVector3(&velocityVector, 0., 0., 0.);
    returnCode = adfinitasAddBody(&system, "Solar", 1.98847e30, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Solar Faild.\n");

    xiInitVector3(&positionVector, 1.495978707e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 29780., 0.);
    returnCode = adfinitasAddBody(&system, "Earth", 5.9722e24, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Earth Faild.\n");

    xiInitVector3(&positionVector, 1.495978707e11 - 3.84399e8, 0., 0.);
    xiInitVector3(&velocityVector, 0., 29780., 1022.);
    returnCode = adfinitasAddBody(&system, "Moon", 7.342e22, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Moon Faild.\n");

    #ifdef _ADFINITAS_MPI
        returnCode = adfinitasMPIRunSystem(&system);
    #else
        returnCode = adfinitasRunSystem(&system);
    #endif
    if(returnCode != _XI_RETURN_OK) printf("Running Faild.\n");
    returnCode = adfinitasExportSystem(&system);
    if(returnCode != _XI_RETURN_OK) printf("Exporting Faild.\n");

    returnCode = adfinitasExportSystemHamilton(&system, &minHamilton, &maxHamilton);
    if(returnCode != _XI_RETURN_OK) printf("Exporting Hamilton Faild.\n");

    adfinitasClearSystem(&system);

    endTime = clock();
    secondsTime = (float)(endTime - startTime) / CLOCKS_PER_SEC;

    #ifdef _ADFINITAS_MPI
        adfinitasMPIStop();
        if(_adfinitasMPI_PID == 0)
            printf("Total Time: %Les\n", secondsTime);
    #else
        printf("Total Time: %Les\n", secondsTime);
    #endif
    
    return 0;
}
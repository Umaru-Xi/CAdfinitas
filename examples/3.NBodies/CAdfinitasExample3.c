#include "libXi.h"
#include "libCAdfinitas.h"

#include <stdio.h>
#include <time.h>

#ifdef _ADFINITAS_MPI
    extern int _adfinitasMPI_PID, _adfinitasMPI_RankSize;
#endif

int main(int argc, char* argv[]){
    xiReturnCode returnCode;
    adfinitasSystem system;
    xiVector3 positionVector, velocityVector;
    char systemName[] = "Example3";
    long double maxHamilton = 0., minHamilton = 0.;

    long double simulationTime = 5.e6 * 365.25 * 24 * 60 * 60; // 5M years
    long double timeStep = 1.e4 * 365.25 * 24 * 60 * 60; // 10k years

    clock_t startTime, endTime;
    long double secondsTime = 0;

    startTime = clock();

    #ifdef _ADFINITAS_MPI
        adfinitasMPIInit();

        startTime = clock();
        returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasMPIIntegratorSemiImplicitEuler);
    #else
        // returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorVerlet);
        returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorSemiImplicitEuler);
    #endif
    if(returnCode != _XI_RETURN_OK) printf("Init System Faild.\n");

    int maxRow = 4, maxCol = 4, maxHeight = 4;
    int row = 0, col = 0, height = 0;
    char bodyName[256];
    long double lightYear = 9.4607e15;

    for(row = 0; row < maxRow; ++row){
        for(col = 0; col < maxCol; ++col){
            for(height = 0; height < maxHeight; ++height){
                sprintf(bodyName, "Body-%d-%d-%d", row, col, height);
                xiInitVector3(&positionVector, row * lightYear, col * lightYear, height * lightYear);
                xiInitVector3(&velocityVector, 0., 0., 0.);
                returnCode = adfinitasAddBody(&system, bodyName, 1.98847e30, 6.95700e8, 0., positionVector, velocityVector);
                if(returnCode != _XI_RETURN_OK) printf("Add %s Faild.\n", bodyName);
            }
        }
    }

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

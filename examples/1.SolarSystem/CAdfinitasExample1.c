#include "../../libXi.h"
#include "../../libCAdfinitas.h"

#include <stdio.h>

int main(int argc, char* argv[]){
    xiReturnCode returnCode;
    adfinitasSystem system;
    xiVector3 positionVector, velocityVector;
    char systemName[] = "Example1";
    long double maxHamilton = 0., minHamilton = 0.;

    long double simulationTime = 2 * 365.25 * 24 * 60 * 60; // 2 years
    long double timeStep = 60 * 60; // 1 hour

    // returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorVerlet);
    returnCode = adfinitasInitSystem(&system, systemName, 0., simulationTime, timeStep, adfinitasIntegratorSemiImplicitEuler);
    if(returnCode != _XI_RETURN_OK) printf("Init System Faild.\n");

    xiInitVector3(&positionVector, 0., 0., 0.);
    xiInitVector3(&velocityVector, 0., 0., 0.);
    returnCode = adfinitasAddBody(&system, "Solar", 1.98847e30, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Solar Faild.\n");

    xiInitVector3(&positionVector, 5.791e10, 0., 0.);
    xiInitVector3(&velocityVector, 0., 47360., 0.);
    returnCode = adfinitasAddBody(&system, "Mercury", 3.3011e23, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Mercury Faild.\n");

    xiInitVector3(&positionVector, 1.0821e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 35020., 0.);
    returnCode = adfinitasAddBody(&system, "Venus", 4.8675e24, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Venus Faild.\n");

    xiInitVector3(&positionVector, 1.495978707e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 29780., 0.);
    returnCode = adfinitasAddBody(&system, "Earth", 5.9722e24, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Earth Faild.\n");

    xiInitVector3(&positionVector, 2.27939366000e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 24070., 0.);
    returnCode = adfinitasAddBody(&system, "Mars", 6.4171e23, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Mars Faild.\n");

    xiInitVector3(&positionVector, 7.78479e11, 0., 0.);
    xiInitVector3(&velocityVector, 0., 13070., 0.);
    returnCode = adfinitasAddBody(&system, "Jupiter", 1.8982e27, 0., positionVector, velocityVector);
    if(returnCode != _XI_RETURN_OK) printf("Add Jupiter Faild.\n");

    returnCode = adfinitasRunSystem(&system);
    if(returnCode != _XI_RETURN_OK) printf("Running Faild.\n");
    returnCode = adfinitasExportSystem(&system);
    if(returnCode != _XI_RETURN_OK) printf("Exporting Faild.\n");

    returnCode = adfinitasExportSystemHamilton(&system, &minHamilton, &maxHamilton);
    if(returnCode != _XI_RETURN_OK) printf("Exporting Hamilton Faild.\n");
    printf("Hamilton In: [%Le,\t%Le], Diff: %Le\n", minHamilton, maxHamilton, maxHamilton - minHamilton);

    adfinitasClearSystem(&system);
    return 0;
}
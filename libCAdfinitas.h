/*
    @libCAdfinitas.h
    Adfinitas C edition.
    2023-11-28
*/

#ifndef _LIB_C_ADFINITAS
#define _LIB_C_ADFINITAS

// #define _ADFINITAS_MPI  // Use this flag to enable MPI (In CC Flags)

#include "libXi.h"

#define maxNameLength 256
#define maxFileNameLength 1204
#define fileBufferSize 4096

typedef struct adfinitasTrackStruct{
    unsigned long steps;
    unsigned long totalSteps;
    long double* time;
    xiVector3* position;
    xiVector3* velocity;
    xiVector3* acceleration;
    struct adfinitasTrackStruct* before;
    struct adfinitasTrackStruct* next;
} adfinitasTrack;

typedef struct {
    char name[maxNameLength];
    long double staticMass;
    long double staticRadius;
    adfinitasTrack* firstTrack;
    adfinitasTrack* lastTrack;
} adfinitasBody;

typedef struct adfinitasSystemStruct{
    char name[maxNameLength];
    long double gravitationalConstant;
    long double timeStep;
    xiReturnCode (*integrator)(struct adfinitasSystemStruct*);
    unsigned long totalSteps;
    unsigned long bodyNumber;
    adfinitasBody* body;
} adfinitasSystem;

#ifdef _ADFINITAS_MPI
xiReturnCode adfinitasMPIIntegratorSemiImplicitEuler(adfinitasSystem* system);

void adfinitaMPIUpdateAllAcceleration(adfinitasSystem* system);

xiReturnCode adfinitasMPIRunSystem(adfinitasSystem* system);

xiReturnCode adfinitasMPIWait();
void adfinitasMPIStop();
void adfinitasMPIInit();
#endif

void adfinitasAcceleration(xiVector3 motionPosition, xiVector3 sourcePosition, long double sourceMass, long double gravitationalConstant, xiVector3 *acceleration);

xiReturnCode adfinitasExportDump(adfinitasSystem *system);
xiReturnCode adfinitasExportSystemHamilton(adfinitasSystem* system, long double *minHamilton, long double *maxHamilton);
xiReturnCode adfinitasExportBodyHamilton(adfinitasSystem* system, adfinitasBody* body);
xiReturnCode adfinitasExportSystem(adfinitasSystem* system);
xiReturnCode adfinitasExportBody(adfinitasBody* body, adfinitasSystem* system);

xiReturnCode adfinitasBodyEnergy(adfinitasBody* body, adfinitasSystem* system, unsigned long step, long double *time, long double *bodyEnergy);
xiReturnCode adfinitasPotentialEnergy(adfinitasBody* body, adfinitasSystem* system, unsigned long step, long double* potentialTime, long double* potentialEnergy);
xiReturnCode adfinitasKineticEnergy(adfinitasBody* body, unsigned long step, long double *kineticTime, long double *kineticEnergy);

xiReturnCode adfinitasRunSystem(adfinitasSystem* system);

xiReturnCode adfinitasIntegratorVerlet(adfinitasSystem* system);
xiReturnCode adfinitasIntegratorSemiImplicitEuler(adfinitasSystem* system);

void adfinitasUpdateAllAcceleration(adfinitasSystem* system);
void adfinitasGravitationalAcceleration(adfinitasBody* body, adfinitasSystem* system, xiVector3 newPosition, xiVector3* accelerationVector);

xiReturnCode adfinitasBodyLoadTrackRecordInv(adfinitasBody* body, signed long beforeIndex, long double *time, xiVector3 *position, xiVector3 *velocity, xiVector3 *acceleration);
xiReturnCode adfinitasBodyLoadTrackRecord(adfinitasBody* body, unsigned long step, long double *time, xiVector3 *position, xiVector3 *velocity, xiVector3 *acceleration);
xiReturnCode adfinitasBodyInsertTrackRecord(adfinitasBody* body, long double time, xiVector3 position, xiVector3 velocity, xiVector3 acceleration);
xiReturnCode adfinitasBodyAddTrack(adfinitasBody* body, unsigned long totalSteps);
xiReturnCode adfinitasAddBody(adfinitasSystem* system, char* name, long double staticMass, long double staticRadius, long double startTime, xiVector3 startPosition, xiVector3 startVelocity);

void adfinitasClearSystem(adfinitasSystem* system);
xiReturnCode adfinitasInitSystemFromDump(adfinitasSystem *system, const char *directoryName, xiReturnCode (*integrator)(adfinitasSystem*));
xiReturnCode adfinitasInitSystem(adfinitasSystem* system, char* name, long double gravitationalConstant, long double simulationTime, long double timeStep, xiReturnCode (*integrator)(adfinitasSystem*));

void adfinitasClearBody(adfinitasBody* body);
xiReturnCode adfinitasInitBody(adfinitasBody* body, char* name, long double staticMass, long double staticRadius);

void adfinitasClearTracksChain(adfinitasBody* body);
void _adfinitasClearTrack(adfinitasTrack* track);
xiReturnCode adfinitasInitTrack(adfinitasTrack* track, unsigned long totalStep, adfinitasTrack* beforePointer);

#endif
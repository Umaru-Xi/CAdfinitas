#include "libXi.h"
#include "libCAdfinitas.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

const long double _adfinitasGravitationalConstant = 6.6743e-11;
const long double _adfinitasVacuumPermittivity = 8.8541878128e-12;
const long double _adfinitasVacuumPermeability = 1.25663706212e-6;

#ifdef _XI_MPI
#include <mpi.h>

extern int _xiMPI_PID, _xiMPI_RankSize;
extern unsigned int _xiMPI_isWait;

xiReturnCode adfinitasMPIRunSystem(adfinitasSystem* system){
    unsigned long timeIndex = 0;
    xiReturnCode returnCode;

    for(timeIndex = 0; timeIndex < system->totalSteps; ++timeIndex){
        returnCode = (*system->integrator)(system);
        if(returnCode != _XI_RETURN_OK) return returnCode;
    }

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasMPIIntegratorSemiImplicitEuler(adfinitasSystem* system){
    adfinitasBody* body = NULL;
    xiVector3 pastPosition;
    xiVector3 pastVelocity;
    xiVector3 pastAcceleration;
    xiVector3 tmpVector;
    long double lastTime = 0., timeStep = 0., newTime = 0.;
    xiReturnCode returnCode;
    xiVector3 newPosition, newVelocity, newAcceleration;
    unsigned long motionBodyIndex = 0;

    if(_xiMPI_PID == 0){
        timeStep = system->timeStep;
        xiInitVector3(&newAcceleration, 0., 0., 0.);
        for(motionBodyIndex = 0; motionBodyIndex < system->bodyNumber; ++motionBodyIndex){
            body = &system->body[motionBodyIndex];
        
            returnCode = adfinitasBodyLoadTrackRecordInv(body, 0, &lastTime, &pastPosition, &pastVelocity, &pastAcceleration);
            if(returnCode != _XI_RETURN_OK) return returnCode;
            newTime = lastTime + timeStep;

            // v1 = v0 + a0*dt
            xiMulNumberVector3(timeStep, pastAcceleration, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, pastVelocity, &newVelocity);

            // x1 = x0 + v1*dt
            xiMulNumberVector3(timeStep, newVelocity, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, pastPosition, &newPosition);

            returnCode = adfinitasBodyInsertTrackRecord(body, newTime, newPosition, newVelocity, newAcceleration);
            if(returnCode != _XI_RETURN_OK) return returnCode;
        }

        for(motionBodyIndex = 0; motionBodyIndex < system->bodyNumber; ++motionBodyIndex){
            system->body[motionBodyIndex].lastTrack->steps = system->body[motionBodyIndex].lastTrack->steps + 1;
        }
    }
    xiMPIWait();

    adfinitaMPIUpdateAllAcceleration(system);

    return _XI_RETURN_OK;
}

void adfinitaMPIUpdateAllAcceleration(adfinitasSystem* system){
    adfinitasBody *sourceBody = NULL, *motionBody = NULL;
    adfinitasTrack *motionTrack = NULL, *sourceTrack = NULL;
    unsigned long sourceBodyIndex = 0, motionBodyIndex = 0, updateStep = 0;
    xiVector3 newAcceleration, subAcceleration;
    xiVector3 sourcePosition, motionPosition;

    long double MPISendPackat[8];
    long double MPIRecvPackat[8];
    MPI_Status MPIStatus;
    int PID = 1, recvPID = 0;

    PID = 1;
    MPIStatus.MPI_TAG = _XI_CONTROL_WAIT;
    if(_xiMPI_PID == 0){
        MPISendPackat[7] = system->gravitationalConstant;
        for(motionBodyIndex = 0; motionBodyIndex < system->bodyNumber; ++motionBodyIndex){

            motionBody = &system->body[motionBodyIndex];
            motionTrack = motionBody->lastTrack;
            updateStep = motionTrack->steps - 1;
            xiInitVector3(&newAcceleration, 0., 0., 0.);

            MPISendPackat[0] = motionTrack->position[updateStep].x;
            MPISendPackat[1] = motionTrack->position[updateStep].y;
            MPISendPackat[2] = motionTrack->position[updateStep].z;

            for(sourceBodyIndex = 0; sourceBodyIndex < system->bodyNumber; ++sourceBodyIndex){
                if(sourceBodyIndex == motionBodyIndex) continue;
                sourceBody = &system->body[sourceBodyIndex];
                sourceTrack = sourceBody->lastTrack;

                MPISendPackat[3] = sourceTrack->position[updateStep].x;
                MPISendPackat[4] = sourceTrack->position[updateStep].y;
                MPISendPackat[5] = sourceTrack->position[updateStep].z;
                MPISendPackat[6] = sourceBody->staticMass;

                MPI_Send(MPISendPackat, 8, MPI_LONG_DOUBLE, PID, _XI_CONTROL_COMPUTE, MPI_COMM_WORLD);
                ++PID;
                if(PID >= _xiMPI_RankSize){
                    for(recvPID = 1; recvPID < _xiMPI_RankSize; ++recvPID){

                        MPI_Recv(MPIRecvPackat, 3, MPI_LONG_DOUBLE, recvPID, MPI_ANY_TAG, MPI_COMM_WORLD, &MPIStatus);

                        xiInitVector3(&subAcceleration, MPIRecvPackat[0], MPIRecvPackat[1], MPIRecvPackat[2]);
                        xiFunc2Vector3(xiAdd, subAcceleration, newAcceleration, &newAcceleration);
                    }
                    PID = 1;
                }
            }
            if(PID >= 1){
                for(recvPID = 1; recvPID < PID; ++recvPID){
                    MPI_Recv(MPIRecvPackat, 3, MPI_LONG_DOUBLE, recvPID, MPI_ANY_TAG, MPI_COMM_WORLD, &MPIStatus);

                    xiInitVector3(&subAcceleration, MPIRecvPackat[0], MPIRecvPackat[1], MPIRecvPackat[2]);
                    xiFunc2Vector3(xiAdd, subAcceleration, newAcceleration, &newAcceleration);
                }
                PID = 1;
            }
            xiInitVector3(&motionTrack->acceleration[updateStep], newAcceleration.x, newAcceleration.y, newAcceleration.z);
        }
        for(recvPID = 1; recvPID < _xiMPI_RankSize; ++recvPID){
            MPI_Send(MPISendPackat, 8, MPI_LONG_DOUBLE, recvPID, _XI_CONTROL_STOP, MPI_COMM_WORLD);
        }
    }else{
        while(MPIStatus.MPI_TAG != _XI_CONTROL_STOP){
            MPI_Recv(MPIRecvPackat, 8, MPI_LONG_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &MPIStatus);

            if(MPIStatus.MPI_TAG == _XI_CONTROL_COMPUTE){
                xiInitVector3(&motionPosition, MPIRecvPackat[0], MPIRecvPackat[1], MPIRecvPackat[2]);
                xiInitVector3(&sourcePosition, MPIRecvPackat[3], MPIRecvPackat[4], MPIRecvPackat[5]);

                adfinitasAcceleration(motionPosition, sourcePosition, MPIRecvPackat[6], MPIRecvPackat[7], &subAcceleration);

                MPISendPackat[0] = subAcceleration.x;
                MPISendPackat[1] = subAcceleration.y;
                MPISendPackat[2] = subAcceleration.z;

                MPI_Send(MPISendPackat,3, MPI_LONG_DOUBLE, 0, _XI_CONTROL_COMPUTE, MPI_COMM_WORLD);
            }
        }
    }
    xiMPIWait();
}

#endif

void adfinitasAcceleration(xiVector3 motionPosition, xiVector3 sourcePosition, long double sourceMass, long double gravitationalConstant, xiVector3 *acceleration){
    xiVector3 distanceVector;
    long double distanceMod = 0., accelerationMod = 0.;

    xiFunc2Vector3(xiSub, sourcePosition, motionPosition, &distanceVector);
    distanceMod = xiModVector3(distanceVector);
    accelerationMod = gravitationalConstant * sourceMass / powl(distanceMod, 3.);
    xiMulNumberVector3(accelerationMod, distanceVector, acceleration);

    return;
}

xiReturnCode adfinitasExportDump(adfinitasSystem *system){
    char dirName[maxFileNameLength] = "\0";
    char fileName[maxFileNameLength] = "\0";
    FILE *filePointer = NULL, *subFilePointer = NULL;
    xiReturnCode returnCode;
    unsigned long bodyIndex = 0, step = 0;
    long double time = 0.;
    xiVector3 position, velocity, acceleration;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0) return xiMPIWait();
    #endif

    strcat(dirName, "adfinitasDump_");
    strcat(dirName, system->name);
    mkdir(dirName, S_IRWXU);
    // if(mkdir(dirName, S_IRWXU) == -1) return _XI_RETURN_DIRECTORY_ERROR;

    strcpy(fileName, dirName);
    strcat(fileName, "/system");
    filePointer = fopen(fileName, "w+");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;
    
    fprintf(filePointer, "%s\t%Le\t%Le\t%lu\t%lu\n", system->name, system->gravitationalConstant, system->timeStep, system->totalSteps, system->bodyNumber);
    fclose(filePointer);

    strcpy(fileName, dirName);
    strcat(fileName, "/bodies");
    filePointer = fopen(fileName, "w+");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;

    for(bodyIndex = 0; bodyIndex < system->bodyNumber; ++bodyIndex){
        fprintf(filePointer, "%s\t%Le\t%Le\n", system->body[bodyIndex].name, system->body[bodyIndex].staticMass, system->body[bodyIndex].staticRadius);

        sprintf(fileName, "%s/track_%lu", dirName, bodyIndex);
        subFilePointer = fopen(fileName, "w+");
        if(subFilePointer == NULL){ 
            fclose(filePointer);
            return _XI_RETURN_FILE_ERROR;
        }
        for(step = 1; step < system->totalSteps; ++step){
            returnCode = adfinitasBodyLoadTrackRecord(&system->body[bodyIndex], step, &time, &position, &velocity, &acceleration);
            if(returnCode != _XI_RETURN_OK) break;
            fprintf(subFilePointer, "%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\n", time, position.x, position.y, position.z, velocity.x, velocity.y, velocity.z, acceleration.x, acceleration.y, acceleration.z);
        }
        fclose(subFilePointer);
    }
    fclose(filePointer);

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0) xiMPIWait();
    #endif
    
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasExportSystemHamilton(adfinitasSystem* system, long double *minHamilton, long double *maxHamilton){
    FILE *filePointer = NULL;
    unsigned long step = 0., bodyIndex = 0;
    long double kineticEnergy = 0., potentialEnergy = 0., time = 0., checkTime = 0., hamilton = 0., systemTime = 0., systemKineticEnergy = 0., systemPotentialEnergy = 0., systemHamilton = 0.;
    xiReturnCode returnCode;
    char fileName[maxFileNameLength] = "\0";
    adfinitasBody* body = NULL;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0) return xiMPIWait();
    #endif

    *minHamilton = INFINITY;
    *maxHamilton = -INFINITY;

    strcat(fileName, "adfinitas-hamilton-");
    strcat(fileName, system->name);
    strcat(fileName, ".dat");
    filePointer = fopen(fileName, "w+");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;

    for(step = 1; step < system->totalSteps; ++step){
        systemTime = 0.;
        systemKineticEnergy = 0.;
        systemPotentialEnergy = 0.;
        for(bodyIndex = 0.; bodyIndex < system->bodyNumber; ++bodyIndex){
            body = &system->body[bodyIndex];
            returnCode = adfinitasKineticEnergy(body, step, &checkTime, &kineticEnergy);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            returnCode = adfinitasPotentialEnergy(body, system, step, &time, &potentialEnergy);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            if(time != checkTime) return _XI_RETURN_MISMATCH_ITEM;
            hamilton = kineticEnergy + potentialEnergy;
            if(bodyIndex == 0) systemTime = time;
            else if(time != systemTime) return _XI_RETURN_MISMATCH_ITEM;

            systemKineticEnergy += kineticEnergy;
            systemPotentialEnergy += potentialEnergy;
        }
        systemHamilton = systemKineticEnergy + systemPotentialEnergy;

        if(systemHamilton == INFINITY || systemHamilton == -INFINITY) continue;

        *maxHamilton = (systemHamilton > *maxHamilton)? systemHamilton : *maxHamilton;
        *minHamilton = (systemHamilton < *minHamilton)? systemHamilton : *minHamilton;

        fprintf(filePointer, "%Le\t%Le\t%Le\t%Le\n", systemTime,  systemHamilton, systemKineticEnergy, systemPotentialEnergy);
    }
    fclose(filePointer);

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0) xiMPIWait();
    #endif

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasExportBodyHamilton(adfinitasSystem* system, adfinitasBody* body){
    FILE *filePointer = NULL;
    unsigned long step = 0.;
    long double kineticEnergy = 0., potentialEnergy = 0., time = 0., checkTime = 0., hamilton = 0.;
    xiReturnCode returnCode;
    char fileName[maxFileNameLength] = "\0";

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0) return xiMPIWait();
    #endif
    
    strcat(fileName, "adfinitas-hamilton-");
    strcat(fileName, system->name);
    strcat(fileName, "-");
    strcat(fileName, body->name);
    strcat(fileName, ".dat");
    filePointer = fopen(fileName, "w+");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;

    for(step = 0; step < system->totalSteps; ++step){
        returnCode = adfinitasKineticEnergy(body, step, &checkTime, &kineticEnergy);
        if(returnCode != _XI_RETURN_OK) return returnCode;

        returnCode = adfinitasPotentialEnergy(body, system, step, &time, &potentialEnergy);
        if(returnCode != _XI_RETURN_OK) return returnCode;

        if(time != checkTime) return _XI_RETURN_MISMATCH_ITEM;
        hamilton = kineticEnergy + potentialEnergy;

        fprintf(filePointer, "%Le\t%Le\t%Le\t%Le\n", time,  hamilton, kineticEnergy, potentialEnergy);
    }
    fclose(filePointer);

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0) xiMPIWait();
    #endif

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasExportSystem(adfinitasSystem* system){
    unsigned long bodyIndex = 0;
    xiReturnCode returnCode;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0) return xiMPIWait();
    #endif

    for(bodyIndex = 0; bodyIndex < system->bodyNumber; ++bodyIndex){
        returnCode = adfinitasExportBody(&system->body[bodyIndex], system);
        if(returnCode != _XI_RETURN_OK) return returnCode;
    }

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0) xiMPIWait();
    #endif

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasExportBody(adfinitasBody* body, adfinitasSystem* system){
    FILE *filePointer = NULL;
    adfinitasTrack* track = NULL;
    char fileName[maxFileNameLength] = "\0";
    unsigned long index = 0;

    track = body->firstTrack;
    strcat(fileName, "adfinitas-");
    strcat(fileName, system->name);
    strcat(fileName, "-");
    strcat(fileName, body->name);
    strcat(fileName, ".dat");
    filePointer = fopen(fileName, "w+");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;
    while(track->next != NULL || index < track->steps){
        if(index >= track->steps){
            if(track->next != NULL) track = track->next;
            index = 0;
        }
        fprintf(filePointer, "%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\n", track->time[index], track->position[index].x, track->position[index].y, track->position[index].z, track->velocity[index].x, track->velocity[index].y, track->velocity[index].z, track->acceleration[index].x, track->acceleration[index].y, track->acceleration[index].z);
        ++index;
    }
    fclose(filePointer);

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasBodyEnergy(adfinitasBody* body, adfinitasSystem* system, unsigned long step, long double *time, long double *bodyEnergy){
    xiReturnCode returnCode;
    long double potentialEnergy = 0., potentialTime = 0., kineticEnergy = 0., kineticTime = 0.;
    
    returnCode = adfinitasPotentialEnergy(body, system, step, &potentialTime, &potentialEnergy);
    if(returnCode != _XI_RETURN_OK) return returnCode;

    returnCode = adfinitasKineticEnergy(body, step, &kineticTime, &kineticEnergy);
    if(returnCode != _XI_RETURN_OK) return returnCode;

    if(kineticEnergy != potentialTime) return _XI_RETURN_MISMATCH_ITEM;

    *bodyEnergy = kineticEnergy + potentialEnergy;

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasPotentialEnergy(adfinitasBody* body, adfinitasSystem* system, unsigned long step, long double* potentialTime, long double* potentialEnergy){
    adfinitasBody *sourceBody = NULL;
    xiVector3 motionPosition, motionVelocity, motionAcceleration, sourcePosition, sourceVelocity, sourceAcceleration, tmpVector;
    xiReturnCode returnCode;
    long double motionTime = 0., sourceTime = 0.;
    unsigned long bodyIndex = 0;
    long double distance = 0., subPotentialEnergy = 0.;

    returnCode = adfinitasBodyLoadTrackRecord(body, step, &motionTime, &motionPosition, &motionVelocity, &motionAcceleration);
    if(returnCode != _XI_RETURN_OK) return returnCode;
    *potentialTime = motionTime;

    *potentialEnergy = 0.;
    for(bodyIndex = 0; bodyIndex < system->bodyNumber; ++bodyIndex){
        sourceBody = &system->body[bodyIndex];
        if(!strcmp(body->name, sourceBody->name)) continue;
        returnCode = adfinitasBodyLoadTrackRecord(sourceBody, step, &sourceTime, &sourcePosition, &sourceVelocity, &sourceAcceleration);
        if(returnCode != _XI_RETURN_OK) return returnCode;

        if(sourceTime != motionTime) return _XI_RETURN_MISMATCH_ITEM;

        xiFunc2Vector3(xiSub, sourcePosition, motionPosition, &tmpVector);
        distance = xiModVector3(tmpVector);
        subPotentialEnergy = body->staticMass * sourceBody->staticMass;
        subPotentialEnergy /= -distance;
        subPotentialEnergy *= system->gravitationalConstant;
        *potentialEnergy += subPotentialEnergy;
    }
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasKineticEnergy(adfinitasBody* body, unsigned long step, long double *kineticTime, long double *kineticEnergy){
    xiVector3 position, velocity, acceleration;
    xiReturnCode returnCode;
    long double time = 0.;
    returnCode = adfinitasBodyLoadTrackRecord(body, step, &time, &position, &velocity, &acceleration);
    if(returnCode != _XI_RETURN_OK) return returnCode;
    *kineticTime = time;

    *kineticEnergy = xiModVector3(velocity);
    *kineticEnergy = (*kineticEnergy) * (*kineticEnergy);
    *kineticEnergy *= (body->staticMass / 2);
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasRunSystem(adfinitasSystem* system){
    unsigned long timeIndex = 0;
    xiReturnCode returnCode;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0) return xiMPIWait();
    #endif

    for(timeIndex = 0; timeIndex < system->totalSteps; ++timeIndex){
        returnCode = (*system->integrator)(system);
        if(returnCode != _XI_RETURN_OK) return returnCode;
    }

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0) return xiMPIWait();
    #endif

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasIntegratorSemiImplicitEuler(adfinitasSystem* system){
    adfinitasBody* body = NULL;
    xiVector3 pastPosition;
    xiVector3 pastVelocity;
    xiVector3 pastAcceleration;
    xiVector3 tmpVector;
    long double lastTime = 0., timeStep = 0., newTime = 0.;
    xiReturnCode returnCode;
    xiVector3 newPosition, newVelocity, newAcceleration;
    unsigned long motionBodyIndex = 0;

    timeStep = system->timeStep;
    xiInitVector3(&newAcceleration, 0., 0., 0.);
    for(motionBodyIndex = 0; motionBodyIndex < system->bodyNumber; ++motionBodyIndex){
        body = &system->body[motionBodyIndex];
    
        returnCode = adfinitasBodyLoadTrackRecordInv(body, 0, &lastTime, &pastPosition, &pastVelocity, &pastAcceleration);
        if(returnCode != _XI_RETURN_OK) return returnCode;
        newTime = lastTime + timeStep;

        // v1 = v0 + a0*dt
        xiMulNumberVector3(timeStep, pastAcceleration, &tmpVector);
        xiFunc2Vector3(xiAdd, tmpVector, pastVelocity, &newVelocity);

        // x1 = x0 + v1*dt
        xiMulNumberVector3(timeStep, newVelocity, &tmpVector);
        xiFunc2Vector3(xiAdd, tmpVector, pastPosition, &newPosition);

        returnCode = adfinitasBodyInsertTrackRecord(body, newTime, newPosition, newVelocity, newAcceleration);
        if(returnCode != _XI_RETURN_OK) return returnCode;
    }

    for(motionBodyIndex = 0; motionBodyIndex < system->bodyNumber; ++motionBodyIndex){
        system->body[motionBodyIndex].lastTrack->steps = system->body[motionBodyIndex].lastTrack->steps + 1;
    }
    adfinitasUpdateAllAcceleration(system);

    return _XI_RETURN_OK;
}

xiReturnCode adfinitasIntegratorVerlet(adfinitasSystem* system){
    unsigned long motionIndex = 0;
    adfinitasBody *motionBody = NULL, *sourceBody = NULL;
    adfinitasTrack *motionTrack = NULL, *sourceTrack = NULL;
    xiReturnCode returnCode;
    xiVector3 pastPositions[3], pastVelocities[3], pastAccelerations[3], newPosition, newVelocity, newAcceleration, tmpVector;
    long double timeStep = 0, newTime = 0, pastTimes[3];

    timeStep = system->timeStep;

    // Update Position
    for(motionIndex = 0; motionIndex < system->bodyNumber; ++motionIndex){
        motionBody = &system->body[motionIndex];
        motionTrack = motionBody->lastTrack;

        // First Stage
        if(motionTrack->before == NULL && motionTrack->steps <= 1){
            // x1 = x0
            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, 0, &pastTimes[1], &newPosition, &newVelocity, &newAcceleration);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            // TEST: x1 = x0 + v0*dt + a0*dt2/2
            xiMulNumberVector3(timeStep, newVelocity, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, newPosition, &newPosition);
            xiMulNumberVector3(timeStep * timeStep / 2, newAcceleration, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, newPosition, &newPosition);

        }else{  // Second Stage
            // x2 = x1 + v1*dt + a1*dt2/2
            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, 0, &pastTimes[1], &newPosition, &newVelocity, &newAcceleration);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            xiMulNumberVector3(timeStep, newVelocity, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, newPosition, &newPosition);

            xiMulNumberVector3(timeStep * timeStep / 2, newAcceleration, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, newPosition, &newPosition);
        }
        newTime = pastTimes[1];
        returnCode = adfinitasBodyInsertTrackRecord(motionBody, newTime, newPosition, newVelocity, newAcceleration);
        if(returnCode != _XI_RETURN_OK) return returnCode;
    }

    // Update Acceleartion
    for(motionIndex = 0; motionIndex < system->bodyNumber; ++motionIndex){
        system->body[motionIndex].lastTrack->steps = system->body[motionIndex].lastTrack->steps + 1;
    }
    adfinitasUpdateAllAcceleration(system);
    for(motionIndex = 0; motionIndex < system->bodyNumber; ++motionIndex){
        system->body[motionIndex].lastTrack->steps = system->body[motionIndex].lastTrack->steps - 1;
    }

    // Update Velocity
    for(motionIndex = 0; motionIndex < system->bodyNumber; ++motionIndex){
        motionBody = &system->body[motionIndex];
        motionTrack = motionBody->lastTrack;

        // First Stage
        if(motionTrack->before == NULL && motionTrack->steps <= 1){
            // v1 = v0 + a1*dt/2
            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, 0, &pastTimes[1], &pastPositions[1], &newVelocity, &pastAccelerations[1]);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, -1, &pastTimes[2], &newPosition, &pastVelocities[2], &newAcceleration);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            // xiMulNumberVector3(timeStep / 2, newAcceleration, &tmpVector);
            // TEST: v1 = v0 + a1*dt
            xiMulNumberVector3(timeStep, newAcceleration, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, newVelocity, &newVelocity);
        }else{  // Second Stage
            // v2 = v1 + (a2+a1)dt/2
            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, 0, &pastTimes[1], &pastPositions[1], &newVelocity, &pastAccelerations[1]);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, 1, &pastTimes[0], &pastPositions[0], &pastVelocities[0], &pastAccelerations[0]);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            returnCode = adfinitasBodyLoadTrackRecordInv(motionBody, -1, &pastTimes[2], &newPosition, &pastVelocities[2], &newAcceleration);
            if(returnCode != _XI_RETURN_OK) return returnCode;

            xiFunc2Vector3(xiAdd, pastAccelerations[1], newAcceleration, &tmpVector);
            xiMulNumberVector3(timeStep / 2, tmpVector, &tmpVector);
            xiFunc2Vector3(xiAdd, tmpVector, newVelocity, &newVelocity);
        }
        newTime = pastTimes[1] + timeStep;
        returnCode = adfinitasBodyInsertTrackRecord(motionBody, newTime, newPosition, newVelocity, newAcceleration);
        if(returnCode != _XI_RETURN_OK) return returnCode;
    }

    for(motionIndex = 0; motionIndex < system->bodyNumber; ++motionIndex){
        system->body[motionIndex].lastTrack->steps = system->body[motionIndex].lastTrack->steps + 1;
    }
    
    return _XI_RETURN_OK;
}

void adfinitasUpdateAllAcceleration(adfinitasSystem* system){
    adfinitasBody *sourceBody = NULL, *motionBody = NULL;
    adfinitasTrack *motionTrack = NULL, *sourceTrack = NULL;
    unsigned long sourceBodyIndex = 0, motionBodyIndex = 0, updateStep = 0;
    xiVector3 newAcceleration, subAcceleration, distanceVector;
    long double distanceMod = 0., accelerationMod = 0.;

    for(motionBodyIndex = 0; motionBodyIndex < system->bodyNumber; ++motionBodyIndex){

        motionBody = &system->body[motionBodyIndex];
        motionTrack = motionBody->lastTrack;
        updateStep = motionTrack->steps - 1;
        xiInitVector3(&newAcceleration, 0., 0., 0.);

        for(sourceBodyIndex = 0; sourceBodyIndex < system->bodyNumber; ++sourceBodyIndex){
            if(sourceBodyIndex == motionBodyIndex) continue;
            sourceBody = &system->body[sourceBodyIndex];
            sourceTrack = sourceBody->lastTrack;

            xiInitVector3(&subAcceleration, 0., 0., 0.);

            xiFunc2Vector3(xiSub, sourceTrack->position[updateStep], motionTrack->position[updateStep], &distanceVector);
            distanceMod = xiModVector3(distanceVector);
            accelerationMod = system->gravitationalConstant * sourceBody->staticMass / powl(distanceMod, 3.);
            xiMulNumberVector3(accelerationMod, distanceVector, &subAcceleration);
            xiFunc2Vector3(xiAdd, subAcceleration, newAcceleration, &newAcceleration);
        }
        xiInitVector3(&motionTrack->acceleration[updateStep], newAcceleration.x, newAcceleration.y, newAcceleration.z);
    }
}

void adfinitasGravitationalAcceleration(adfinitasBody* body, adfinitasSystem* system, xiVector3 newPosition, xiVector3* accelerationVector){
    unsigned long motionIndex = 0., index = 0.;

    xiVector3 distanceVector, subAcceleration;
    adfinitasBody *sourceBody = NULL;
    adfinitasTrack *sourceTrack = NULL;
    long double distanceMod = 0., accelerationMod = 0.;

    xiInitVector3(accelerationVector, 0., 0., 0.);

    for(motionIndex = 0; motionIndex < system->bodyNumber; ++motionIndex){
        if(!strcmp(body->name, system->body[motionIndex].name)) break;
    }

    for(index = 0; index < system->bodyNumber; ++index){
        if(index == motionIndex) continue;
        sourceBody = &system->body[index];
        sourceTrack = sourceBody->lastTrack;

        xiInitVector3(&subAcceleration, 0., 0., 0.);

        xiFunc2Vector3(xiSub, sourceTrack->position[sourceTrack->steps - 1], newPosition, &distanceVector);
        distanceMod = xiModVector3(distanceVector);
        accelerationMod = system->gravitationalConstant * sourceBody->staticMass / powl(distanceMod, 3.);
        xiMulNumberVector3(accelerationMod, distanceVector, &subAcceleration);
        xiFunc2Vector3(xiAdd, subAcceleration, *accelerationVector, accelerationVector);
    }
    return;
}

xiReturnCode adfinitasBodyLoadTrackRecordInv(adfinitasBody* body, signed long beforeIndex, long double *time, xiVector3 *position, xiVector3 *velocity, xiVector3 *acceleration){
    adfinitasTrack *track = NULL;
    signed long step = 0.;
    xiVector3 *tmpVector = NULL;

    track =  body->lastTrack;
    step = (signed long)track->steps - beforeIndex - 1;

    while(step >= (signed long)track->totalSteps){
        if(track->next == NULL) return _XI_RETURN_NO_ENOUGH_ITEM;
        step -= (signed long)track->steps;
        track = track->next;
    }
    while(step < 0){
        if(track->before == NULL) return _XI_RETURN_NO_ENOUGH_ITEM;
        track = track->before;
        step += (signed long)track->steps;
    }

    *time = track->time[step];

    tmpVector = &track->position[step];
    xiInitVector3(position, tmpVector->x, tmpVector->y, tmpVector->z);

    tmpVector = &track->velocity[step];
    xiInitVector3(velocity, tmpVector->x, tmpVector->y, tmpVector->z);
    
    tmpVector = &track->acceleration[step];
    xiInitVector3(acceleration, tmpVector->x, tmpVector->y, tmpVector->z);
    
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasBodyLoadTrackRecord(adfinitasBody* body, unsigned long step, long double *time, xiVector3 *position, xiVector3 *velocity, xiVector3 *acceleration){
    adfinitasTrack* track = NULL;
    unsigned long nowStep = step;
    
    track = body->firstTrack;
    while(nowStep > track->steps){
        nowStep -= track->steps;
        track = track->next;
        if(track == NULL || nowStep < 1) return _XI_RETURN_NO_ENOUGH_ITEM;
    }
    --nowStep;
    *time = track->time[nowStep];
    xiInitVector3(position, track->position[nowStep].x, track->position[nowStep].y, track->position[nowStep].z);
    xiInitVector3(velocity, track->velocity[nowStep].x, track->velocity[nowStep].y, track->velocity[nowStep].z);
    xiInitVector3(acceleration, track->acceleration[nowStep].x, track->acceleration[nowStep].y, track->acceleration[nowStep].z);
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasBodyInsertTrackRecord(adfinitasBody* body, long double time, xiVector3 position, xiVector3 velocity, xiVector3 acceleration){
    adfinitasTrack* track;
    unsigned long lastIndex = 0, totalSteps = 0;
    xiReturnCode returnCode;

    track = body->lastTrack;
    lastIndex = track->steps;
    totalSteps = track->totalSteps;
    if(lastIndex >= totalSteps){
        returnCode = adfinitasBodyAddTrack(body, totalSteps);
        if(returnCode != _XI_RETURN_OK) return returnCode;
        track = body->lastTrack;
        track->steps = 0;
        lastIndex = 0;
        totalSteps = track->totalSteps;
    }
    track->time[lastIndex] = time;
    xiInitVector3(&track->position[lastIndex], position.x, position.y, position.z);
    xiInitVector3(&track->velocity[lastIndex], velocity.x, velocity.y, velocity.z);
    xiInitVector3(&track->acceleration[lastIndex], acceleration.x, acceleration.y, acceleration.z);
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasBodyAddTrack(adfinitasBody* body, unsigned long totalSteps){
    adfinitasTrack* lastTrack = NULL;
    xiReturnCode returnCode = _XI_RETURN_OK;
    lastTrack = body->lastTrack;
    body->lastTrack = (adfinitasTrack*)malloc(sizeof(adfinitasTrack));
    if(body->lastTrack == NULL){
        body->lastTrack = lastTrack;
        return _XI_RETURN_ALLOCATION_ERROR;
    }
    returnCode = adfinitasInitTrack(body->lastTrack, totalSteps, lastTrack);
    return returnCode;
}

xiReturnCode adfinitasAddBody(adfinitasSystem* system, char* name, long double staticMass, long double staticRadius, long double startTime, xiVector3 startPosition, xiVector3 startVelocity){
    adfinitasBody* tmpBodyPointer = NULL;
    xiReturnCode returnCode = _XI_RETURN_OK;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0) return xiMPIWait();
    #endif

    tmpBodyPointer = (adfinitasBody*)realloc(system->body, (system->bodyNumber + 1) * sizeof(adfinitasBody));
    if(tmpBodyPointer == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    system->body = tmpBodyPointer;

    returnCode = adfinitasInitBody(&system->body[system->bodyNumber], name, staticMass, staticRadius);
    if(returnCode != _XI_RETURN_OK) return returnCode;

    returnCode = adfinitasInitTrack(system->body[system->bodyNumber].firstTrack, system->totalSteps, NULL);
    if(returnCode != _XI_RETURN_OK){
        _adfinitasClearTrack(system->body[system->bodyNumber].firstTrack);
        return returnCode;
    }

    system->body[system->bodyNumber].firstTrack->time[0] = startTime;
    xiInitVector3(&system->body[system->bodyNumber].firstTrack->position[0], startPosition.x, startPosition.y, startPosition.z);
    xiInitVector3(&system->body[system->bodyNumber].firstTrack->velocity[0], startVelocity.x, startVelocity.y, startVelocity.z);
    xiInitVector3(&system->body[system->bodyNumber].firstTrack->acceleration[0], 0., 0., 0.);
    system->body[system->bodyNumber].firstTrack->steps = 1;

    system->body[system->bodyNumber].lastTrack = system->body[system->bodyNumber].firstTrack;

    ++system->bodyNumber;

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0) xiMPIWait();
    #endif

    return _XI_RETURN_OK;
}

void adfinitasClearSystem(adfinitasSystem* system){
    unsigned long index = 0;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0){
            xiMPIWait();
            return;
        }
    #endif

    for(index = 0; index < system->bodyNumber; ++index)
        adfinitasClearBody(&system->body[index]);
    free(system->body);

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0){
            xiMPIWait();
        }
    #endif

    return;
}

xiReturnCode adfinitasInitSystemFromDump(adfinitasSystem *system, const char *directoryName, xiReturnCode (*integrator)(adfinitasSystem*)){
    char fileName[maxFileNameLength] = "\0";
    char name[maxNameLength] = "\0";
    char *fileBuffer = NULL;
    FILE *filePointer = NULL, *subFilePointer = NULL;
    xiReturnCode returnCode;
    long double timeStep = 0., gravitationalConstant = 0., staticMass = 0., staticRadius = 0., time = 0.;
    unsigned long totalSteps = 0, bodyNumber = 0, bodyIndex = 0, step = 0;
    xiVector3 zeroVector, position, velocity, acceleration;

    xiInitVector3(&zeroVector, 0., 0., 0.);

    strcpy(fileName, directoryName);
    strcat(fileName, "/system");
    filePointer = fopen(fileName, "r");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;

    fscanf(filePointer, "%s\t%Le\t%Le\t%lu\t%lu", name, &gravitationalConstant, &timeStep, &totalSteps, &bodyNumber);
    fclose(filePointer);
    returnCode = adfinitasInitSystem(system, name, gravitationalConstant, totalSteps * timeStep, timeStep, integrator);
    if(returnCode != _XI_RETURN_OK) return returnCode;

    #ifdef _XI_MPI
        ++_xiMPI_isWait;
        if(_xiMPI_PID != 0){
            xiMPIWait();
            return _XI_RETURN_OK;
        }
    #endif

    strcpy(fileName, directoryName);
    strcat(fileName, "/bodies");
    filePointer = fopen(fileName, "r");
    if(filePointer == NULL) return _XI_RETURN_FILE_ERROR;

    for(bodyIndex = 0; bodyIndex < bodyNumber; ++bodyIndex){
        fscanf(filePointer, "%s\t%Le\t%Le", name, &staticMass, &staticRadius);

        returnCode = adfinitasAddBody(system, name, staticMass, staticRadius, 0., zeroVector, zeroVector);
        if(returnCode != _XI_RETURN_OK){
            fclose(filePointer);
            return returnCode;
        }
        system->body[bodyIndex].lastTrack->steps = 0;

        sprintf(fileName, "%s/track_%lu", directoryName, bodyIndex);
        subFilePointer = fopen(fileName, "r");
        if(subFilePointer == NULL){ 
            fclose(filePointer);
            return _XI_RETURN_FILE_ERROR;
        }

        size_t length = 0;
        while(getline(&fileBuffer, &length, subFilePointer) != -1){
            sscanf(fileBuffer, "%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le\t%Le", &time, &position.x, &position.y, &position.z, &velocity.x, &velocity.y, &velocity.z, &acceleration.x, &acceleration.y, &acceleration.z);
            returnCode = adfinitasBodyInsertTrackRecord(&system->body[bodyIndex], time, position, velocity, acceleration);
            ++system->body[bodyIndex].lastTrack->steps;
            if(returnCode != _XI_RETURN_OK) break;
        }
        fclose(subFilePointer);
    }
    fclose(filePointer);

    #ifdef _XI_MPI
        --_xiMPI_isWait;
        if(_xiMPI_PID == 0){
            xiMPIWait();
        }
    #endif
    
    return _XI_RETURN_OK;
}

xiReturnCode adfinitasInitSystem(adfinitasSystem* system, char* name, long double gravitationalConstant, long double simulationTime, long double timeStep, xiReturnCode (*integrator)(adfinitasSystem*)){

    system->gravitationalConstant = (gravitationalConstant != 0)? gravitationalConstant : _adfinitasGravitationalConstant;
    system->timeStep = timeStep;
    system->totalSteps = (unsigned long)(ceill(simulationTime / timeStep));
    system->body = (adfinitasBody*)malloc(sizeof(adfinitasBody));
    system->bodyNumber = 0;
    if(system->body == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    system->integrator = integrator;
    strcpy(system->name, name);

    return _XI_RETURN_OK;
}

void adfinitasClearBody(adfinitasBody* body){
    adfinitasClearTracksChain(body);
    return;
}

xiReturnCode adfinitasInitBody(adfinitasBody* body, char* name, long double staticMass, long double staticRadius){
    strcpy(body->name, name);
    body->staticMass = staticMass;
    body->staticRadius = staticRadius;
    body->firstTrack = (adfinitasTrack*)malloc(sizeof(adfinitasTrack));
    if(body->firstTrack == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    return _XI_RETURN_OK;
}

void adfinitasClearTracksChain(adfinitasBody* body){
    adfinitasTrack* track = NULL;
    track = body->firstTrack;
    while(track->next != NULL) track = track->next;
    while(track->before != NULL){
        track = track->before;
        _adfinitasClearTrack(track->next);
    }
    free(body->firstTrack);
    return;
}

void _adfinitasClearTrack(adfinitasTrack* track){
    free(track->time);
    free(track->position);
    free(track->velocity);
    free(track->acceleration);
    if(track->before != NULL) track->before->next = track->next;
    if(track->next != NULL) track->next->before = track->before;
    free(track);
    return;
}

xiReturnCode adfinitasInitTrack(adfinitasTrack* track, unsigned long totalStep, adfinitasTrack* beforePointer){
    track->steps = 0;
    track->totalSteps = totalStep;
    track->time = (long double*)malloc(totalStep * sizeof(long double));
    if(track->time == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    track->position = (xiVector3*)malloc(totalStep * sizeof(xiVector3));
    if(track->position == NULL){
        free(track->time);
        return _XI_RETURN_ALLOCATION_ERROR;
    } 
    track->velocity = (xiVector3*)malloc(totalStep * sizeof(xiVector3));
    if(track->velocity == NULL){
        free(track->time);
        free(track->position);
        return _XI_RETURN_ALLOCATION_ERROR;
    } 
    track->acceleration = (xiVector3*)malloc(totalStep * sizeof(xiVector3));
    if(track->acceleration == NULL){
        free(track->time);
        free(track->position);
        free(track->acceleration);
        return _XI_RETURN_ALLOCATION_ERROR;
    } 
    track->before = beforePointer;
    if(track->before != NULL) track->before->next = track;
    track->next = NULL;
    return _XI_RETURN_OK;
}

#include "libXi.h"
#include <math.h>
#include <stdlib.h>

// ====== Vector Functions ======

void xiFuncVector(long double (*func)(long double), xiVector* result){
    unsigned long index = 0;
    for(index = 0; index < result->length; ++index) result->vec[index] = (*func)(result->vec[index]);
    return;
}

xiReturnCode xiFunc2Vector(long double (*func)(long double, long double), xiVector a, xiVector* result){
    unsigned long index = 0;
    if(a.length != result->length) return _XI_RETURN_VECTOR_LENGTH_NOT_EQUAL;
    for(index = 0; index < a.length; ++index) result->vec[index] = (*func)(result->vec[index], a.vec[index]);
    return _XI_RETURN_OK;
}

xiReturnCode xiAppendVector(xiVector* result, xiVector a){
    long double* tmpPointer = NULL;
    unsigned long index = 0;
    unsigned long newLength = 0;
    newLength = a.length + result->length;
    tmpPointer = (long double*)realloc((void *)result->vec, sizeof(long double) * newLength);
    if(tmpPointer == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    result->vec = tmpPointer;
    for(index = result->length; index < newLength; ++index) result->vec[index] = a.vec[index - result->length];
    result->length = newLength;
    return _XI_RETURN_OK;
}

xiReturnCode xiCopyVector(xiVector a, xiVector* result){
    unsigned long index = 0;
    if(result->length != 0) free(result->vec);
    result->length = a.length;
    result->vec = (long double*)malloc(sizeof(long double) * a.length);
    if(result->vec == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    for(index = 0; index < a.length; ++index) result->vec[index] = a.vec[index];
    return _XI_RETURN_OK;
}

void xiDelVector(xiVector* a){
    free(a->vec);
    return;
}

xiReturnCode xiFillNumberVector(xiVector* a, unsigned long length, long double number){
    unsigned long index = 0;
    a->length = length;
    a->vec = (long double*)malloc(sizeof(long double) * length);
    if(a->vec == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    for(index = 0; index < length; ++index) a->vec[index] = number;
    return _XI_RETURN_OK;
}

xiReturnCode xiFillVector(xiVector* a, unsigned long length, long double vector[]){
    unsigned long index = 0;
    a->length = length;
    a->vec = (long double*)malloc(sizeof(long double) * length);
    if(a->vec == NULL) return _XI_RETURN_ALLOCATION_ERROR;
    for(index = 0; index < length; ++index) a->vec[index] = vector[index];
    return _XI_RETURN_OK;
}

void xiInitVector(xiVector* a){
    a->length = 0;
    a->vec = NULL; 
    return;
}

// ====== 3D Vector Functions ======

void xiFuncVector3(long double (*func)(long double), xiVector3 a, xiVector3* result){
    result->x = (*func)(a.x);
    result->y = (*func)(a.y);
    result->z = (*func)(a.z);
    return;
}

void xiFunc2Vector3(long double (*func)(long double, long double), xiVector3 a, xiVector3 b, xiVector3* result){
    result->x = (*func)(a.x, b.x);
    result->y = (*func)(a.y, b.y);
    result->z = (*func)(a.z, b.z);
    return;
}

long double xiProductInnerVector3(xiVector3 a, xiVector3 b){
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

void xiProductOuterVector3(xiVector3 a, xiVector3 b, xiVector3* result){
    result->x = (a.y * b.z) - (a.z * b.y);
    result->y = (a.z * b.x) - (a.x * b.z);
    result->z = (a.x * b.y) - (a.y - b.x);
    return;
}

long double xiModVector3(xiVector3 a){
    return sqrtl((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

void xiMulNumberVector3(long double k, xiVector3 a, xiVector3* result){
    result->x = k * a.x;
    result->y = k * a.y;
    result->z = k * a.z;
    return;
}

void xiInitVector3(xiVector3* a, long double x, long double y, long double z){
    a->x = x;
    a->y = y;
    a->z = z;
    return;
}

// ====== Basic Functions ======

long double xiDiv(long double a, long double b){ return a / b; }
long double xiMul(long double a, long double b){ return a * b; }
long double xiSub(long double a, long double b){ return a - b; }
long double xiAdd(long double a, long double b){ return a + b; }
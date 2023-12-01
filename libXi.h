/*
    @libXi.h
    Some useful functions and data types for Umaru Aya's usage.
    2023-11-27
*/

#ifndef _LIB_XI
#define _LIB_XI

// Return Code
typedef enum {
    _XI_RETURN_OK = 0,
    _XI_RETURN_ALLOCATION_ERROR,
    _XI_RETURN_VECTOR_LENGTH_NOT_EQUAL,
    _XI_RETURN_NO_ENOUGH_ITEM,
    _XI_RETURN_FILE_ERROR,
    _XI_RETURN_MISMATCH_ITEM,
    _XI_RETURN_DIVIDE_BY_ZERO
    } xiReturnCode;

// 3D Vector type
typedef struct {
    long double x;
    long double y;
    long double z;
    } xiVector3;

// Vector type
typedef struct {
    unsigned long length;
    long double* vec;
} xiVector;


// ====== Vector Functions ======
// Result=func.(Result) for vector;
void xiFuncVector(long double (*func)(long double), xiVector* result);
// Result=func.(Result, A) for vector;
xiReturnCode xiFunc2Vector(long double (*func)(long double, long double), xiVector a, xiVector* result);
// Append vector A after vector Result;
xiReturnCode xiAppendVector(xiVector* result, xiVector a);
// Copy vector A to vector Result;
xiReturnCode xiCopyVector(xiVector a, xiVector* result);
// Delete vector A;
void xiDelVector(xiVector* a);
// Fill A with Number length Length;
xiReturnCode xiFillNumberVector(xiVector* a, unsigned long length, long double number);
// Fill A with Vector length Length;
xiReturnCode xiFillVector(xiVector* a, unsigned long length, long double vector[]);
// Initialize vector A;
void xiInitVector(xiVector* a);

// ====== 3D Vector Functions ======
// Initialize 3d vector A from (x, y, z); 
void xiInitVector3(xiVector3* a, long double x, long double y, long double z);
// Return |a| from 3d vector;
long double xiModVector3(xiVector3 a);
// Result=func.(A) for 3d vector;
void xiFuncVector3(long double (*func)(long double), xiVector3 a, xiVector3* result);
// Result=func.(A, B) for 3d vector;
void xiFunc2Vector3(long double (*func)(long double, long double), xiVector3 a, xiVector3 b, xiVector3* result);
// Return a.b from 3d vector;
long double xiProductInnerVector3(xiVector3 a, xiVector3 b);
// Result=aXb from 3d vector;
void xiProductOuterVector3(xiVector3 a, xiVector3 b, xiVector3* result);
// Result=ka from 3d vector;
void xiMulNumberVector3(long double k, xiVector3 a, xiVector3* result);

// ====== Basic Functions ======
// Return a+b, a-b, a*b, a/b simply;
long double xiAdd(long double a, long double b);
long double xiSub(long double a, long double b);
long double xiMul(long double a, long double b);
long double xiDiv(long double a, long double b);

#endif
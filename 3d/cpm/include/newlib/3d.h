/*
 * 3d.h
 *
 * Copyright (c) 2022 Phillip Stevens
 * Create Time: October 2022
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted,free of charge,to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),to deal
 * in the Software without restriction,including without limitation the rights
 * to use,copy,modify,merge,publish,distribute,sublicense,and/or sell
 * copies of the Software,and to permit persons to whom the Software is
 * furnished to do so,subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR
 * IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER
 * LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * 3D homogeneous coordinate definition
 * https://en.wikipedia.org/wiki/Homogeneous_coordinates
 *
 * project 3D coords onto 2D screen:
 * https://stackoverflow.com/questions/724219/how-to-convert-a-3d-point-into-2d-perspective-projection
 *
 * transformation matrix:
 * https://www.tutorialspoint.com/computer_graphics/3d_transformation.htm
 *
 */

include(__link__.m4)
undefine(`m4_SCCZ80_NOLIB')

#ifndef _3D_H
#define _3D_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>

#define MATRIX_ORDER    4           // order for 3D homogeneous coordinate graphics

#ifdef __MATH_AM9511

    #define FLOAT       float_t

    #define INV(x)      1/(x)
    #define SQR(x)      sqr(x)
    #define SQRT(x)     sqrt(x)
    #define INVSQRT(x)  1/SQRT(x)

    #define COS(x)      cos(x)
    #define SIN(x)      sin(x)
    #define TAN(x)      tan(x)

#elif __MATH_MATH32

    #define FLOAT       float_t

    #define INV(x)      inv(x)
    #define SQR(x)      sqr(x)
    #define SQRT(x)     sqrt(x)
    #define INVSQRT(x)  invsqrt(x)

    #define COS(x)      cos(x)
    #define SIN(x)      sin(x)
    #define TAN(x)      tan(x)

#elif __MATH_MATH16

    #define FLOAT       _Float16

    #define INV(x)      invf16(x)
    #define SQR(x)      ((x)*(x))
    #define SQRT(x)     sqrtf16(x)
    #define INVSQRT(x)  invsqrtf16(x)

    #define COS(x)      cosf16(x)
    #define SIN(x)      sinf16(x)
    #define TAN(x)      tanf16(x)

#else

    #define FLOAT       float_t

    #define INV(x)      1/(x)
    #define SQR(x)      ((x)*(x))
    #define SQRT(x)     sqrt(x)
    #define INVSQRT(x)  1/SQRT(x)

    #define COS(x)      cos(x)
    #define SIN(x)      sin(x)
    #define TAN(x)      tan(x)

#endif


/****************************************************************************/
/***          Type Definitions                                            ***/
/****************************************************************************/

typedef struct vector_s
{
    FLOAT x;            // x dimension
    FLOAT y;            // y dimension
    FLOAT z;            // z dimension
    FLOAT w;            // w dimension
} vector_t;


typedef struct matrix_s // homogeneous coordinate system
{
    FLOAT e[MATRIX_ORDER*MATRIX_ORDER];
} matrix_t;


/****************************************************************************/
/***        Function Definitions                                          ***/
/****************************************************************************/

/* Scale a vector by m, but don't touch w dimension */
__OPROTO(,,void,,scale_v,vector_t * vect,FLOAT scale)

/* Produce a unit vector */
__OPROTO(,,void,,unit_v,vector_t * vect)

/* Produce a dot product between vectors */
__OPROTO(,,FLOAT,,dot_v,vector_t * vect1,vector_t * vect2)

/* Vector Matrix Multiplication */
__OPROTO(,,void,,mult_v,vector_t * vect,matrix_t * multiplier)

/* Produce an identity matrix */
__OPROTO(,,void,,identity_m,matrix_t * matrix)

/* Produce a transformation (translation) matrix */
__OPROTO(,,void,,translate_m,matrix_t * matrix,FLOAT x,FLOAT y,FLOAT z)

/* Produce a transformation (scale) matrix */
__OPROTO(,,void,,scale_m,matrix_t * matrix,FLOAT x,FLOAT y,FLOAT z)

/* Produce a transformation (shear) matrix */
__OPROTO(,,void,,shear_m,matrix_t * matrix,FLOAT x,FLOAT y,FLOAT z)

/* Rotation in x dimension */
__OPROTO(,,void,,rotx_m,matrix_t * matrix,FLOAT angle)

/* Rotation in y dimension */
__OPROTO(,,void,,roty_m,matrix_t * matrix,FLOAT angle)

/* Rotation in z dimension */
__OPROTO(,,void,,rotz_m,matrix_t * matrix,FLOAT angle)

/* Matrix Multiplication */
__OPROTO(,,void,,mult_m,matrix_t * multiplicand,matrix_t * multiplier)


#ifdef __cplusplus
}
#endif

#endif  /* _3D_H  */

/*
 * shear_m.c
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#if __SCCZ80
#include "include/sccz80/3d.h"
#elif __SDCC
#include "include/sdcc/3d.h"
#endif


/****************************************************************************/
/***       Function                                                       ***/
/****************************************************************************/


/* Produce a transformation (shear) matrix */
void shear_m(matrix_t * matrix, FLOAT x, FLOAT y, FLOAT z)
{
    matrix_t shear;

    FLOAT invx = INV(x);
    FLOAT invy = INV(y);
    FLOAT invz = INV(z);

    identity_m( &shear );

    shear.e[1] = y*invx;
    shear.e[2] = z*invx;
    shear.e[4] = x*invy;
    shear.e[6] = z*invy;
    shear.e[8] = x*invz;
    shear.e[9] = y*invz;

    mult_m( matrix, &shear );
}

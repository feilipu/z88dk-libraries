/*
 * 3D animated graphics over the REGIS protocol for z88dk Z80 systems
 *
 * Copyright (c) 2022 Phillip Stevens
 *
 * Derived from original C++ code by:
 *
 * Copyright (C) 2021 Adam Williams <broadcast at earthling dot net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 *
 * 
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

// ZSDCC compile
// zcc +cpm -clib=sdcc_iy -v -m --list -llib/cpm/regis -llib/cpm/3d --math32 --max-allocs-per-node100000 demo_3d.c -o 3d -create-app

// SCCZ80 compile
// zcc +cpm -clib=new -v -m -O2 --list -llib/cpm/regis -llib/cpm/3d --math32 demo_3d.c -o 3d -create-app

// SCCZ80 compile with math16 (16-bit floating point)
// zcc +cpm -clib=new -v -m -O2 --list -llib/cpm/regis -llib/cpm/3df16 --math16 demo_3d.c -o 3d -create-app


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


// REGIS library
#include <lib/cpm/regis.h>

// 3D library
#include <lib/cpm/3d.h>

#define W 480
#define H 480
#define NEAR -100.0
#define FAR 100.0
#define FOV 3.0 // degrees

#include "models.h"

// these are the demonstration options
#define CUBE '1'
#define ICOS '2'
#define GEAR '3'
#define GLXGEARS '4'

// select an initial demonstration from above options
uint8_t demo = CUBE;

uint8_t animate = 1;
FLOAT user_rotx = 0;
FLOAT user_roty = 0;

FLOAT halfWidth;
FLOAT halfHeight;

// REGIS library
window_t mywindow;

// create the matrix which transforms from 3D to 2D
matrix_t clipMatrix;


void setupClipMatrix(matrix_t * matrix, FLOAT fov, FLOAT aspectRatio, FLOAT near, FLOAT far)
{
    FLOAT f = 1.0/TAN(fov * 0.5f);

    identity_m( matrix );

    matrix->e[0] = f * aspectRatio;
    matrix->e[5] = f;
    matrix->e[10] = (far + near) / (far - near);
    matrix->e[11] = 1.0f; /* this 'plugs' the old z into w */
    matrix->e[14] = (2.0f * near * far) / (near - far);
    matrix->e[15] = 0.0f;
}

void begin_projection()
{
    halfWidth = (FLOAT)W * 0.5f;
    halfHeight = (FLOAT)H * 0.5f;
    setupClipMatrix(&clipMatrix, FOV * (M_PI / 180.0f), (FLOAT)W/(FLOAT)H, NEAR, FAR);
}

void read_point(point_t * point, unsigned char **ptr)
{
    memcpy((uint8_t *)point, *ptr, sizeof(point_t));
    (*ptr) += sizeof(point_t);
}


// draw the model
void regis_plot(const point_t *model, uint16_t count, matrix_t * transform, intensity_t intensity, uint8_t do_init);

void regis_plot(const point_t *model, uint16_t count, matrix_t * transform, intensity_t intensity, uint8_t do_init)
{
    if(do_init)
    {
        window_new( &mywindow, H, W );
        window_clear ( &mywindow );
    }

    draw_intensity( &mywindow, intensity );

    unsigned char *ptr = (unsigned char*)model;

    for(uint16_t i = 0; i < count; ++i)
    {
        point_t point;
        vector_t vertex;

        read_point(&point, &ptr);

        vertex.x = point.x;
        vertex.y = point.y;
        vertex.z = point.z;

        mult_v(&vertex,transform);
        scale_v(&vertex, 1.0/(vertex.w));

/* TODO: Clipping here */

        vertex.x = (vertex.x * (FLOAT)W) / (2.0f * vertex.w) + halfWidth;
        vertex.y = (vertex.y * (FLOAT)H) / (2.0f * vertex.w) + halfHeight;

        if(point.begin_poly)
        {
            draw_abs( &mywindow, (uint16_t)vertex.x, (uint16_t)vertex.y );
        }
        else
        {
            draw_line_abs( &mywindow, (uint16_t)vertex.x, (uint16_t)vertex.y );
        }
    }

    if(do_init)
    {
        window_write( &mywindow );
        window_close( &mywindow );
    }
}


void glxgears_loop()
{
    static FLOAT rotz = 0;
    static FLOAT roty = 30.0 / 180 * M_PI;
    static FLOAT step = -1.0 / 180 * M_PI;

    window_new( &mywindow, H, W );
    window_clear ( &mywindow );

    matrix_t transform;
    matrix_t user_rotx_;
    matrix_t user_roty_;
    matrix_t view_rotx;
    matrix_t view_roty;
    matrix_t view_transform;
    matrix_t big_matrix;
    matrix_t rz;

    translate_m(&transform, -1.0, 2.0, 0);
    rotx_m(&user_rotx_, user_rotx);
    roty_m(&user_roty_, user_roty);
    rotx_m(&view_rotx, 0.0 / 180 * M_PI);
    roty_m(&view_roty, roty);
    rotz_m(&rz, rotz);
    translate_m(&view_transform, 0, 1.0, 20.0);

    big_matrix = rz;
    mult_m(&big_matrix, &transform);
    mult_m(&big_matrix, &view_roty);
    mult_m(&big_matrix, &view_rotx);
    mult_m(&big_matrix, &user_rotx_);
    mult_m(&big_matrix, &user_roty_);
    mult_m(&big_matrix, &view_transform);
    mult_m(&big_matrix, &clipMatrix);

    regis_plot(glxgear1, sizeof(glxgear1) / sizeof(point_t), &big_matrix, _R, 0);

    window_write( &mywindow );
    window_reset( &mywindow );

    translate_m(&transform, 5.2, 2.0, 0);
    rotz_m(&rz, -2.0 * rotz + 9.0 / 180 * M_PI);

    big_matrix = rz;
    mult_m(&big_matrix, &transform);
    mult_m(&big_matrix, &view_roty);
    mult_m(&big_matrix, &view_rotx);
    mult_m(&big_matrix, &user_rotx_);
    mult_m(&big_matrix, &user_roty_);
    mult_m(&big_matrix, &view_transform);
    mult_m(&big_matrix, &clipMatrix);

    regis_plot(glxgear2, sizeof(glxgear2) / sizeof(point_t), &big_matrix, _G, 0);

    window_write( &mywindow );
    window_reset( &mywindow );

    translate_m(&transform, -1.1, -4.2, 0);
    rotz_m(&rz, -2.0 * rotz + 30.0 / 180 * M_PI);

    big_matrix = rz;
    mult_m(&big_matrix, &transform);
    mult_m(&big_matrix, &view_roty);
    mult_m(&big_matrix, &view_rotx);
    mult_m(&big_matrix, &user_rotx_);
    mult_m(&big_matrix, &user_roty_);
    mult_m(&big_matrix, &view_transform);
    mult_m(&big_matrix, &clipMatrix);

    regis_plot(glxgear3, sizeof(glxgear3) / sizeof(point_t), &big_matrix, _B, 0);

    window_write( &mywindow );
    window_close( &mywindow );

    if(animate)
    {
        rotz += 2.0 / 180 * M_PI;
//      roty += step;
        if((step > 0 && roty >= 45.0 / 180 * M_PI) ||
            (step < 0 && roty <= -45.0 / 180 * M_PI))
        {
            step = -step;
        }
    }
}


void gear_loop()
{
    static FLOAT rotz = 0;
    static FLOAT roty = 0;
    static FLOAT step2 = 1.0 / 180 * M_PI;

    matrix_t transform;
    matrix_t user_rotx_;
    matrix_t user_roty_;
    matrix_t ry;
    matrix_t rz;
    matrix_t big_matrix;

    translate_m(&transform, 0, 0, 8.0);
    rotx_m(&user_rotx_, user_rotx);
    roty_m(&user_roty_, user_roty);
    roty_m(&ry, roty);
    rotz_m(&rz, rotz);

    big_matrix = ry;
    mult_m(&big_matrix, &rz);
    mult_m(&big_matrix, &user_rotx_);
    mult_m(&big_matrix, &user_roty_);
    mult_m(&big_matrix, &transform);
    mult_m(&big_matrix, &clipMatrix);

    regis_plot(gear, sizeof(gear) / sizeof(point_t), &big_matrix, _W, 1);

    if(animate)
    {
        rotz += 2.0 / 360 * M_PI * 2;
//      roty += step2;
        if((step2 > 0 && roty >= 45.0 / 180 * M_PI) ||
            (step2 < 0 && roty <= -45.0 / 180 * M_PI))
        {
            step2 = -step2;
        }
    }
}


void icos_loop(void)
{
    static FLOAT rotz = 0;
    static FLOAT roty = 0;

    matrix_t transform;
    matrix_t user_rotx_;
    matrix_t user_roty_;
    matrix_t rx;
    matrix_t rz;
    matrix_t ry;
    matrix_t big_matrix;

    translate_m(&transform, 0, 0, 8.0);
    rotx_m(&user_rotx_, user_rotx);
    roty_m(&user_roty_, user_roty);
    rotx_m(&rx, M_PI/2);
    roty_m(&ry, roty);
    rotz_m(&rz, rotz);

    big_matrix = rx;
    mult_m(&big_matrix, &ry);
    mult_m(&big_matrix, &rz);
    mult_m(&big_matrix, &user_rotx_);
    mult_m(&big_matrix, &user_roty_);
    mult_m(&big_matrix, &transform);
    mult_m(&big_matrix, &clipMatrix);

    regis_plot(icos, sizeof(icos) / sizeof(point_t), &big_matrix, _W, 1);

    if(animate)
    {
//      rotz += .25 / 360 * M_PI * 2;
        roty += 2.0 / 360 * M_PI * 2;
    }
}


void cube_loop(void)
{
    static FLOAT rotz = 0;
    static FLOAT roty = 0;

    matrix_t transform;
    matrix_t user_rotx_;
    matrix_t user_roty_;
    matrix_t ry;
    matrix_t rz;
    matrix_t big_matrix;

    translate_m(&transform, 0, 0, 10.0);
    rotx_m(&user_rotx_, user_rotx);
    roty_m(&user_roty_, user_roty);
    roty_m(&ry, roty);
    rotz_m(&rz, rotz);

    big_matrix = ry;
    mult_m(&big_matrix, &rz);
    mult_m(&big_matrix, &user_rotx_);
    mult_m(&big_matrix, &user_roty_);
    mult_m(&big_matrix, &transform);
    mult_m(&big_matrix, &clipMatrix);

    regis_plot(cube, sizeof(cube) / sizeof(point_t), &big_matrix, _W, 1);

    if(animate)
    {
        rotz += 2.0 / 360 * M_PI * 2;
//      roty += .5 / 360 * M_PI * 2;
    }
}


void loop(void) {
    uint8_t key;
    if( key = getc(stdin) != 0 ) demo = key;
    switch(demo)
    {
        case CUBE:
            cube_loop();
            break;
        case ICOS:
            icos_loop();
            break;
        case GEAR:
            gear_loop();
            break;
        case GLXGEARS:
            glxgears_loop();
            break;
        default:
            exit(0);
    }
}

int main(void)
{
    begin_projection();

    while(1)
    {
        loop();
    }

    return 0;
}

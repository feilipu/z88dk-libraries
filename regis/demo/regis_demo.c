/*
 * Demo name   : regis_demo 
 * Author      : Phillip Steven @feilipu
 * Version     : V0.1
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <lib/rc2014/regis.h>

// ZSDCC compile
// zcc +rc2014 -subtype=cpm -v -m -SO3 --list -llib/rc2014/regis --c-code-in-asm --max-allocs-per-node200000 regis_demo.c -o regis_demo -create-app

// SCCZ80 compile
// zcc +rc2014 -subtype=cpm -clib=new -v -m -SO3 --list -llib/rc2014/regis regis_demo.c -o regis_demo -create-app

// Expected output (where ^ is ESC character).
// ^P1pS(E)W(I(M))P[400,200]V[-200,+200]V[200,100]P[600,100]V(W(S1))[-100,][,-050][+100,][,+050]^\


#pragma printf = "%s %c %f %g"     // Enables %s %c %f %g only. Others: %i %X %lx %u

window_t mywindow;

int main(void)
{

    window_new( &mywindow, 768, 480 );

    draw_intensity( &mywindow, MA);
    draw_abs( &mywindow, 400, 200 );
    draw_line_rel( &mywindow, -200, 200 );
    draw_line_abs( &mywindow, 200, 100);

    draw_abs( &mywindow, 600, 100 );
    draw_box_fill( &mywindow, -100, -50);

    window_write( &mywindow );
    window_close( &mywindow );

    return 0;
}

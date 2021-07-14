/*
 * Demo name   : regis_demo 
 * Author      : Phillip Stevens @feilipu
 * Version     : V0.2
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <lib/rc2014/regis.h>

// ZSDCC compile
// zcc +rc2014 -subtype=cpm -v -m --list -llib/rc2014/regis --max-allocs-per-node100000 regis_demo.c -o regis -create-app

// SCCZ80 compile
// zcc +rc2014 -subtype=cpm -clib=new -v -m -O2 --list -llib/rc2014/regis regis_demo.c -o regis -create-app

// Expected output (where ^ is ESC character).
// ^P1pS(E)W(I(M))P[600,200]V[][-200,+200]V[][400,100]W(I(G))P[700,100]V(B)[+050,][,+050][-050,](E)V(W(S1))(B)[-100,][,-050][+100,](E)V(W(S1,E))(B)[-050,][,-025][+050,](E)W(I(C))P[200,100]C(A-180)[+100]C(A+180)[+050]W(I(B))P[200,300]C(W(S1))[+100]C(W(S1,E))[+050]W(I(W))T(S01)"hello world"^\


window_t mywindow;

int main(void)
{

    window_new( &mywindow, 768, 480 );

    draw_intensity( &mywindow, M);

    draw_abs( &mywindow, 600, 200 );
    draw_line_rel( &mywindow, -200, 200 );
    draw_line_abs( &mywindow, 400, 100);

    draw_intensity( &mywindow, G);

    draw_abs( &mywindow, 700, 100 );
    draw_box( &mywindow, 50, 50);
    draw_box_fill( &mywindow, -100, -50);
    draw_unbox_fill( &mywindow, -50, -25);

    draw_intensity( &mywindow, C);

    draw_abs( &mywindow, 200, 100 );
    draw_arc( &mywindow, 100, -180);
    draw_arc( &mywindow, 50, 180);

    draw_intensity( &mywindow, B);

    draw_abs( &mywindow, 200, 300 );
    draw_circle_fill( &mywindow, 100);
    draw_uncircle_fill( &mywindow, 50);

    draw_intensity( &mywindow, W);

    draw_text( &mywindow, "hello world", 1);

    window_write( &mywindow );
    window_close( &mywindow );

    return 0;
}

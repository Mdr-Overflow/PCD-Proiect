#include <stdio.h>
#include <stdlib.h>

int main() {
    
 char filename[] = "./e2.jpg"; // This is an example, replace with your filename
    char cmd[512];

    snprintf(cmd, sizeof(cmd), "cd .. && cd Operations && gcc menu.c bmp2jpeg.c bmp2png.c jpeg2bmp.c jpeg2png.c png2bmp.c png2jpeg.c bmp_tint.c jpeg_blurr.c jpeg_grayscale.c jpeg_tint.c png_blurr.c png_grayscale.c png_tint.c bmp_stats.c png_stats.c jpeg_stats.c -o menu -ljpeg -lpng && ./menu %s", filename);
    system(cmd);

}
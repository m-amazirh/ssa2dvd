#ifndef SUBPICTURE_H
#define SUBPICTURE_H

#include <gd.h>
#include <ass/ass.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>



typedef struct _Color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color;

typedef struct _Palette
{
    Color background;
    Color foreground;
    Color outline;
} Palette;

typedef struct _Subpicture
{
    long long start;
    long long end;
    int w;
    int h;
    Color *bitmap;
    Palette palette;
    int id;
} Subpicture;

#include "utils.h"

Subpicture *sp_new(int id, int w, int h, long long start, Palette *p);
void sp_free(Subpicture * s);
void sp_draw_subtitles(Subpicture * s, ASS_Image * images);
void sp_reduce_colors(Subpicture *s);
void sp_save(Subpicture * s, FILE * out);
void sp_append_info_to_xml(Subpicture * s, char *image_filename,
                                   FILE * xml_out);
int color_compare(Color *c1, Color *c2);
void color_copy(Color *dst, Color *src);
Color *color_of_pixel(Subpicture *s, int x, int y);
uint32_t color_distance(Color *c1, Color *c2);

#endif

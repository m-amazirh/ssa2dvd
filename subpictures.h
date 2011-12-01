#ifndef SUBPICTURE_H
#define SUBPICTURE_H

#include <gd.h>
#include <ass/ass.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct _rectangle{
    int x1,y1;
    int x2,y2;
    struct _rectangle *next;
} rectangle;

typedef struct _subpicture
{
    long long start;/* time position when this subpicture is first displayed*/
    long long end;/* time position when this subpicture is hidden*/
    int canvas_w;/*canvas width*/
    int canvas_h;/*canvas height*/
    gdImagePtr canvas;/*canvas used to draw subtitles*/
    rectangle *first_rectangle;

    int id;
} subpicture;

rectangle *rectangle_new();
void rectangle_modify_coordinates(rectangle *r, int a,int b, int c,int d);
int rectangle_get_width(rectangle *r);
int rectangle_get_height(rectangle *r);
int rectangle_can_be_fitted_to_area(rectangle *r, rectangle *a);
void rectangle_expand_to_fit_area(rectangle *r,rectangle *a);
void rectangle_free(rectangle *r);

subpicture *subpicture_new();
void subpicture_free(subpicture * s);
void subpicture_draw_subtitles(subpicture * s, ASS_Image * images);
int subpicture_reduce_colors(subpicture *s, int new_palette_size);
void subpicture_get_palette(subpicture *s, int **palette, int *palette_size);
void subpicture_remap_colors(subpicture * s);
void subpicture_make_subtitles_backround_opaque(subpicture *s,ASS_Image *images);
void subpicture_save(subpicture * s, FILE * out);
void subpicture_append_info_to_xml(subpicture * s, char *image_filename,
                                   FILE * xml_out);
void subpicture_init(subpicture * s, int id, int w, int h, long long start);

char *long2time(long long l);
#endif

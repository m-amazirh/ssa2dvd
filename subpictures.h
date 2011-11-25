#ifndef SUBPICTURE_H
#define SUBPICTURE_H

#include <gd.h>
#include <ass/ass.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct _subpicture
{
    long long start;/* time position when this subpicture is first displayed*/
    long long end;/* time position when this subpicture is hidden*/
    int canvas_w;/*canvas width*/
    int canvas_h;/*canvas height*/
    gdImagePtr canvas;/*canvas used to draw subtitles*/

    int id;
} subpicture;

subpicture *subpicture_new();
void subpicture_free(subpicture * s);
void subpicture_draw_subtitles(subpicture * s, ASS_Image * images);
void subpicture_remap_colors(subpicture * s, int *colors, int colors_count);
void subpicture_prepare_for_save(subpicture * s);
void subpicture_save(subpicture * s, FILE * out);
void subpicture_append_info_to_xml(subpicture * s, char *image_filename,
                                   FILE * xml_out);
void subpicture_init(subpicture * s, int id, int w, int h, long long start);

char *long2time(long long l);
int min_int(int x, int y);
int max_int(int x, int y);

#endif

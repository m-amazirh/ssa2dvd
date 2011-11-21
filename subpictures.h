#ifndef SUBPICTURE_H
#define SUBPICTURE_H

#include <gd.h>
#include <ass/ass.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct _subpicture
{
    long long start;
    long long end;
	/*size of the whole canvas representing the subpicture*/
    int canvas_w;
    int canvas_h;
	gdImagePtr canvas;
	
	int id;
} subpicture;

subpicture* subpicture_new();
void subpicture_free(subpicture* s);
void subpicture_blend_images(subpicture* s, ASS_Image *images);
int subpicture_detect_background_color(subpicture* s,int palette_count);
void subpicture_remap_colors(subpicture *s,int *colors,int colors_count);
void subpicture_prepare_for_save(subpicture *s);
void subpicture_save(subpicture* s,FILE *out);
void subpicture_append_info_to_xml(subpicture *s,char *image_filename,FILE *xml_out);
void subpicture_init(subpicture *s,int id,int w,int h,long long start);

char *long2time(long long l);
int min_int(int x,int y);
int max_int(int x,int y);

#endif

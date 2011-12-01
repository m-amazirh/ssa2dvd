#include "subpictures.h"

rectangle *rectangle_new()
{
    rectangle *r = (rectangle *) malloc(sizeof(rectangle));
    r->next = NULL;
    return r;
}

void rectangle_modify_coordinates(rectangle *r, int a,int b,int c,int d)
{
    assert(r != NULL);
    r->x1 = a; r->y1 = b;
    r->x2 = c; r->y2 = d;
}

int rectangle_get_width(rectangle *r)
{
    assert(r != NULL);
    
    return r->x2 - r->x1;
}

int rectangle_get_height(rectangle *r)
{
    assert(r != NULL);

    return r->y2 - r->y1;
}

int rectangle_can_be_fitted_to_area(rectangle *r, rectangle *a)
{
    int a_height = rectangle_get_height(a);
    int r_height = rectangle_get_height(r);

    if ( (r->y1 + r_height) < a->y1 || (a->y1 + a_height) < r->y1){
        return 0;
    }

    return 1;
}

void rectangle_expand_to_fit_area(rectangle *r, rectangle *a)
{

    r->x2 = a->x2;

    if ( r->y1 >= a->y1 ){
        r->y1 = a->y1;
    }

    if ( r->y2 <= a->y2){
        r->y2 = a->y2;
    }
}

void rectangle_free(rectangle *r)
{
    if(r != NULL)
        free(r);
}

subpicture *subpicture_new()
{
    return (subpicture *) malloc(sizeof(subpicture));
}

void subpicture_free(subpicture * s)
{
    if (s != NULL) {
        gdImageDestroy(s->canvas);
        free(s);
    }
}

void subpicture_init(subpicture * s, int id, int w, int h, long long start)
{
    s->id = id;
    s->canvas_w = w;
    s->canvas_h = h;
    s->start = start;
    s->end = -1;

    s->canvas = gdImageCreateTrueColor(s->canvas_w, s->canvas_h);
    int background_color = gdImageColorAllocate(s->canvas, 0, 0, 0);
    gdImageFill(s->canvas, 0, 0, background_color);
}

void subpicture_draw_subtitles(subpicture * s, ASS_Image * images)
{
    ASS_Image *frame = NULL;
    assert(images != NULL);
    assert(s != NULL);
    for (frame = images; frame != NULL; frame = frame->next) {

        if (frame->h == 0 || frame->w == 0)
            continue;

        unsigned char r = (frame->color >> 24) & 0xff;
        unsigned char g = (frame->color >> 16) & 0xff;
        unsigned char b = (frame->color >> 8) & 0xff;
        unsigned char a = (frame->color) & 0xff;
        unsigned char opacity = 255 - a;

        int x, y;

        for (y = 0; y < frame->h; y++) {
            for (x = 0; x < frame->w; x++) {
                int dst_x = frame->dst_x + x;
                int dst_y = frame->dst_y + y;
                unsigned char src_alpha_prenormalized =
                    frame->bitmap[y * frame->stride + x];
                unsigned src_alpha =
                    (src_alpha_prenormalized) * opacity / 255;

                int dst_old_color =
                    gdImageTrueColorPixel(s->canvas, dst_x, dst_y);
                int dst_old_red = gdTrueColorGetRed(dst_old_color);
                int dst_old_green = gdTrueColorGetGreen(dst_old_color);
                int dst_old_blue = gdTrueColorGetBlue(dst_old_color);
                int dst_old_alpha = gdTrueColorGetAlpha(dst_old_color);

                if (((dst_y * s->canvas_w * 4) + dst_x * 4) >
                    (s->canvas_w * s->canvas_h * 4))
                    continue;

                int dst_new_color, dst_new_red, dst_new_green, dst_new_blue,
                    dst_new_alpha;
                dst_new_blue =
                    (src_alpha * b + (255 - src_alpha) * dst_old_blue) / 255;
                dst_new_green =
                    (src_alpha * g + (255 - src_alpha) * dst_old_green) / 255;
                dst_new_red =
                    (src_alpha * r + (255 - src_alpha) * dst_old_red) / 255;
                dst_new_color =
                    gdImageColorAllocate(s->canvas, dst_new_red,
                                         dst_new_green, dst_new_blue);
                gdImageSetPixel(s->canvas, dst_x, dst_y, dst_new_color);
            }
        }
    }
}

void subpicture_remap_colors(subpicture * s)
{
    int x, y, i, j, color;
    int *current_palette = NULL; /* Most used color first, lest used color last 
                                    (see subpicture_get_palette)*/
    int current_palette_size = 0;
    int new_palette[4];

    assert(s != NULL);

    gdImagePtr new_canvas = gdImageCreate(s->canvas_w, s->canvas_h);

    new_palette[0] = gdImageColorAllocate(new_canvas, 0, 0, 0);
    new_palette[1] = gdImageColorAllocate(new_canvas, 255, 255, 255);
    new_palette[2] = gdImageColorAllocate(new_canvas, 127, 127, 127);
    new_palette[3] = gdImageColorAllocate(new_canvas, 90, 90, 90);

    subpicture_reduce_colors(s, 4);
    subpicture_get_palette(s, &current_palette, &current_palette_size);

    assert(current_palette_size <= 4);
    
    for (x = 0; x < s->canvas_w; x++) {
        for (y = 0; y < s->canvas_h; y++) {
            color = gdImagePalettePixel(s->canvas, x, y);

            for (i = 0; color != current_palette[i]; i++)
                ;

            assert(i < 4);

            gdImageSetPixel(new_canvas, x, y, new_palette[i]);
        }
    }

    gdImageColorTransparent(new_canvas, new_palette[0]);
    gdImageDestroy(s->canvas);
    s->canvas = new_canvas;

    free(current_palette);
}

int subpicture_reduce_colors(subpicture *s, int new_palette_size)
{
    int palette_size;
    assert(s != NULL);

    gdImageTrueColorToPalette(s->canvas, 1, new_palette_size);

    palette_size = gdImageColorsTotal(s->canvas);

    return palette_size;
}

void subpicture_get_palette(subpicture *s, int **palette, int *palette_size)
{
    int x, y, i, j, color, tmp;
    int *colors_frequencies = NULL;
    int *p = NULL;
    int p_size ;


    p_size = gdImageColorsTotal(s->canvas);
    p = (int *) calloc(p_size, sizeof(int));

    colors_frequencies = (int *) calloc(p_size, sizeof(int));

    for (i = 0; i < p_size; i++){
        p[i] = -1;
    }

    for (x = 0; x < s->canvas_w; x++) {
        for (y = 0; y < s->canvas_h; y++) {
            color = gdImagePalettePixel(s->canvas, x, y);

            for (i = 0; p[i] != color && p[i] != -1; i++){

            }

            assert(i < p_size);

            if (p[i] == -1)
                p[i] = color;

            colors_frequencies[i]++;
        }
    }

    for (i = 0; i < p_size; i++) {
        for (j = i; j < p_size; j++) {
            if (colors_frequencies[i] < colors_frequencies[j]) {
                tmp = colors_frequencies[i];
                colors_frequencies[i] = colors_frequencies[j];
                colors_frequencies[j] = tmp;

                tmp = p[i];
                p[i] = p[j];
                p[j] = tmp;
            }
        }
    }

    *palette = p;
    *palette_size = p_size;
    free(colors_frequencies);
}

void subpicture_make_subtitles_background_opaque(subpicture *s,ASS_Image *images)
{
    rectangle *first_rect = NULL;
    rectangle *r = NULL;
    ASS_Image *frame = NULL;
    int subtitle_background = gdImageColorExact(s->canvas, 90, 90, 90);/* subtitle background color*/
    int subpicture_background = gdImageColorExactAlpha(s->canvas, 0, 0, 0, gdAlphaTransparent);

    for (frame = images; frame != NULL; frame = frame->next){
        if (frame->h == 0 || frame->w == 0)
            continue;

        for (r = first_rect; r != NULL; r = r->next){
            rectangle area;

            area.x1 = frame->dst_x;
            area.y1 = frame->dst_y;
            area.x2 = frame->dst_x + frame->w;
            area.y2 = frame->dst_y + frame->h;

            if ( rectangle_can_be_fitted_to_area(r, &area) != 0 ){
                rectangle_expand_to_fit_area(r,&area);
                break;
            }
        }


        if (r == NULL){
            r = rectangle_new();
            r->x1 = frame->dst_x;
            r->y1 = frame->dst_y;
            r->x2 = frame->dst_x + frame->w;
            r->y2 = frame->dst_y + frame->h;

            r->next = first_rect;
            first_rect = r;
        }
    }


    while (first_rect != NULL){
        int x,y;

        r = first_rect;
        for (x = r->x1; x <= r->x2; x++){
            for (y = r->y1; y <= r->y2; y++){
                int pixel_color = gdImagePalettePixel(s->canvas, x, y);

                if (pixel_color == subpicture_background ){
                    gdImageSetPixel(s->canvas, x, y, subtitle_background);
                }

            }
        }

        first_rect = r->next;
        rectangle_free(r);
    }

}

void subpicture_save(subpicture * s, FILE * out)
{
    assert(out != NULL);
    gdImagePng(s->canvas, out);
}

void subpicture_append_info_to_xml(subpicture * s, char *image_filename,
                                   FILE * xml_out)
{
    assert(xml_out != NULL);
    assert(s != NULL);

    fprintf(xml_out,
            "\t\t<spu start=\"%s\" end=\"%s\" image=\"%s\" ></spu>\n",
            long2time(s->start), long2time(s->end), image_filename);
}


char *long2time(long long l)
{
    /*12 = ( 4 for ':' and '.', 8 for the numbers ) -> time = "HH:MM:SS.MS */
    char *time = (char *) calloc(12, sizeof(char));

    long long millis = l % 1000;
    long long seconds = l / 1000;
    long long minutes = seconds / 60;
    long long hours = seconds / 3600;

    sprintf(time, "%02lli:%02lli:%02lli.%02lli", hours, minutes % 60,
            seconds % 60, millis / 10);

    return time;
}

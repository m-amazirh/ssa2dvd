#include <math.h>
#include "subpictures.h"

Subpicture *sp_new(int id, int w, int h, long long start, Palette *p)
{
    Subpicture *s = (Subpicture *) malloc(sizeof(Subpicture));
    s->start = start;
    s->w = w;
    s->h = h;
    s->bitmap = (Color *) calloc(w * h, sizeof(Color));
    s->palette = *p;
    
    s->id = id;
    
    int x,y;
    for (y = 0; y < s->h; y++)
        for (x = 0; x < s->w; x++) {
            Color *color = &s->bitmap[(y*s->h) + x];
            color_copy(color, &(s->palette.background));
        }
        
    return s;
}

void sp_free(Subpicture * s)
{
    free(s->bitmap);
    free(s);
}

void sp_draw_subtitles(Subpicture * s, ASS_Image * images)
{
    ASS_Image *frame = NULL;
    assert(images != NULL);
    assert(s != NULL);
    
    for (frame = images; frame != NULL; frame = frame->next) {

        if (frame->h == 0 || frame->w == 0)
            continue;

        uint8_t r = (frame->color >> 24) & 0xff;
        uint8_t g = (frame->color >> 16) & 0xff;
        uint8_t b = (frame->color >> 8) & 0xff;
        uint8_t a = (frame->color) & 0xff;
        uint8_t opacity = 255 - a;

        int x, y;

        for (y = 0; y < frame->h; y++) {
            for (x = 0; x < frame->w; x++) {
                int dst_x = frame->dst_x + x;
                int dst_y = frame->dst_y + y;

                if (dst_y >= s->h && dst_x >= s->w)
                    continue;
                    
                uint8_t k =  (frame->bitmap[y*frame->stride + x]) * opacity / 255;   
                Color *color = &s->bitmap[(dst_x + (dst_y * s->w))];
                color->red = (k * r + (255 - k) * color->red) / 255;
                color->green = (k * g + (255 - k) * color->green) / 255;
                color->blue = (k * b + (255 - k) * color->blue) / 255;
            }
        }
    }
}

void sp_reduce_colors(Subpicture *s)
{
    int x,y;
    Color origin = { 0, 0, 0 };
    Color c1, c2, c3;
    
    uint32_t dist_fg = color_distance(&origin, &(s->palette.foreground));
    uint32_t dist_bg = color_distance(&origin, &(s->palette.background));
    uint32_t dist_ol = color_distance(&origin, &(s->palette.outline));
    uint32_t dist_c1, dist_c2, dist_c3;
    
    if (dist_ol <= dist_fg) {
        if (dist_bg <= dist_ol) {
            c1 = s->palette.background; dist_c1 = dist_bg;
            c2 = s->palette.outline; dist_c2 = dist_ol;
            c3 = s->palette.foreground; dist_c3 = dist_fg;
        } 
        else if (dist_bg >= dist_fg) {
            c1 = s->palette.outline; dist_c1 = dist_ol;
            c2 = s->palette.foreground; dist_c2 = dist_fg;
            c3 = s->palette.background; dist_c3 = dist_bg;
        } 
        else {
            c1 = s->palette.outline; dist_c1 = dist_ol;
            c2 = s->palette.background; dist_c2 = dist_bg;
            c3 = s->palette.foreground; dist_c3 = dist_fg;
        }
    } 
    else if (dist_fg <= dist_ol) {
        if (dist_bg <= dist_fg) {
            c1 = s->palette.background; dist_c1 = dist_bg;
            c2 = s->palette.foreground; dist_c2 = dist_fg;
            c3 = s->palette.outline; dist_c3 = dist_ol;
        } 
        else if (dist_bg >= dist_fg) {
            c1 = s->palette.foreground; dist_c1 = dist_fg;
            c2 = s->palette.outline; dist_c2 = dist_ol;
            c3 = s->palette.background; dist_c3 = dist_bg;
        } 
        else {
            c1 = s->palette.foreground; dist_c1 = dist_fg;
            c2 = s->palette.background; dist_c2 = dist_bg;
            c3 = s->palette.outline; dist_c3 = dist_ol;
        }
    }
    
    for(y=0; y<s->h; y++)
        for(x=0; x<s->w; x++){
            Color *color = &s->bitmap[(y*s->w) + x];
            uint32_t dist_color = color_distance(&origin, color);
            
            if (dist_color < dist_c1 ||
                    (dist_color >= dist_c1 && 
                        dist_color < (dist_c1 + dist_c2)/2 )) {
                *color = c1;
            }
            else if ((dist_color >= (dist_c1 + dist_c2)/2 && 
                                    dist_color < dist_c2) ||
                (dist_color >= dist_c2 && 
                                dist_color < (dist_c2 + dist_c3)/2)) {
                *color = c2;
            }
            else if (dist_color >= (dist_c2 + dist_c3)/2 ) {
                *color = c3;
            }
        }
}

void sp_save(Subpicture * s, FILE * out)
{
    assert( out != NULL && s != NULL );
    
    int x,y, color;
    gdImagePtr canvas;
    canvas = gdImageCreate(s->w, s->h);
        
    for(x = 0; x < s->w; x++)
        for(y = 0; y< s->h; y++){
            Color *c = &s->bitmap[ ((y * s->w) + x)];
            color = gdImageColorResolve(canvas, c->red, c->green, c->blue);
            gdImageSetPixel(canvas, x, y, color);
        }
    
    /* set the background color as transparent */    
    color = gdImageColorExact(canvas,s->palette.background.red, s->palette.background.green, s->palette.background.blue);
    gdImageColorTransparent(canvas, color);
    
    gdImagePng(canvas, out);
    gdImageDestroy(canvas);
}

void sp_append_info_to_xml(Subpicture * s, char *image_filename,
                                   FILE * xml_out)
{
    assert(xml_out != NULL);
    assert(s != NULL);

    fprintf(xml_out,
            "\t\t<spu start=\"%s\" end=\"%s\" image=\"%s\" ></spu>\n",
            long2time(s->start), long2time(s->end), image_filename);
}

int color_compare(Color *c1, Color *c2)
{
    return c1->red   == c2->red   &&
           c1->green == c2->green &&
           c1->blue  == c2->blue;
}

void color_copy(Color *dst, Color *src)
{
    dst->red   = src->red;
    dst->green = src->green;
    dst->blue  = src->blue;
}

Color *color_of_pixel(Subpicture *s, int x, int y)
{
    if (x >= s->w || y >= s->h || x < 0 || y < 0)
        return NULL;
        
    return &s->bitmap[ ((y * s->w) + x)];
}

uint32_t color_distance(Color *c1, Color *c2)
{
    Color c3;
    
    c3.red = abs(c1->red - c2->red);
    c3.green = abs(c1->green - c2->green);
    c3.blue = abs(c1->blue - c2->blue);
    
    return (uint32_t) sqrt(c3.red*c3.red + c3.green*c3.green + c3.blue*c3.blue);
}


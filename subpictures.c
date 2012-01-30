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
    
    for(y=0; y<s->h; y++)
        for(x=0; x<s->w; x++){
            Color *color = &s->bitmap[(y*s->w) + x];
            if( color_compare(color, &(s->palette.background)) == 0 && 
                color_compare(color, &(s->palette.foreground)) == 0    ) {
                    *color = s->palette.outline;
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

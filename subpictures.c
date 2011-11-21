#include "subpictures.h"

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

void subpicture_blend_images(subpicture * s, ASS_Image * images)
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

void subpicture_remap_colors(subpicture * s, int *colors, int colors_count)
{
    int x, y, i, j, color;
    int new_colors[4];

    assert(colors != NULL && s != NULL);

    gdImagePtr new_canvas = gdImageCreate(s->canvas_w, s->canvas_h);

    new_colors[0] = gdImageColorAllocate(new_canvas, 0, 0, 0);
    new_colors[1] = gdImageColorAllocate(new_canvas, 255, 255, 255);
    new_colors[2] = gdImageColorAllocate(new_canvas, 127, 127, 127);
    new_colors[3] = gdImageColorAllocate(new_canvas, 90, 90, 90);

    for (x = 0; x < s->canvas_w; x++) {
        for (y = 0; y < s->canvas_h; y++) {
            color = gdImagePalettePixel(s->canvas, x, y);

            assert(i < 4);
            for (i = 0; color != colors[i]; i++)
	      ;
	    
            gdImageSetPixel(new_canvas, x, y, new_colors[i]);
        }
    }

    gdImageColorTransparent(new_canvas, new_colors[0]);
    gdImageDestroy(s->canvas);
    s->canvas = new_canvas;
}

void subpicture_prepare_for_save(subpicture * s)
{
    int new_colors[4];
    int *colors;
    int *colors_frequencies;
    int color, colors_count;
    int x, y, i, j, tmp;

    assert(s != NULL);

    gdImageTrueColorToPalette(s->canvas, 1, 4);

    colors_count = gdImageColorsTotal(s->canvas);
    assert(colors_count <= 4);

    colors = (int *) calloc(colors_count, sizeof(int));
    colors_frequencies = (int *) calloc(colors_count, sizeof(int));

    for (i = 0; i < colors_count; i++)
        colors[i] = -1;

    for (x = 0; x < s->canvas_w; x++) {
        for (y = 0; y < s->canvas_h; y++) {
            color = gdImagePalettePixel(s->canvas, x, y);

            for (i = 0; colors[i] != color && colors[i] != -1; i++)
	      ;

            assert(i < colors_count);

            if (colors[i] == -1)
                colors[i] = color;

            colors_frequencies[i]++;
        }
    }

    for (i = 0; i < colors_count; i++) {
        for (j = i; j < colors_count; j++) {
            if (colors_frequencies[i] < colors_frequencies[j]) {
                tmp = colors_frequencies[i];
                colors_frequencies[i] = colors_frequencies[j];
                colors_frequencies[j] = tmp;

                tmp = colors[i];
                colors[i] = colors[j];
                colors[j] = tmp;
            }
        }
    }

    subpicture_remap_colors(s, colors, colors_count);

    free(colors);
    free(colors_frequencies);
}

void subpicture_save(subpicture * s, FILE * out)
{
    assert(out != NULL);
    gdImagePng(s->canvas, out);
}

int subpicture_detect_background_color(subpicture * s, int palette_count)
{
    int *color_frequency = (int *) calloc(palette_count, sizeof(int));
    int max_frequency_index = 0;
    int x, y;

    for (x = 0; x < s->canvas_w; x++) {
        for (y = 0; y < s->canvas_h; y++) {

            int color = gdImagePalettePixel(s->canvas, x, y);
            color_frequency[color]++;
        }
    }

    for (x = 0; x < palette_count; x++) {
        if (color_frequency[max_frequency_index] < color_frequency[x])
            max_frequency_index = x;
    }

    int red = gdImageRed(s->canvas, max_frequency_index);
    int blue = gdImageBlue(s->canvas, max_frequency_index);
    int green = gdImageGreen(s->canvas, max_frequency_index);

    free(color_frequency);

    return max_frequency_index;
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

int max_int(int x, int y)
{
    return x > y ? x : y;
}

int min_int(int x, int y)
{
    return x < y ? x : y;
}

int max_in_range(int *arr, int inf, int sup)
{
    int i;
    int max = 0;
    int max_index = -1;
    for (i = inf; i <= sup; i++) {
        if (max < arr[i]) {
            max = arr[i];
            max_index = i;
        }

    }
    return max_index;
}

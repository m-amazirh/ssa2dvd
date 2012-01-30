#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <ass/ass.h>
#include "subpictures.h"
#include "libass_utils.h"
#include "configuration.h"

int main(int argc, char **argv)
{
    Configuration conf;
    LibassConfiguration libass_conf;
    
    ASS_Track *subtitles = NULL;
    ASS_Image *images = NULL;
    Subpicture *current_subpicture = NULL;
    Palette palette;
    
    char *xml_path = NULL;
    FILE *xml_out = NULL;
    
    int detect_change = 0;
    int id = 0;

    conf = process_arguments(argv, argc);

    libass_conf = libass_configure(conf);
    
    /* init palette */
    palette.background.red = 0;
    palette.background.green = 0;
    palette.background.blue = 0;
    
    palette.foreground.red = 255;
    palette.foreground.green = 255;
    palette.foreground.blue = 255;
    
    palette.outline.red = 120;
    palette.outline.green = 120;
    palette.outline.blue = 120;
    /* ************** */

    subtitles = libass_load_subtitles(&libass_conf, conf.subtitles_path);

    libass_override_styles(&libass_conf,subtitles, &palette);

    xml_path =
        (char *) calloc(strlen(conf.output_directory) + strlen(conf.prefix) + 1 +
                        4 + 1, sizeof(char));
    sprintf(xml_path, "%s/%s.xml", conf.output_directory, conf.prefix);

    if (conf.overwrite_files == 0 && check_file_exists(xml_path) == 0){
        printf("Can't create %s. It already exists. Use option '-f' to overwrite existing files.\n",xml_path);
        return EXIT_FAILURE;
    }

    xml_out = fopen(xml_path, "wb");
    if (xml_out == NULL){
        printf("Couldn't create xml file : %s\n", xml_path);
        return EXIT_FAILURE;
    }

    free(xml_path);


    fprintf(xml_out, "<subpictures>\n\t<stream>\n");

    long long pos = 0;
    long long pos_max = determine_total_duration(subtitles);

    for (pos = 0; pos <= pos_max; pos += 25) {
        images = ass_render_frame(libass_conf.renderer, subtitles, pos, &detect_change);

        if (detect_change != 0) {
            if (current_subpicture != NULL) {
                char *image_path = NULL;

                image_path =
                    (char *) calloc(strlen(conf.output_directory) +
                                    strlen(conf.prefix) + 10, 
                                    sizeof(char));

                sprintf(image_path, "%s/%s_%i.png", conf.output_directory,
                        conf.prefix, current_subpicture->id);

                if (current_subpicture->start >= pos)
                    current_subpicture->end = pos - 25;
                else
                    current_subpicture->end = pos;


                if (conf.overwrite_files == 0 && check_file_exists(image_path) == 0){
                    printf("Can't create %s. It already exists. Try option '-f' to overwrite existing files.\n",image_path);
                    return EXIT_FAILURE;
                }

                FILE *out = fopen(image_path, "wb");

                if (out == NULL){
                    printf("Couldn't save subpicture %i to %s\n",current_subpicture->id, image_path);
                    return EXIT_FAILURE;
                }

                sp_save(current_subpicture, out);
                sp_append_info_to_xml(current_subpicture,
                                              image_path, xml_out);
                sp_free(current_subpicture);
                printf("Subpicture %i was saved.\n", id);

                free(image_path);
                fclose(out);

                current_subpicture = NULL;
            }

            if (images != NULL) {
                
                current_subpicture = sp_new(++id, conf.width,
                                conf.height, pos, &palette);
                sp_draw_subtitles(current_subpicture, images);
                sp_reduce_colors(current_subpicture);
            }
        }
    }

    fprintf(xml_out, "</stream></subpictures>");

    fclose(xml_out);
    
    ass_free_track(subtitles);
    ass_renderer_done(libass_conf.renderer);
    ass_library_done(libass_conf.library);

    return EXIT_SUCCESS;
}



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <ass/ass.h>
#include "subpictures.h"

#define PROGRAM_NAME "ssa2dvd"

typedef struct _settings
{
    char *video_standard;
    int subpicture_width;
    int subpicture_height;
    int top_margin;
    int bottom_margin;
    int right_margin;
    int left_margin;
    double display_aspect;
    int overwrite_files;
    int opaque_subtitle_background;
    int dont_override_styles;
    double font_scale;
    char *subtitle_path;
    char *output_directory;
    char *name;
} settings;

ASS_Library *init_ass_library();
ASS_Renderer *init_ass_renderer(ASS_Library * library, settings s);
void override_ass_styles(ASS_Library *library,ASS_Track *track);
long long determine_total_duration(ASS_Track * subtitles);
void usage(char *program_name);
settings process_arguments(char **argv, char argc);
int check_file_exists(const char *path);

void usage(char *program_name)
{
    printf("\nUsage : \n");
    printf
        ("\t%s -w width -h height -o output_directory -s subtitle_file [-r right_margin] [-l left_margin] \n",
         program_name);
}

settings process_arguments(char **argv, char argc)
{
    int c,arg_len,option_index;

    settings s = { NULL,
        -1,
        -1,
        20,
        20,
        60,
        60,
        -1.0,
        0,
        0,
        0,
        1.0,
        NULL,
        NULL,
        "dvd_sub"
    };

    struct option longopts[] = {
        { "subtitles", required_argument, NULL, 's' },
        { "video-standard", required_argument, NULL, 'S' },
        { "video-width", required_argument, NULL, 'w' },
        { "video-height", required_argument, NULL, 'h' },
        { "right-margin", required_argument, NULL, 'r' },
        { "left-margin", required_argument, NULL, 'l' },
        { "display-aspect", required_argument, NULL, 'a'},
        { "output-directory", required_argument, NULL, 'o' },
        { "overwrite-files", no_argument, NULL, 'f' },
        { "opaque-subtitle-background", no_argument, NULL, 'q'},
        { "dont-override-styles", no_argument, NULL, 'z'},
        { "font-scale", required_argument, NULL, 'x'},
        { NULL, 0, NULL, 0 }
    };


    while ((c = getopt_long(argc, argv, "w:h:r:l:s:o:a:x:fqS:",longopts,&option_index)) != -1){

        switch (c){
            case 'w' :
                s.subpicture_width = atoi(optarg);
                break;
            case 'h' :
                s.subpicture_height = atoi(optarg);
                break;
            case 'r' :
                s.right_margin = atoi(optarg);
                break;
            case 'l' :
                s.left_margin = atoi(optarg);
                break;
            case 'a' :
                s.display_aspect = atof(optarg);
                break;
            case 's' :
                arg_len = strlen(optarg);
                s.subtitle_path = (char *) calloc(arg_len + 1, sizeof(char));
                strcpy(s.subtitle_path, optarg);
                break;
            case 'o' :
                arg_len = strlen(optarg);
                s.output_directory = (char *) calloc(arg_len + 1, sizeof(char));
                strcpy(s.output_directory, optarg);
                break;
            case 'f' :
                s.overwrite_files = 1;
                break;
            case 'q' :
                s.opaque_subtitle_background = 1;
                break;
            case 'z' :
                s.dont_override_styles = 1;
                break;
            case 'x' :
                s.font_scale = atof(optarg);
                break;
            case 'S':
                s.video_standard = (char *) calloc(strlen(optarg), sizeof(char));
                strcpy(s.video_standard, optarg);
                break;

            case '?':
            default :
                usage(PROGRAM_NAME);
                exit(1);
        }

    }
    
    if(s.video_standard == NULL){
        /* Do nothing*/
    }
    else if(strcmp(s.video_standard, "ntsc") == 0) {
        s.subpicture_width = 720;
        s.subpicture_height = 480;
    }
    else if(strcmp(s.video_standard, "pal") == 0) {
        s.subpicture_width = 720;
        s.subpicture_height = 576;
    }
    else {
        printf("Unknown video standard : %s . Try \"ntsc\" or \"pal\" instead.\n", s.video_standard);
        usage(PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    if (s.subpicture_width == -1)  { printf("Option -w is missing.\n"); usage(PROGRAM_NAME); exit(1); }
    if (s.subpicture_height == -1) { printf("Option -h is missing.\n"); usage(PROGRAM_NAME); exit(1); }
    if (s.subtitle_path == NULL)   { printf("Option -s is missing.\n"); usage(PROGRAM_NAME); exit(1); }
    if (s.output_directory == NULL)  { printf("Option -o is missing.\n"); usage(PROGRAM_NAME); exit(1); }

    return s;
}

int check_file_exists(const char *path)
{
    return access(path,F_OK);
}

ASS_Library *init_ass_library()
{
    ASS_Library *library = ass_library_init();

    if (library == NULL) {
        printf("Couldn't initialize ass library ...\n");
        exit(1);
    }

    return library;
}

ASS_Renderer *init_ass_renderer(ASS_Library * library, settings s)
{
    assert(library != NULL);

    ASS_Renderer *renderer = ass_renderer_init(library);

    if (renderer == NULL) {
        printf("Couldn't initialize ass library ...\n");
        exit(1);
    }

    ass_set_frame_size(renderer, s.subpicture_width, s.subpicture_height);

    if (s.display_aspect != -1)
        ass_set_aspect_ratio(renderer, s.display_aspect, ((double)s.subpicture_width) / ((double)s.subpicture_height));

    ass_set_margins(renderer, s.top_margin, s.bottom_margin, s.left_margin,
                    s.right_margin);

    ass_set_fonts(renderer, NULL, "Arial", 1, NULL, 1);
    ass_set_font_scale(renderer,s.font_scale);

    return renderer;
}

void override_ass_styles(ASS_Library *library,ASS_Track *track)
{
    assert(track != NULL);
    int styles_count = track->n_styles;
    int i = 0, j = 0;
    char **styles_list = NULL;
    char *setting_value[] = { "&H00FFFFFF", "&H000000FF", "&H00808080", "&H00000000", "1", "4", "0"};
    char *setting_label[] = { "PrimaryColour", "SecondaryColour", "OutlineColour", "BorderStyle", "Outline", "Shadow" };

    printf("Max styles allocated : %i\n",track->max_styles);
    printf("Number styles used : %i\n", track->n_styles);

    styles_list = (char **) calloc(sizeof(char *) * 7, styles_count);

    for (i=0; i< styles_count; i++){
        ASS_Style *style = &(track->styles[i]);
        int len;

        for (j=0;j<7;j++){
            len = strlen(style->Name) + 1 + strlen(setting_label[j]) + 1 + strlen(setting_value[j]) + 1;
            styles_list[i + j] = (char *)calloc(sizeof(char), len);
            sprintf(styles_list[i+j], "%s.%s=%s",style->Name,setting_label[j], setting_value[j]);
        }
    }
    ass_set_style_overrides(library,styles_list);
    ass_process_force_style(track);
}

long long determine_total_duration(ASS_Track * track)
{
    assert(track != NULL);
    long long max = 0;
    int i = 0;

    for (i = 0; i < track->n_events; i++) {
        ASS_Event event = track->events[i];

        if (max < event.Start + event.Duration)
            max = event.Start + event.Duration;
    }
    return max;
}

int main(int argc, char **argv)
{
    ASS_Library *library = NULL;
    ASS_Renderer *renderer = NULL;
    ASS_Track *subtitles = NULL;
    ASS_Image *images = NULL;
    subpicture *current_subpicture = NULL;
    settings st;
    char *xml_path = NULL;
    FILE *xml_out = NULL;
    int detect_change = 0;
    int id = 0;

    st = process_arguments(argv, argc);

    library = init_ass_library();
    renderer = init_ass_renderer(library, st);

    subtitles = ass_read_file(library, st.subtitle_path, "utf-8");
    if (subtitles == NULL){
        printf("Couldn't load the subtitles : %s\n",st.subtitle_path);
        return EXIT_FAILURE;
    }

    if(st.dont_override_styles == 0)
        override_ass_styles(library,subtitles);

    xml_path =
        (char *) calloc(strlen(st.output_directory) + strlen(st.name) + 1 +
                        4 + 1, sizeof(char));
    sprintf(xml_path, "%s/%s.xml", st.output_directory, st.name);

    if (st.overwrite_files == 0 && check_file_exists(xml_path) == 0){
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

    long long pos = 0;          /*Current time postition */
    long long pos_max = determine_total_duration(subtitles);

    for (pos = 0; pos <= pos_max; pos += 25) {
        images = ass_render_frame(renderer, subtitles, pos, &detect_change);

        if (detect_change != 0) {
            if (current_subpicture != NULL) {
                char *image_path = NULL;
                char *image_filename = NULL;

                image_path =
                    (char *) calloc(strlen(st.output_directory) +
                                    strlen(st.name) + 10, sizeof(char));
                image_filename =
                    (char *) calloc(strlen(st.name) + 10, sizeof(char));

                sprintf(image_filename, "%s_%i.png", st.name,
                        current_subpicture->id);
                sprintf(image_path, "%s/%s", st.output_directory,
                        image_filename);

                if (current_subpicture->start < pos)
                    current_subpicture->end = pos - 1;
                else
                    current_subpicture->end = pos;


                if (st.overwrite_files == 0 && check_file_exists(image_path) == 0){
                    printf("Can't create %s. It already exists. Try option '-f' to overwrite existing files.\n",image_path);
                    return EXIT_FAILURE;
                }

                FILE *out = fopen(image_path, "wb");

                if (out == NULL){
                    printf("Couldn't save subpicture %i to %s\n",current_subpicture->id, image_path);
                    return EXIT_FAILURE;
                }

                subpicture_save(current_subpicture, out);
                subpicture_append_info_to_xml(current_subpicture,
                                              image_filename, xml_out);
                subpicture_free(current_subpicture);
                printf("Subpicture %i was saved.\n", id);

                free(image_path);
                free(image_filename);
                fclose(out);

                current_subpicture = NULL;
            }

            if (images != NULL) {
                current_subpicture = subpicture_new();
                subpicture_init(current_subpicture, ++id, st.subpicture_width,
                                st.subpicture_height, pos);
                subpicture_draw_subtitles(current_subpicture, images);
                subpicture_remap_colors(current_subpicture);

                if (st.opaque_subtitle_background != 0){
                    subpicture_make_subtitles_background_opaque(current_subpicture,images);
                }
            }
        }
    }

    fprintf(xml_out, "</stream></subpictures>");

    fclose(xml_out);
    ass_free_track(subtitles);
    ass_renderer_done(renderer);
    ass_library_done(library);

    return EXIT_SUCCESS;
}

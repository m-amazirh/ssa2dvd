#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <ass/ass.h>
#include "subpictures.h"

#define PROGRAM_NAME "ssa2dvd"

typedef struct _settings
{
    int subpicture_width;
    int subpicture_height;
    int top_margin;
    int bottom_margin;
    int right_margin;
    int left_margin;
    char *subtitle_path;
    char *output_directory;
    char *name;
} settings;

ASS_Library *init_ass_library();
ASS_Renderer *init_ass_renderer(ASS_Library * library, settings s);
long long determine_total_duration(ASS_Track * subtitles);
void usage(char *program_name);
settings process_arguments(char **argv, char argc);

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

    settings s = { -1,
        -1,
        20,
        20,
        60,
        60,
        NULL,
        NULL,
        "dvd_sub"
    };

    struct option longopts[] = {
        { "subtitles", required_argument, NULL, 's' },
        { "video-width", required_argument, NULL, 'w' },
        { "video-height", required_argument, NULL, 'h' },
        { "right-margin", required_argument, NULL, 'r' },
        { "left-margin", required_argument, NULL, 'l' },
        { "output-directory", required_argument, NULL, 'o' },
        { NULL, 0, NULL, 0 }
    };


    while ((c = getopt_long(argc, argv, "w:h:r:l:s:o:",longopts,&option_index)) != -1){

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

            case '?':
            default :
                usage(PROGRAM_NAME);
                exit(1);
        }

    }

    if (s.subpicture_width == -1)  { printf("Option -w is missing.\n"); usage(PROGRAM_NAME); exit(1); }
    if (s.subpicture_height == -1) { printf("Option -h is missing.\n"); usage(PROGRAM_NAME); exit(1); }
    if (s.subtitle_path == NULL)   { printf("Option -s is missing.\n"); usage(PROGRAM_NAME); exit(1); }
    if (s.output_directory == NULL)  { printf("Option -o is missing.\n"); usage(PROGRAM_NAME); exit(1); }

    return s;
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
    ass_set_margins(renderer, s.top_margin, s.bottom_margin, s.left_margin,
                    s.right_margin);

    ass_set_fonts(renderer, NULL, "Arial", 1, NULL, 1);

    return renderer;
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

void printf_and_quit(const char *msg, ...)
{
    va_list argptr;

    va_start(argptr, msg);
    vprintf(msg, argptr);
    va_end(argptr);
    exit(1);
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
    if (subtitles == NULL)
        printf_and_quit("Couldn't load the subtitles : %s\n",
                        st.subtitle_path);

    xml_path =
        (char *) calloc(strlen(st.output_directory) + strlen(st.name) + 1 +
                        4 + 1, sizeof(char));
    sprintf(xml_path, "%s/%s.xml", st.output_directory, st.name);

    xml_out = fopen(xml_path, "wb");
    if (xml_out == NULL)
        printf_and_quit("Couldn't create xml file : %s\n", xml_path);

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


                FILE *out = fopen(image_path, "wb");

                if (out == NULL)
                    printf_and_quit("Couldn't save subpicture %i to %s\n",
                                    current_subpicture->id, image_path);

                subpicture_remap_colors(current_subpicture);
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
            }
        }
    }

    fprintf(xml_out, "</stream></subpictures>");

    fclose(xml_out);
    ass_free_track(subtitles);
    ass_renderer_done(renderer);
    ass_library_done(library);

    return 0;
}

#include "libass_utils.h"

LibassConfiguration libass_configure(Configuration conf)
{
    LibassConfiguration libass_conf;
    
    ASS_Library *library = ass_library_init();

    if (library == NULL) {
        printf("Couldn't initialize ass library ...\n");
        exit(EXIT_FAILURE);
    }
    
    libass_conf.library = library;
    
    ASS_Renderer *renderer = ass_renderer_init(library);

    if (renderer == NULL) {
        printf("Couldn't initialize ass library ...\n");
        exit(EXIT_FAILURE);
    }

    ass_set_frame_size(renderer, conf.width, conf.height);

    if (conf.display_aspect != -1.0)
        ass_set_aspect_ratio(renderer, conf.display_aspect, 
                        ((double)conf.width) / ((double)conf.height));

    ass_set_margins(renderer, 20, 20, 60, 60);

    ass_set_fonts(renderer, NULL, "Arial", 1, NULL, 1);
    ass_set_font_scale(renderer, conf.font_scale);
    
    libass_conf.renderer = renderer;
    
    return libass_conf;
    
}

void libass_override_styles(LibassConfiguration *libass_conf,ASS_Track *track, Palette *palette)
{
    assert(track != NULL);
    int styles_count = track->n_styles;
    int i = 0, j = 0;
    char **styles_list = NULL;
    char *setting_value[] = { color2hex(&(palette->foreground)),
                              color2hex(&(palette->foreground)), 
                              color2hex(&(palette->outline)),
                               "1", "4", "0"};
    char *setting_label[] = { "PrimaryColour", 
                              "SecondaryColour", 
                              "OutlineColour", 
                              "BorderStyle", "Outline", "Shadow" };

    printf("Max styles allocated : %i\n",track->max_styles);
    printf("Number styles used : %i\n", track->n_styles);

    styles_list = (char **) calloc(sizeof(char *) * 6, styles_count);

    for (i=0; i< styles_count; i++){
        ASS_Style *style = &(track->styles[i]);
        int len;

        for (j=0;j<6;j++){
            len = strlen(style->Name) + 1 + strlen(setting_label[j]) 
                  + 1 + strlen(setting_value[j]) + 1;

            styles_list[i + j] = (char *)calloc(sizeof(char), len);
            
            sprintf(styles_list[i+j], "%s.%s=%s",style->Name,
                                                 setting_label[j], 
                                                 setting_value[j]);
        }
    }
    
    ass_set_style_overrides(libass_conf->library,styles_list);
    ass_process_force_style(track);
}

ASS_Track *libass_load_subtitles(LibassConfiguration *libass_conf,
                                    char *subtitles_path)
{
    ASS_Track *subtitles = NULL;
    subtitles = ass_read_file(libass_conf->library, subtitles_path, "utf-8");
    if (subtitles == NULL){
        printf("Couldn't load the subtitles : %s\n",subtitles_path);
        exit(EXIT_FAILURE);
    }
}

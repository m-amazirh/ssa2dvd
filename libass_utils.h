#ifndef LIBASS_UTILS_H
#define LIBASS_UTILS_H

#include <ass/ass.h>

#include "utils.h"
#include "subpictures.h"
#include "configuration.h"

typedef struct _LibassConfiguration 
{
    ASS_Library *library;
    ASS_Renderer *renderer;
} LibassConfiguration;




LibassConfiguration libass_configure(Configuration conf);

ASS_Track *libass_load_subtitles(LibassConfiguration *libass_conf,
                                    char *subtitles_path);
                                    
void libass_override_styles(LibassConfiguration *libass_conf,
                            ASS_Track *track, Palette *palette);

#endif

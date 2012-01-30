#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define PROGRAM_NAME "ssa2dvd"

typedef struct _Configuration
{
    char *video_standard;
    int width;
    int height;
    double display_aspect;
    double font_scale;
    
    char *subtitles_path;
    char *output_directory;
    char *prefix;
    int overwrite_files; 
    
} Configuration;

void usage(char *program_name);
Configuration process_arguments(char **argv, char argc);

#endif

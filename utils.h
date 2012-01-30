#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "subpictures.h"

char *long2time(long long l);
char *color2hex(Color *c);
int check_file_exists(const char *path);
long long determine_total_duration(ASS_Track * subtitles);

#endif

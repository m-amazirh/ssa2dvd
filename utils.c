#include "utils.h"

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

char *color2hex(Color *c)
{
    char *hex =  (char *)calloc(2 + 8, sizeof(char));
    sprintf(hex, "&H00%02X%02X%02X", c->red,  c->green, c->blue);
    return hex;
}

int check_file_exists(const char *path)
{
    return access(path,F_OK);
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

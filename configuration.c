#include "configuration.h"

void usage(char *program_name)
{
    printf("\nUsage : \n");
    printf
        ("\t%s -w width -h height -o output_directory -s subtitle_file [-r right_margin] [-l left_margin] \n",
         program_name);
}

Configuration process_arguments(char **argv, char argc)
{
    int c,arg_len,option_index;

    Configuration conf;
    
    conf.video_standard = NULL;
    conf.width = -1;
    conf.height = -1;
    conf.display_aspect = -1.0;
    conf.font_scale = 1.0;
    
    conf.subtitles_path = NULL;
    conf.output_directory = NULL;
    conf.prefix = NULL;
    conf.overwrite_files = 0;

    struct option longopts[] = {
        { "subtitles", required_argument, NULL, 's' },
        { "video-standard", required_argument, NULL, 'S' },
        { "video-width", required_argument, NULL, 'w' },
        { "video-height", required_argument, NULL, 'h' },
        { "display-aspect", required_argument, NULL, 'a'},
        { "output-directory", required_argument, NULL, 'o' },
        { "overwrite-files", no_argument, NULL, 'f' },
        { "font-scale", required_argument, NULL, 'x'},
        { "prefix", required_argument, NULL, 'p'},
        { NULL, 0, NULL, 0 }
    };


    while ((c = getopt_long(argc, argv, "w:h:s:o:a:x:S:p:f",longopts,&option_index)) != -1){

        switch (c){
            case 'w' :
                conf.width = atoi(optarg);
                break;
            case 'h' :
                conf.height = atoi(optarg);
                break;
            case 'a' :
                conf.display_aspect = atof(optarg);
                break;
            case 's' :
                arg_len = strlen(optarg);
                conf.subtitles_path = (char *) calloc(arg_len + 1, sizeof(char));
                strcpy(conf.subtitles_path, optarg);
                break;
            case 'o' :
                arg_len = strlen(optarg);
                conf.output_directory = (char *) calloc(arg_len + 1, sizeof(char));
                strcpy(conf.output_directory, optarg);
                break;
            case 'f' :
                conf.overwrite_files = 1;
                break;
            case 'x' :
                conf.font_scale = atof(optarg);
                break;
            case 'S':
                arg_len = strlen(optarg);
                conf.video_standard = (char *) calloc(arg_len + 1, sizeof(char));
                strcpy(conf.video_standard, optarg);
                break;
            case 'p':
                arg_len = strlen(optarg);
                conf.prefix = (char *) calloc(arg_len + 1, sizeof(char));
                strcpy(conf.prefix, optarg);
                break;

            case '?':
            default :
                usage(PROGRAM_NAME);
                exit(EXIT_FAILURE);
        }

    }
    
    if(conf.video_standard == NULL){
        /* Do nothing*/
    }
    else if(strcmp(conf.video_standard, "ntsc") == 0) {
        conf.width = 720;
        conf.height = 480;
    }
    else if(strcmp(conf.video_standard, "pal") == 0) {
        conf.width = 720;
        conf.height = 576;
    }
    else {
        printf("Unknown video standard : %s . Try \"ntsc\" or \"pal\" instead.\n", conf.video_standard);
        usage(PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }
    
    if (conf.prefix == NULL){
        conf.prefix = (char *) calloc(strlen("ssa2dvd") + 1, sizeof(char));
        strcpy(conf.prefix, "ssa2dvd"); 
    }
    
    if (conf.width == -1)  { 
        printf("Option -w is missing.\n"); 
        usage(PROGRAM_NAME);
        exit(EXIT_FAILURE); 
    }
    
    if (conf.height == -1) { 
        printf("Option -h is missing.\n"); 
        usage(PROGRAM_NAME); 
        exit(EXIT_FAILURE); 
    }
    
    if (conf.subtitles_path == NULL) { 
        printf("Option -s is missing.\n"); 
        usage(PROGRAM_NAME); 
        exit(EXIT_FAILURE); 
    }
    
    if (conf.output_directory == NULL){
        printf("Option -o is missing.\n");
        usage(PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    return conf;
}


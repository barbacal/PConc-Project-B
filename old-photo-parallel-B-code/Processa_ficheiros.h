#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif
//#define _GNU_SOURCE  

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "image-lib.h"
#include <stdbool.h>


extern char* OLD_PHOTO_PAR_B;
extern char* CONTRAST_DIR;
extern char* SMOOTH_DIR;
extern char* TEXTURE_DIR;
extern char* SEPIA_DIR;
extern char* IMG_DIR;
extern char** files;

//extern struct thread_info /*thread_process*/;

int Texturing(int file);
int Contrasting(int file);
int Smoothing(int file);
int Sepiaing(int file);
int Check_existing_image(char* img_file);
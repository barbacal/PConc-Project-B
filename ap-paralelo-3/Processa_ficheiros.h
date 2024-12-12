#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "image-lib.h"
#include "gd.h"



extern char* WATER_DIR;
extern char* RESIZE_DIR;
extern char* THUMB_DIR;
extern char* IMG_DIR;
extern char** files;

//extern struct thread_info /*thread_process*/;

int Watermarking(int file);
int Resizing(int file);
int Thumbnailing(int file);
int Check_existing_image(char* img_file);
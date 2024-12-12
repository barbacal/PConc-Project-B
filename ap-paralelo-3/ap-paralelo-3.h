#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "image-lib.h"
#include "Processa_ficheiros.h"

/* The directories where output files will be placed */
char* RESIZE_DIR = "./Resize-dir/";
char* THUMB_DIR  = "./Thumbnail-dir/";
char* WATER_DIR  = "./Watermark-dir/";

/* The images' filename list*/
char* PNG_LIST =   "image-list.txt";

/* The directory where input files will be picked */
char* IMG_DIR = 0;

int n_threads = 1;                // Minimum threads (besides main)
char* png_file = ".png";          // type of image format (in this case PNG)
int n_img = 0;                    // number of images to process
char** files = 0;                 // array of images to process

// 2 files descriptors used to write read on the pipe on stage 1
int stg1_pipe_fd[2];
// 2 files descriptors used to write read on the pipe on stage 2
int stg2_pipe_fd[2];
// 2 files descriptors used to write read on the pipe on stage 3
int stg3_pipe_fd[2];
// 2 files descriptors used to write read on the notifier's pipe
int notifier_fd[2];

void* Check_Input_Args(int argc, char* argv[]);
void* Check_Dirs();
char** Read_Files_List();
void* Make_pipes();
void* Processa_threads();
void* FreeAlloc();
void* Processa_watermarks(void* args);
void* Processa_resizes(void* args);
void* Processa_thumbnails(void* args);
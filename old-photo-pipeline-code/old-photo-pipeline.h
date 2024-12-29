#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
//#include <stdbool.h>
//#include <string.h>
//#include "image-lib.h"
#include <ctype.h>
#include "Processa_ficheiros.h"
#include <bits/pthreadtypes.h>
//#include <linux/time.h> //CLOCK_MONOTONIC not available in my Ubuntu PC
#include <time.h>
//#include <sys/time.h>
//#include <errno.h>

/*Timing structures for to check processing performance*/
struct timespec start_time_total, end_time_total;
struct timespec start_time_ser, end_time_ser;
struct timespec* start_time_par;
struct timespec* end_time_par;
struct timespec** start_time_thrd;
struct timespec** end_time_par_thrd;

/* File with timings: total, serial, parallel per thread*/
char* timing_file = 0;

/* The image ordering option */
char* OPTION = 0;

/* The directories where output files will be placed */
char* OLD_PHOTO_PAR_B = "/old_photo_PAR_B/";
char* CONTRAST_DIR = "/old_photo_PAR_B/Contrast-dir/";
char* SMOOTH_DIR = "/old_photo_PAR_B/Smooth-dir/";
char* TEXTURE_DIR  = "/old_photo_PAR_B/Texture-dir/";
char* SEPIA_DIR  = "/old_photo_PAR_B/Sepia-dir/";

/* The images' filename list*/
char* IMG_LIST =   "image-list.txt";

/* The directory where input files will be picked */
char* IMG_DIR = 0;

//pthread_barrier_t bar; // barrier for threads; experimental for for self learning

int n_threads = 1;                // Default threads (besides main)
const char* jpg_file = ".jpg";          // type of image format (in this case JPG)
const char* jpeg_file = ".jpeg";        // type of image format (in this case JPEG)
const char* png_file = ".png";          // type of image format (in this case PNG)
char* image_format = 0;            // type of image format 
int n_img = 0;                    // number of images to process
char** files = 0;                 // array of images to process

// 2 files descriptors used to write read on the pipe on stage 1
int stg1_pipe_fd[2];

// 2 files descriptors used to write read on the pipe on stage 2
int stg2_pipe_fd[2];

// 2 files descriptors used to write read on the pipe on stage 3
int stg3_pipe_fd[2];

// 2 files descriptors used to write read on the pipe on stage 4
int stg4_pipe_fd[2];

// 2 files descriptors used to write read on the notifier's pipe
int notifier_fd[2];

void* Check_Input_Args(int argc, char* argv[]);
void* Check_Dirs();
char** Read_Files_List();
void* Make_pipes();
void* Processa_threads();
void* FreeAlloc();
void* Processa_contrast(void* arg);
void* Processa_smooth();
void* Processa_texture();
void* Processa_sepia(void* arg);
bool Check_for_Images();
_Bool Check_for_Extension(const char* filename, const char* ext);
void* OrderFiles();
int Compare_Size(const void* a, const void* b);
int Compare_Name(const void* a, const void* b);
void* FinishTiming();
void* StartTiming();
void* FinishTimingSerial();
void* GetParallelTimingPhotos(struct timespec* start, struct timespec* end, char* file);
void* GetParallelTiming(struct timespec* start, struct timespec* end, long int thr_id);
/*************************************************************************************************************************
 * Programação Concorrente                                                                                               *
 * MEEC 22/23                                                                                                            *
 *                                                                                                                       *    
 * Projecto - Parte B                                                                                                    *                            
 *                           ap-paralelo-3.c                                                                             *
 *                                                                                                                       *
 * Compilação: gcc -Wall -std=c11 -v -pedantic ap-paralelo-3.c Processa_ficheiros.c image-lib.c -g -o ap-paralelo-3 -lgd *
 *                                                                                                                       *
 ************************************************************************************************************************/

#include "ap-paralelo-3.h"

/****************************************************************************************************
 * main()                                                                                           *                                                                               
 *                                                                                                  *
 * Arguments: (PNG_DIR_NAME, #THREADS)                                                              *
 * Returns: 0 in case of sucess, -1 in case of failure                                 *
 * Side-Effects: creates watermarked, watermarked thumbnail, watermarked resized copies of images   *
 *                                                                                                  *
 * Description: implementation of the parallelized serial-basic version                             *
 *              This application only works for a fixed pre-defined set of PNG files                *
 *                                                                                                  * 
 ***************************************************************************************************/

int main(int argc, char* argv[]) {
    Check_Input_Args(argc, argv);
    Check_Dirs();
    files = Read_Files_List();
    //puts("files in main:"); for (int i = 0; i < n_img; i++) puts(files[i]); // Dbg purpose; to delete
    Make_pipes();
    Processa_threads();
    FreeAlloc();
    exit(0);
}


void* Processa_threads() {
    // Stages to process: watermarks, resizing and thumbnailing PNGs
        pthread_t stage1_threads[n_threads]; // Watermarking 
        pthread_t stage2_threads[n_threads]; // Resizing
        pthread_t stage3_threads[n_threads]; // Thumbnailing


    for (int i = 0; i < n_threads; i++) {
       // printf("Stage 1 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage1_threads[i], 0, Processa_watermarks, 0);
       // printf("Stage 2 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage2_threads[i], 0, Processa_resizes, 0);
      //  printf("Stage 3 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage3_threads[i], 0, Processa_thumbnails, 0);
    }

    int notification = 1; //1: pipe on; 0: pipe off
    int file_index = 0;
    int n_files = n_img;
    while (n_img != 0) {
        for (int i = 0; i < n_threads && file_index < n_files; i++) {
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 1 notifier
            write(stg1_pipe_fd[1], &file_index, sizeof(file_index));    //Start piping
            file_index++;
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 2 notifier
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 3 notifier
        }
        if (n_img == 0) notification = 0;
    }

    return (void*)0;
}

void* Check_Dirs() {
    if (!create_directory(IMG_DIR)){
		fprintf(stderr, "No such directory exists: '%s'. Exiting\n", IMG_DIR);
		exit(-1);
	}
    if (!n_threads) {
        puts("No threads given, assuming one thread then");
        n_threads = 1;
    }
    if (n_threads < 0) {
        n_threads*= -1;
        printf("Only a positive number of threads, threads will be %d\n", n_threads); 
    }
    printf("Image directory is: %s\n", IMG_DIR);         // Dbg purpose; to delete 
    if (create_directory(RESIZE_DIR) == 0) {
		fprintf(stderr, "Impossible to create '%s' directory. Exiting.\n", RESIZE_DIR);
		exit(-1);
	}
	if (create_directory(THUMB_DIR) == 0) {
		fprintf(stderr, "Impossible to create '%s' directory. Exiting.\n", THUMB_DIR);
		exit(-1);
	}
	if (create_directory(WATER_DIR) == 0) {
		fprintf(stderr, "Impossible to create '%s' directory. Exiting.\n", WATER_DIR);
		exit(-1);
	}
    return (void*)0;
}

void* Check_Input_Args(int argc, char* argv[]) {
    //printf("argv[0]: %s\n", argv[0]); // Dbg purpose; to delete
    //printf("argv[1]: %s\n", argv[1]); // Dbg purpose; to delete
    //printf("argv[2]: %s\n", argv[2]); // Dbg purpose; to delete
    (argc == 3) ? (void)(n_threads = atoi(argv[2]), IMG_DIR = argv[1]) : (fprintf(stderr, "No image directory or no number of threades given.\n"), puts("Usage example: 'ap-paralelo-3 Image-dir 4'. Exiting."), exit(-1));
    return (void*)0;
    if (!n_threads) n_threads = 1;
    if (n_threads < 0) n_threads*= -1;
   // printf("Number of threads will be: %d\n", n_threads); // Dbg purpose; to delete 
    //printf("Image directory is: %s\n", IMG_DIR);          // Dbg purpose; to delete 
}

char** Read_Files_List() {
    FILE* fp = 0;
    const int name_size = 100; // maximum size in chars of an image filename
    char img_name[name_size];
    char* img_file = 0;
    char* img_dir = malloc(name_size * sizeof(char));
  	sprintf(img_dir, "%s/%s", IMG_DIR, PNG_LIST);
    //printf("Reading '%s'.\n", img_dir);
    fp = fopen(PNG_LIST, "rb");
    if (!fp) {
        fprintf(stderr, "File %s can't be opened or does not exist.\n", PNG_LIST);
        exit(-1);
    }
    while (fgets(img_name, name_size, fp) != 0) {
        img_file = strtok((char*)img_name, "\n");
        if (!img_file) break;
        char* file_format = strstr(img_file, png_file);
        if (!strcmp(file_format, png_file)) n_img++;
    }
   // printf("n_img: %d\n", n_img); // Dbg purpose; to delete
    rewind(fp);
    char** files = (char**)calloc(n_img, sizeof(char*));
    if (!files) {
        puts("Cannot process images. Aborting.");
        exit(-1);
    }
    int i = 0;
    while (fgets(img_name, name_size, fp) != 0) {
        img_file = strtok((char*)img_name, "\n");
        if (!img_file) break;
        char* file_format = strstr(img_file, png_file);
        if (!strcmp(file_format, png_file)) {
            files[i] = (char*)malloc((strlen(img_name) + 1) * sizeof(char));
            if (!files[i]) {
                printf("Cannot process %s. Skipping this one.\n", files[i]);
                continue;
            } 
            strcpy(files[i], img_file);
            i++;
        }
    }
    fclose(fp);
    fp = 0;
    return files;
}

void* FreeAlloc() {
    free(files);
    return (void*)0;
}

void* Make_pipes() {
    // initialization of the stage 1's pipe
    if (pipe(stg1_pipe_fd)!=0){
        printf("error creating the pipe on stage 1");
        exit(-1);
    }

    // initialization of the stage 2's pipe
    if (pipe(stg2_pipe_fd)!=0){
        printf("error creating the pipe on stage 2");
        exit(-1);
    }

    // initialization of the stage 3's pipe
    if (pipe(stg3_pipe_fd)!=0){
        printf("error creating the pipe on stage 3");
        exit(-1);
    }

    // initialization of the notifier's pipe
    if (pipe(notifier_fd)!=0){
        printf("error creating the notifier's pipe");
        exit(-1);
    }
    return (void*)0;
}

void* Processa_watermarks(void* args) {
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    //printf("notification: %d\n", notification);
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) {
        read(stg1_pipe_fd[0], &next_file, sizeof(next_file));
        //printf("next_file: %d\n", next_file);
        Watermarking(next_file);
        write(stg2_pipe_fd[1], &next_file, sizeof(next_file));
    }
    pthread_exit(NULL);
}

void* Processa_resizes(void* args) {
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) { 
        read(stg2_pipe_fd[0], &next_file, sizeof(next_file));
        Resizing(next_file);
        write(stg3_pipe_fd[1], &next_file, sizeof(next_file));
    }
    pthread_exit(NULL);
}

void* Processa_thumbnails(void* args) {
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) { 
        read(stg3_pipe_fd[0], &next_file, sizeof(next_file));
        Thumbnailing(next_file);
        n_img--;
       // printf("n_img: %d\n", n_img); // Dbg purpose; to delete
    }
    pthread_exit(NULL);
}
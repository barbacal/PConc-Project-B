/*************************************************************************************************************************
 * Programação Concorrente                                                                                               *
 * LEEC 24/25                                                                                                            *
 *                                                                                                                       *    
 * Projecto - Parte B                                                                                                    *                            
 *                           ap-paralelo-3.c                                                                             *
 *                                                                                                                       *
 * Compilação: gcc -Wall -std=c11 -v -pedantic ap-paralelo-3.c Processa_ficheiros.c image-lib.c -g -o ap-paralelo-3 -lgd *
 *                                                                                                                       *
 ************************************************************************************************************************/

#include "old-photo-pipeline.h"

/****************************************************************************************************
 * main()                                                                                           *                                                                               
 *                                                                                                  *
 * Arguments: (PNG_DIR_NAME, #THREADS)                                                              *
 * Returns: 0 in case of sucess, -1 in case of failure                                              *
 * Side-Effects: creates watermarked, watermarked thumbnail, watermarked resized copies of images   *
 *                                                                                                  *
 * Description: implementation of the parallelized old-photoserial version                          *
 *  This application works for a non fixed pre-defined set of PNG, JPEG or JPG files                *
 *                                                                                                  * 
 ***************************************************************************************************/

int main(int argc, char* argv[]) {
    StartTiming();
    Check_Input_Args(argc, argv);
    Check_Dirs();
    files = Read_Files_List();
    /*puts("Before sorting:)"); // Dbg purpose; to delete
    for (size_t i = 0; i < n_img; i++)
    {
     printf("'%s'", files[i]);
    }
    puts("\n");*/
    OrderFiles();
    /*puts("After sorting:)"); // Dbg purpose; to delete
     for (size_t i = 0; i < n_img; i++)
    {
     printf("'%s'", files[i]);
    }
    puts("\n");*/
    Make_pipes();
    FinishTimingSerial();
    Processa_threads();
    FinishTiming();
    puts("Done.");
    exit(0);
}


void* Processa_threads() {
    start_time_par = (struct timespec*) malloc(n_threads * sizeof(struct timespec));
    end_time_par = (struct timespec*) malloc(n_threads * sizeof(struct timespec));
    // Stages to process: watermarks, Smoothing and Texturing PNGs
    pthread_t stage1_threads[n_threads]; // Contrasting 
    pthread_t stage2_threads[n_threads]; // Smoothing
    pthread_t stage3_threads[n_threads]; // Texturing
    pthread_t stage4_threads[n_threads]; // Sepiaing

    for (int i = 0; i < n_threads; i++) {
        // printf("Stage 1 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage1_threads[i], 0, Processa_contrast, 0);
        // printf("Stage 2 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage2_threads[i], 0, Processa_smooth, 0);
        // printf("Stage 3 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage3_threads[i], 0, Processa_texture, 0);
        // printf("Stage 4 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage4_threads[i], 0, Processa_sepia, 0);

    }

    int notification = 1; //1: pipe on; 0: pipe off ????
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
    if (create_directory(IMG_DIR) == 0){
		fprintf(stderr, "'%s' does not exist. Exiting.\n", IMG_DIR);
        rmdir(IMG_DIR);
        exit(-1);   
    }
    char* dir;
    int res;
    size_t img_dir_len = strlen(IMG_DIR);
    dir = (char*)malloc((img_dir_len + strlen(CONTRAST_DIR) + 1) * sizeof(char));
    sprintf(dir, "%s%s", IMG_DIR, CONTRAST_DIR);
     
    CONTRAST_DIR = dir;
    printf("Contrast directory is: %s\n", CONTRAST_DIR);
    if ((res = create_directory(CONTRAST_DIR)) == 0) {
		printf("'%s' created.\n", CONTRAST_DIR);
	} else if (res == 1) fprintf(stderr, "'%s' already exists.\n", CONTRAST_DIR);
    dir = (char*)malloc((img_dir_len + strlen(SMOOTH_DIR) + 1) * sizeof(char));
    sprintf(dir, "%s%s", IMG_DIR, SMOOTH_DIR);
     
    SMOOTH_DIR = dir;
    if ((res = create_directory(SMOOTH_DIR)) == 0) {
		printf("'%s' created.\n", SMOOTH_DIR);
	} if (res == 1) fprintf(stderr, "'%s' already exists.\n", SMOOTH_DIR);
    dir = (char*)malloc((img_dir_len + strlen(TEXTURE_DIR) + 1) * sizeof(char));
    sprintf(dir, "%s%s", IMG_DIR, TEXTURE_DIR);
     
    TEXTURE_DIR = dir;
	if ((res = create_directory(TEXTURE_DIR)) == 0) {
		printf("'%s' created.\n", TEXTURE_DIR);
	} if (res == 1) fprintf(stderr, "'%s' already exists.\n", TEXTURE_DIR);
    dir = (char*)malloc(img_dir_len + strlen((SEPIA_DIR) + 1) * sizeof(char));
    sprintf(dir, "%s%s", IMG_DIR, (SEPIA_DIR));
     
    SEPIA_DIR = dir;
	if ((res = create_directory(SEPIA_DIR)) == 0) {
		printf("'%s' created.\n", SEPIA_DIR);
	}if (res == 1) fprintf(stderr, "'%s' already exists.\n", SEPIA_DIR);
    return (void*)0;
}

void* Check_Input_Args(int argc, char* argv[]) {
    const char* help = "Wrong calling. Usage example with 4 threads: './old-photo-parallel-A ./dir-1 4 -size (or -name)'. Exiting.";
    if (argc < 4) {
        puts(help);
        exit(-1);
    } else {
        char *endptr;
        n_threads = strtol(argv[2], &endptr, 10); // Base 10
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid number for threads: %s\n", argv[2]);
            puts(help);
            exit(-1);
        }
    }   
    OPTION = argv[3];
    if (!n_threads) {
        fprintf(stderr, "Invalid positive number %s\n", argv[2]);
        puts(help);
        exit(-1);
    } else if ((strcmp(OPTION, "-size") != 0 ) && (strcmp(OPTION, "-name") != 0)) {
        fprintf(stderr, "Invalid image ordering option: %s\n", OPTION);
        puts(help);
        exit(-1);
    }
    IMG_DIR = argv[1];
    if (IMG_DIR[strlen(IMG_DIR) - 1] == '/') IMG_DIR[strlen(IMG_DIR) - 1] = '\0';
    printf("Number of threads will be: %d\n", n_threads); // Dbg purpose; to delete 
    printf("Image directory is: %s\n", IMG_DIR);          // Dbg purpose; to delete
    return (void*)0;
}

char** Read_Files_List() {
    const int name_size = 100; // maximum size in chars of an image filename
    files = (char**)malloc(name_size * sizeof(char*));
    bool isFileList = true;
    FILE* fp = 0;
    char* img_file = malloc(name_size * sizeof(char));
    char* img_name = malloc(name_size * sizeof(char));
	char* list = malloc(name_size * sizeof(char));
	//sprintf(out_file_name, "%s%s", TEXTURE_DIR, files[i]);
	sprintf(list, "%s/%s", IMG_DIR, IMG_LIST); 
    IMG_LIST = list;
    printf("Reading '%s'.\n", IMG_LIST);
    fp = fopen(IMG_LIST, "r");
    if (!fp) {
        fprintf(stderr, "File %s can't be opened or does not exist.\n", IMG_LIST);
        isFileList = false;
        puts("Checking for images in image folder.");
        if (!Check_for_Images()) {
            puts("No images found of formats .jpg, .jpeg or .png. Exiting.");
            exit(-1);
        }
    }
    if (isFileList) {
        Check_for_Images();
        while (fgets(img_name, name_size, fp) != 0) {
            img_file = strtok((char*)img_name, "\n");
            img_name[strlen(img_name) - 1] = '\0';
            if (!img_file) break;
            char* file_format = strstr(img_file, image_format);
            if (file_format != NULL && !strcmp(file_format, image_format)) n_img++;
        }
        rewind(fp);
        files = (char**)malloc(n_img * sizeof(char*));
        if (!files) {
            fclose(fp);
            puts("Cannot process images from list. Aborting.");
            fp = 0;
            exit(-1);
        }
        int i = 0;
        while (fgets(img_name, name_size, fp) != 0) { //perhaps 'fscanf' is a better option: fscanf(fp, "%s", img_name); 
            img_file = strtok((char*)img_name, "\n");
            img_name[strlen(img_name) - 1] = '\0';
            if (!img_file) continue;
            char* file_format = strstr(img_file, image_format);
            //printf("strstr: %s\n", file_format);  // Dbg purpose; to delete
            if (file_format != NULL && !strcmp(file_format, image_format)) {
                files[i] = (char*)malloc(name_size * sizeof(char));
                if (!files[i]) {
                    printf("Cannot process %s. Skipping this one.\n", img_name);
                    continue;
                } 
                strcpy(files[i], img_file);
                printf("Found image of '%s'.\n", files[i] );
                i++;
            }
        }
    } else {
        DIR* dir;
        dir = opendir(IMG_DIR);
        if (!dir) {
            fprintf(stderr, "Directory %s does not exist. Exiting.\n", IMG_DIR);
            exit(-1);
        }
        struct dirent* entry;
        struct stat st;
        int i = 0;
        while ((entry = readdir(dir)) != NULL) {
            char* img_path;
            img_path = (char*)malloc(name_size * sizeof(char));
            sprintf(img_path, "%s/%s", IMG_DIR, entry->d_name);
            if (!stat(img_path, &st)) {
                if (S_ISREG(st.st_mode)) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                    sprintf(img_name, "%s", entry->d_name);
                    img_name[sizeof(entry->d_name) - 1] = '\0';
                    if (Check_for_Extension(img_name, image_format)) {
                        files[i] = (char*)malloc(name_size * sizeof(char));
                        if (!files[i]) {
                            printf("Cannot process %s. Skipping this one.\n", files[i]);
                            free(img_path);
                            continue;
                        }
                        sprintf(files[i], "%s", img_name);
                        //strcpy(files[i], img_name);
                        printf("Found image of '%s'.\n", files[i] );
                        i++;
                        n_img++;
                    }
                }
            }
            free(img_path);
        }
        if(dir) closedir(dir);
    }
    if (fp) {
        fclose(fp);
        fp = 0;
    }
    return files;   
}

bool Check_for_Images() {
    bool res = false;
    int n_formats = 2;
    const char* file_format[3] = {png_file, jpg_file, jpeg_file}; //Assumes either PNG, JPG or JPEG or empty folder
    const int name_size = 100; // maximum size in chars of an image filename
    char img_name[name_size];
    DIR* dir;
 loop:  dir = opendir(IMG_DIR);
    if (!dir) {
        fprintf(stderr, "Directory %s does not exist. Exiting.\n", IMG_DIR);
        exit(-1);
    }
    struct dirent* entry;
    struct stat st;
    while ((entry = readdir(dir)) != NULL) {
        char* file_path;
        file_path = (char*)malloc((name_size) * sizeof(char));
        sprintf(file_path, "%s/%s", IMG_DIR, entry->d_name);
        if (!stat(file_path/*entry->d_name*/, &st)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
             if (S_ISREG(st.st_mode)) {
                sprintf(img_name, "%s", entry->d_name);
                if (Check_for_Extension(entry->d_name, file_format[n_formats])) {
                    res = true;
                    image_format = (char*)file_format[n_formats];
                    printf("Found image(s) of format: %s\n", image_format);
                    break;
                }
            }
        }
    }
    closedir(dir);
    if(!res && n_formats) {
        n_formats--;
        goto loop;
    }
    return res;
}


void* FreeAlloc() {
    free(files);
    return (void*)0;
}

void* Make_pipes() {
    // initialization of stage 1's pipe
    if (pipe(stg1_pipe_fd)!=0){
        printf("error creating the pipe on stage 1");
        exit(-1);
    }

    // initialization of stage 2's pipe
    if (pipe(stg2_pipe_fd)!=0){
        printf("error creating the pipe on stage 2");
        exit(-1);
    }

    // initialization of stage 3's pipe
    if (pipe(stg3_pipe_fd)!=0){
        printf("error creating the pipe on stage 3");
        exit(-1);
    }

    // initialization of stage 4's pipe
    if (pipe(stg4_pipe_fd)!=0){
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

void* Processa_contrast(){
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    //printf("notification: %d\n", notification);
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) {
        read(stg1_pipe_fd[0], &next_file, sizeof(next_file));
        //printf("next_file: %d\n", next_file);
        Contrasting(next_file);
        write(stg2_pipe_fd[1], &next_file, sizeof(next_file));
    }
    pthread_exit(NULL);
}

void* Processa_smooth(){
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) { 
        read(stg2_pipe_fd[0], &next_file, sizeof(next_file));
        Smoothing(next_file);
        write(stg3_pipe_fd[1], &next_file, sizeof(next_file));
    }
    pthread_exit(NULL);
}

void* Processa_texture(){
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) { 
        read(stg3_pipe_fd[0], &next_file, sizeof(next_file));
        Texturing(next_file);
        n_img--;
       // printf("n_img: %d\n", n_img); // Dbg purpose; to delete
    }
    pthread_exit(NULL);
}

void* Processa_sepia(){
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) { 
        read(stg4_pipe_fd[0], &next_file, sizeof(next_file));
        Sepiaing(next_file);
        n_img--;
       // printf("n_img: %d\n", n_img); // Dbg purpose; to delete
    }
    pthread_exit(NULL);
}

_Bool Check_for_Extension(const char* filename, const char* ext) {
    size_t filename_len = strlen(filename);
    size_t ext_len = strlen(ext);
    const char *file_ext = filename + filename_len - ext_len;
    while (*file_ext && *ext) {
        if (tolower(*file_ext++) != tolower(*ext++)) {
            return false;
        }
    }
    return true;
}

void* OrderFiles() {
    if (strcmp(OPTION, "-size") == 0) {
        qsort(files, n_img, sizeof(char*), Compare_Size);
    } else if (strcmp(OPTION, "-name") == 0) {
        qsort(files, n_img, sizeof(char*), Compare_Name);
    }
    return (void*)0;
}

int Compare_Size(const void* a, const void* b) {
    struct stat st_a, st_b;
    stat(*(char**)a, &st_a);
    stat(*(char**)b, &st_b);
    if (st_a.st_size > st_b.st_size) return 1;
    if (st_a.st_size < st_b.st_size) return -1;
    return 0;
}

int Compare_Name(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void* StartTiming() {
	clock_gettime(CLOCK_REALTIME, &start_time_total);
    clock_gettime(CLOCK_REALTIME, &start_time_ser);
    return (void*)0;
}

void* FinishTimingSerial() {
    clock_gettime(CLOCK_REALTIME, &end_time_ser); 
    struct timespec ser_time = diff_timespec(&end_time_ser, &start_time_ser);
    FILE *fp;
    char* timing = (char*)malloc(100 * sizeof(char));
    timing_file = (char*)malloc(100 * sizeof(char));
    sprintf(timing_file, "%s%s%s", IMG_DIR, OLD_PHOTO_PAR_B, "/timing_");
    timing_file = strcat(timing_file, "<");
    char* str_n_threads = (char*)malloc(3 * sizeof(char));
    sprintf(str_n_threads, "%d", n_threads);
    timing_file = strcat(timing_file, str_n_threads);
    sprintf(timing_file, "%s>%s.txt", timing_file, OPTION);
    fp = fopen(timing_file, "w");
    if (!fp) {
        fprintf(stderr, "File %s can't be created.\n", timing_file);
        return (void*) -1;
    }
    sprintf(timing, "\tser \t %10jd.%09ld\n", ser_time.tv_sec, ser_time.tv_nsec);
    if (fputs(timing, fp) == EOF) {
        fprintf(stderr, "Error on writing in file %s.\n", timing_file);
        fclose(fp);
        fp = 0;
        return (void*) -1;
    }
    fclose(fp);
    fp = 0;
    return (void*)0;
}

void* FinishTiming() {
    clock_gettime(CLOCK_REALTIME, &end_time_total);
    struct timespec total_time = diff_timespec(&end_time_total, &start_time_total);
    char* timing = (char*)malloc(100 * sizeof(char));
    FILE *fp;
    fp = fopen(timing_file, "a");
    if (!fp) {
        fprintf(stderr, "File %s can't be appended.\n", timing_file);
        return (void*) -1;
    }
    sprintf(timing, "\ttotal \t %10jd.%09ld\n", total_time.tv_sec, total_time.tv_nsec);
    if (fputs(timing, fp) == EOF) {
        fprintf(stderr, "Error on writing in file %s.\n", timing_file);
        fclose(fp);
        fp = 0;
        return (void*) -1;
    }
    fclose(fp);
    fp = 0;
    return (void*)0;
}

void* GetParallelTiming(struct timespec* start, struct timespec* end, long int thr_id) {
    struct timespec par_time = diff_timespec(end, start);
    char* timing = (char*)malloc(100 * sizeof(char));
    FILE *fp;
    fp = fopen(timing_file, "a");
    if (!fp) {
        fprintf(stderr, "File %s can't be appended.\n", timing_file);
        return (void*) -1;
    }
    sprintf(timing, "\tpar \t %10jd.%09ld\n", par_time.tv_sec, par_time.tv_nsec);
    //sprintf(timing, "\tpar (thr id: %ld)\t %10jd.%09ld\n", thr_id, par_time.tv_sec, par_time.tv_nsec);
    if (fputs(timing, fp) == EOF) {
        fprintf(stderr, "Error on writing in file %s.\n", timing_file);
        fclose(fp);
        fp = 0;
        return (void*) -1;
    }
    fclose(fp);
    fp = 0;
    return (void*)0;
}
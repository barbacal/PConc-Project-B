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
 * Arguments: (IMG_DIR_NAME, #THREADS, ORDERING_OPTION)                                             *                 *
 * Returns: 0 in case of sucess, -1 in case of failure                                              *
 * Side-Effects: creates contrasted, textured, smoothed and sepia copies of images                  *
 *                                                                                                  *
 * Description: implementation of the parallelized old-photoserial version                          *
 *  This application works for a non fixed pre-defined set of JPEG or JPG files                     *
 *                                                                                                  * 
 ***************************************************************************************************/

int main(int argc, char* argv[]) {
    StartTiming();
    Check_Input_Args(argc, argv);
    Check_Dirs();
    files = Read_Files_List();
    /*puts("\nBefore sorting: "); // Dbg purpose; to delete or uncomment
    for (int i = 0; i < n_img; i++)
    {
     printf("'%s'", files[i]);
    }
    puts("\n");*/
    OrderFiles();
    /*puts("After sorting: "); // Dbg purpose; to delete
     for (int i = 0; i < n_img; i++)
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
    start_time_thrd = (struct timespec**) malloc(n_threads * sizeof(struct timespec*));
    end_time_par_thrd = (struct timespec**) malloc( n_threads * sizeof(struct timespec*));
    for(int i = 0; i < n_threads; i++) {
        start_time_thrd[i] = (struct timespec*) malloc(4 * sizeof(struct timespec));
        end_time_par_thrd[i] = (struct timespec*) malloc(4 * sizeof(struct timespec));
    }
    // Stages to process: Contrasting, Smoothing, Texturing and Sepiaing JPEgs
    pthread_t stage1_threads[n_threads]; // Contrasting 
    pthread_t stage2_threads[n_threads]; // Smoothing
    pthread_t stage3_threads[n_threads]; // Texturing
    pthread_t stage4_threads[n_threads]; // Sepiaing

    for (int i = 0; i < n_threads; i++) {
        // printf("Stage 1 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage1_threads[i], 0, Processa_contrast, i);
        clock_gettime(CLOCK_REALTIME, &start_time_thrd[i][0]);
        // printf("Stage 2 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage2_threads[i], 0, Processa_smooth, 0);
        clock_gettime(CLOCK_REALTIME, &start_time_thrd[i][1]);
        // printf("Stage 3 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage3_threads[i], 0, Processa_texture, 0);
        clock_gettime(CLOCK_REALTIME, &start_time_thrd[i][2]);
        // printf("Stage 4 thread %d creation\n", i + 1); // Dbg purpose; to delete or uncomment
        pthread_create(&stage4_threads[i], 0, Processa_sepia, i);
        clock_gettime(CLOCK_REALTIME, &start_time_thrd[i][3]);
    }
    
    int notification = 1; //1: pipe on; 0: pipe off; double check
    int file_index = 0;
    int n_files = n_img;
    while (n_img != 0) {
        for (int i = 0; i < n_threads && file_index < n_files; i++) {
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 1 notifier
            write(stg1_pipe_fd[1], &file_index, sizeof(file_index));    //Start piping
            file_index++;
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 2 notifier
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 3 notifier
            write(notifier_fd[1], &notification, sizeof(notification)); //Stage 4 notifier

        }
        if (n_img == 0) notification = 0; //Stop piping
    }
    for(int j= 0; j < 4; j++) {
        for (int i = 0; i < n_threads; i++) {
            clock_gettime(CLOCK_REALTIME, &end_time_par_thrd[i][j]);
            GetParallelTiming(&start_time_thrd[i][j], &end_time_par_thrd[i][j], i + j);    
        }   
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
    const int name_size = 1000; // maximum size in chars of an image filename
    files = (char**)malloc(name_size * sizeof(char*));
    bool isFileList = true;
    FILE* fp = 0;
    char* img_file = malloc(name_size * sizeof(char));
    char* list = malloc(name_size * sizeof(char));
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
        while (fgets(img_file, name_size, fp) != 0) {
            if (strlen(img_file) > 0 && img_file[strlen(img_file) - 1] == '\n') img_file[strlen(img_file) - 1] = '\0';
            if (!img_file) continue;
            if (Check_for_Extension(img_file, image_format)){
                files[n_img] = (char*)malloc(name_size * sizeof(char));
                if (!files[n_img]) {
                    printf("Cannot process %s. Skipping this one.\n", files[n_img]);
                    continue;
                }
                sprintf(files[n_img++], "%s", img_file);
            }
        }
       if (!files) {
            fclose(fp);
            puts("Cannot process images from list. Aborting.");
            fp = 0;
            exit(-1);
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
        img_file = malloc(name_size * sizeof(char));
        while ((entry = readdir(dir)) != NULL) {
            char* img_path;
            img_path = (char*)malloc(name_size * sizeof(char));
            sprintf(img_path, "%s/%s", IMG_DIR, entry->d_name);
            if (!stat(img_path, &st)) {
                if (S_ISREG(st.st_mode)) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                    sprintf(img_file, "%s", entry->d_name);
                    img_file[sizeof(entry->d_name) - 1] = '\0';
                    if (Check_for_Extension(img_file, image_format)) {
                        files[i] = (char*)malloc(name_size * sizeof(char));
                        if (!files[i]) {
                            printf("Cannot process %s. Skipping this one.\n", files[i]);
                            free(img_path);
                            continue;
                        }
                        sprintf(files[i++], "%s", img_file);
                        n_img++;
                    }
                }
            }
            free(img_path);
        }
        free(img_file);
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
    const int name_size = 1000; // maximum size in chars of an image filename
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
    for (int i = 0 ; i < n_threads ; i++) free(files[i]);
    free(files);
    free(start_time_par);
    free(end_time_par);
    for(int i = 0; i < n_threads; i++) {
        free(start_time_thrd[i]);
        free(end_time_par_thrd[i]);
    }
    free(start_time_thrd);
    free(end_time_par_thrd);
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

void* Processa_contrast(void* arg){
    int thr_id = (int)arg;
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    //printf("notification: %d\n", notification);
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) {
        read(stg1_pipe_fd[0], &next_file, sizeof(next_file));
        clock_gettime(CLOCK_REALTIME, &start_time_par[thr_id]);
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
        write(stg4_pipe_fd[1], &next_file, sizeof(next_file));
    }
    pthread_exit(NULL);
}

void* Processa_sepia(void* arg){
    int thr_id = (int)arg;
    int notification;
    int next_file;
    read(notifier_fd[0], &notification, sizeof(notification));
    if (notification == 0) pthread_exit(NULL);
    while (notification == 1) { 
        read(stg4_pipe_fd[0], &next_file, sizeof(next_file));
        Sepiaing(next_file);
        clock_gettime(CLOCK_REALTIME, &end_time_par[thr_id]);
        GetParallelTimingPhotos(&start_time_par[thr_id], &end_time_par[thr_id], files[next_file]);    
        n_img--;
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
    timing_file = (char*)malloc(1000 * sizeof(char));
    sprintf(timing_file, "%s%s", IMG_DIR, "/timing_pipeline_");
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
    free(timing);
    free(timing_file);
    return (void*)0;
}

void* GetParallelTimingPhotos(struct timespec* start, struct timespec* end, char* file) {
    struct timespec par_time = diff_timespec(end, start);
    char* timing = (char*)malloc(100 * sizeof(char));
    FILE *fp;
    fp = fopen(timing_file, "a");
    if (!fp) {
        fprintf(stderr, "File %s can't be appended.\n", timing_file);
        return (void*) -1;
    }
    sprintf(timing, "\t'%s' \t %10jd.%09ld\n", file, par_time.tv_sec, par_time.tv_nsec);
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

void* GetParallelTiming(struct timespec* start, struct timespec* end, long int thr_id) {
    struct timespec par_time = diff_timespec(end, start);
    char* timing = (char*)malloc(100 * sizeof(char));
    FILE *fp;
    fp = fopen(timing_file, "a");
    if (!fp) {
        fprintf(stderr, "File %s can't be appended.\n", timing_file);
        return (void*) -1;
    }
    sprintf(timing, "\tpar%d \t %10jd.%09ld\n", thr_id, par_time.tv_sec, par_time.tv_nsec);
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
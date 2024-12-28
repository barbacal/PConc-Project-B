/****************************************************************************************************************************************************                                                                                                                          *
 * LEEC 24/25                                                                                                                                       *
 *                                                                                                                                                  *    
 * Projecto - Parte A - Programação Concorrente                                                                                                                               *                            
 *                           old-photo-parallel-A.c                                                                                                 *
 *                                                                                                                                                  *
 * Compilação: gcc -Wall -std=c11 -v old-photo-parallel-A.c Processa_ficheiros.c image-lib.c -g -o old-photo-parallel-A -lpthread -lgd-lrt -pthread *
 *                                                                                                                                                  *
 ***************************************************************************************************************************************************/

#include "old-photo-parallel-B.h"


/****************************************************************************************************
 * main()                                                                                           *                                                                               
 *                                                                                                  *
 * Arguments: (PNG_DIR_NAME, #THREADS, ORDERING_OPTION)                                             *
 * Returns: 0 in case of sucess, -1 in case of failure                                              *
 * Side-Effects: creates contrast, smooth, sepia and texture copies of images                       *
 *                                                                                                  *
 * Description: refactoring of old-photo-parallel-A  from parallelized old-photo-serial version     *
 *   This application works for a non fixed pre-defined set of JPEG or JPG files                    *
 ***************************************************************************************************/

int main(int argc, char* argv[]) {
    StartTiming();
    Check_Input_Args(argc, argv);
    Check_Dirs();
    files = Read_Files_List();
    puts("Before sorting:)"); // Dbg purpose; to delete or uncomment  to check
    for (int i = 0; i < n_img; i++)
    {
     printf("'%s'", files[i]);
    }
    puts("\n");
    OrderFiles();
    puts("After sorting:)"); // Dbg purpose; to delete or uncomment to check
     for (int i = 0; i < n_img; i++)
    {
     printf("'%s'", files[i]);
    }
    puts("\n");
    Make_pipe();
    FinishTimingSerial();
    Parallelize_Serial();
    FreeAlloc();
    FinishTiming();
    puts("Done.");
    exit(0);
}

void* Parallelize_Serial() {
    pthread_t thread_id[n_threads];
    pthread_t stats_thread;
    /*void* thread_ret;*/
    /*int ret_val;*/
    start_time_par = (struct timespec*) malloc(n_threads * sizeof(struct timespec));
    end_time_par = (struct timespec*) malloc(n_threads * sizeof(struct timespec));
    //pthread_create(&stats_thread, 0, Mostra_stats, 0);
    for (int i = 0; i < n_threads; i++) {
        clock_gettime(CLOCK_REALTIME, &start_time_par[i]);
        pthread_create(&thread_id[i], 0, Processa_threads, 0);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
    }
    for (int i = 0; i < n_threads; i++) {
        pthread_join(thread_id[i], NULL/*&thread_ret*/);
        if (!do_piping) {
            close(img_pipe_fd[0]); //Close read
            close(img_pipe_fd[1]); //Close write
        }
        long int thr_id = pthread_self();
        clock_gettime(CLOCK_REALTIME, &end_time_par[i]);
        GetParallelTiming(&start_time_par[i], &end_time_par[i], thr_id);    
    }
    //pthread_join(stats_thread, NULL/*&thread_ret*/);
    pthread_mutex_destroy(&stats_mux);
    free(start_time_par);
    free(end_time_par); //falta o resto
    //threads = 0;
    return (void*)0;    
}

void* Processa_threads() {
    bool piping = false;
pipe:
    pthread_mutex_lock(&stats_mux);
    if (stop_stats && !do_piping && !piping && !n_img) {
        pthread_mutex_unlock(&stats_mux);
        pthread_exit(NULL);
    } 
    pthread_mutex_unlock(&stats_mux);
    int file;
    pthread_mutex_lock(&stats_mux);
    read(img_pipe_fd[0], &file, sizeof(file));
    Processa_contrast(file);
    Processa_smooth(file);
    Processa_texture(file);
    Processa_sepia(file);
    n_img_processed++;
    n_img_to_process--;
    piping = !stop_stats && (n_img_processed < n_img);
    if (n_img_to_process == 0) {
        stop_stats = true;
        piping = false;
    }                                                                                                                                                                                        
    if (!stop_stats && piping) {
        pthread_mutex_unlock(&stats_mux);
        goto pipe;
    }
    pthread_mutex_unlock(&stats_mux);  
    pthread_exit(NULL);    
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
    //free(dir);
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
        fprintf(stderr, "File '%s' can't be opened or does not exist.\n", IMG_LIST);
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

void* FreeAlloc() {
    free(files);// falta o resto
    return (void*)0;
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
    sprintf(timing_file, "%s%s", IMG_DIR,"/timing_B_<");
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

void* Make_pipe() {
    // initialization of image streamline pipe
    if (pipe(img_pipe_fd)!=0){
        printf("error creating the pipe. Exiting.\n");
        exit(-1);
    }
    if (n_threads > n_img){
        printf("Less images than threads. Processing %d threads, one per image.\n", n_img);
        n_threads = n_img;
    }
    int file_index = 0;
    n_img_to_process = n_img;
    int counter = n_img;
    while (do_piping) {
        write(img_pipe_fd[1], &file_index, sizeof(file_index));    //Start piping
        file_index++;
        counter--;
        if (!counter) do_piping = false; //Stop piping
    }
    return (void*)0;
}

void* Processa_contrast(int next_file) {
    Contrasting(next_file);
    return (void*)0;
}

void* Processa_smooth(int next_file) {
    Smoothing(next_file);
    return (void*)0;
}

void* Processa_texture(int next_file) {
    Texturing(next_file);
    return (void*)0;
}

void* Processa_sepia(int next_file) {
    Sepiaing(next_file);
    return (void*)0;
}

void* Processa_stats() {
    //pthread_mutex_lock(&stats_mux);
    printf("Number of images processed: %d\n", n_img_processed);
    printf("Number of images to process: %d\n", n_img_to_process);
    //pthread_mutex_unlock(&stats_mux);
    return (void*)0;
}

void* Mostra_stats() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        puts("Error fetching stdin flags. No nonblocking stdin.");
    } else if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) puts(" Error unblocking stdin.");

    int stats_request;
    pthread_mutex_lock(&stats_mux);
    bool stop = stop_stats;
    bool stating = n_img_to_process && !stop;     // inner flag to close stats thread
    pthread_mutex_unlock(&stats_mux);
    while (!stop) {
        while (stating) {
            if (!stating) break;
            if ((stats_request = getc(stdin)) != EOF) {    
                if (stats_request != '\n') while (getc(stdin) != '\n');
                if (toupper(stats_request) == 'S') {
                    pthread_mutex_lock(&stats_mux);
                    Processa_stats();
                    pthread_mutex_unlock(&stats_mux);      
                }
            } else stop = true;
            pthread_mutex_lock(&stats_mux);
            stop = stop_stats;
            stating = n_img_to_process && !stop;     // inner flag to close stats thread
            pthread_mutex_unlock(&stats_mux);
            break;
        }
    }
    return (void*)0;
}
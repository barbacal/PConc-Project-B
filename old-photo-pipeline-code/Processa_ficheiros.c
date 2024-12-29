 #include "Processa_ficheiros.h"
 
const int name_size = 1000;

int Texturing(int file) {
	int res = 0;
	/* file name of the image to porcessed at this stage */
	char* img_from_dir = malloc(name_size * sizeof(char));
	sprintf(img_from_dir, "%s%s", SMOOTH_DIR, files[file]);
	/* file name of the image created and to be saved on disk */
	char* out_file_name = malloc(name_size * sizeof(char));
	sprintf(out_file_name, "%s%s", TEXTURE_DIR, files[file]);
	int file_exists = Check_existing_image(out_file_name);
	/* input images */
	if (file_exists) {
		free(img_from_dir);
		free(out_file_name);
		return 0;
	}
	gdImagePtr in_img,  texture_img;
	/* output image */
	gdImagePtr out_texture_img;
	bool texture_exists = true; 
    texture_img = read_png_file("./old-photo-pipeline-code/paper-texture.png");
	if(!texture_img) {
		puts("No Texture...");
		fprintf(stderr, "Impossible to read %s image. Skipping.\n", "paper-texture.png");
		texture_exists = false;
	} 
	if (texture_exists) { 
		printf("Texture '%s'\n", files[file]);;
		/* load of the input file */
    	in_img = read_jpeg_file(img_from_dir);
		if (!in_img)
			printf("Impossible to read %s image\n", img_from_dir);
			out_texture_img = texture_image(in_img, texture_img);
			gdImageDestroy(texture_img);

		if (!out_texture_img) //{
        	fprintf(stderr, "Impossible to create texture of %s image\n", files[file]);
    		/* save texture */
			if(!write_jpeg_file(out_texture_img, out_file_name)){
				fprintf(stderr, "Impossible to write %s image\n", out_file_name);
				res = -1;
			}
			gdImageDestroy(out_texture_img);
	} else {
		puts("Texture does not exist.");
		in_img = read_jpeg_file(img_from_dir);
		if (!in_img) //{
			printf("Impossible to read %s image\n", img_from_dir);
			if(!write_jpeg_file(in_img, out_file_name))		
				fprintf(stderr, "Impossible to write %s image\n", out_file_name);
			res = -1;
	}
	gdImageDestroy(in_img);
	free(out_file_name);
	free(img_from_dir);
	return res;//exit(0);
}

int Contrasting(int file) {
	int res = 0;
	/* file name of the image to porcessed at this stage */
	char* img_from_dir = malloc(name_size * sizeof(char));
	sprintf(img_from_dir, "%s/%s", IMG_DIR, files[file]);
	/* file name of the image created and to be saved on disk */
	char* out_file_name = malloc(name_size * sizeof(char));
	sprintf(out_file_name, "%s%s",CONTRAST_DIR, files[file]);
	int file_exists = Check_existing_image(out_file_name);
	if (file_exists) {
		free(img_from_dir);
		free(out_file_name);
		return 0;
	}
	printf("Contrast '%s'\n", files[file]);
	/* input images */	
	gdImagePtr in_img;
	/* output images */
	gdImagePtr out_contrast_img;
	/* load of the input file */
	in_img = read_jpeg_file(img_from_dir);
	if (!in_img){
		printf("Impossible to read %s image\n", img_from_dir);
		}
	out_contrast_img = contrast_image(in_img);
  	if (!out_contrast_img) fprintf(stderr, "Impossible to resize %s image\n", files[file]);
    else {
		/* save resized image*/
		if(!write_jpeg_file(out_contrast_img, out_file_name)) fprintf(stderr, "Impossible to write %s image\n", out_file_name);
			res = -1;
	}
	gdImageDestroy(in_img);
	gdImageDestroy(out_contrast_img);
	free(out_file_name);
	free(img_from_dir);
	return res;//exit(0);
}

int Smoothing(int file) {
	int res = 0;
	/* file name of the image to porcessed at this stage */
	char* img_from_dir = malloc(name_size * sizeof(char));
	sprintf(img_from_dir, "%s%s", CONTRAST_DIR, files[file]);
	/* file name of the image created and to be saved on disk */
	char* out_file_name = malloc(name_size * sizeof(char));
	sprintf(out_file_name, "%s%s", SMOOTH_DIR, files[file]);
	int file_exists = Check_existing_image(out_file_name);
	if (file_exists) {
		free(img_from_dir);
		free(out_file_name);
		return 0;
	}
	printf("Smooth '%s'\n", files[file]);
	/* input images */
	gdImagePtr in_img;
	/* output images */
	gdImagePtr out_smooth_img;
	in_img = read_jpeg_file(img_from_dir);
	if (!in_img){
		fprintf(stderr, "Impossible to read %s image\n", files[file]);
	}
	/* creation of thumbnail from image */
	out_smooth_img = smooth_image(in_img);
	if (!out_smooth_img) fprintf(stderr, "Impossible to creat thumbnail of %s image\n", files[file]);
    else {
		/* save thumbnail image */
		if(!write_jpeg_file(out_smooth_img, out_file_name)) fprintf(stderr, "Impossible to write %s image\n", out_file_name);
		res = -1;
	}
	gdImageDestroy(out_smooth_img);
	gdImageDestroy(in_img);
	free(out_file_name);
	free(img_from_dir);//	}
	return res;//exit(0);
}

int Sepiaing(int file) {
	int res = 0;
	/* file name of the image to porcessed at this stage */
	char* img_from_dir = malloc(name_size * sizeof(char));
	sprintf(img_from_dir, "%s%s", TEXTURE_DIR, files[file]);
	/* file name of the image created and to be saved on disk */
	char* out_file_name = malloc(name_size * sizeof(char));
	sprintf(out_file_name, "%s%s", SEPIA_DIR, files[file]);
	int file_exists = Check_existing_image(out_file_name);
	if (file_exists) {
		free(img_from_dir);
		free(out_file_name);
		return 0;
	}
	printf("Sepia '%s'\n", files[file]);
	/* input images */
	gdImagePtr in_img;
	/* output images */
	gdImagePtr out_sepia_img;
	in_img = read_jpeg_file(img_from_dir);
	if (!in_img){
		fprintf(stderr, "Impossible to read %s image\n", files[file]);
	}
	/* creation of thumbnail from image */
	out_sepia_img = sepia_image(in_img);
	if (!out_sepia_img) fprintf(stderr, "Impossible to creat thumbnail of %s image\n", files[file]);
    else {
		/* save thumbnail image */
		if(!write_jpeg_file(out_sepia_img, out_file_name)) fprintf(stderr, "Impossible to write %s image\n", out_file_name);
			res = -1;
	}
	gdImageDestroy(out_sepia_img);
	gdImageDestroy(in_img);
	free(out_file_name);
	free(img_from_dir);//	}
	return res;//exit(0);
}


int Check_existing_image(char* img_file) {
	int res = 0;
	if (access(img_file, F_OK) != -1) {
		printf("%s encontrado\n", img_file);
		res = 1;
	} else printf("%s nao encontrado\n", img_file);
	return res;
}
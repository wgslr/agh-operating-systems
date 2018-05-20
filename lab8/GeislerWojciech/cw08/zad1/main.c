// 2018-05
// Wojciech Geisler

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

const int MAX_PIX = 255;

int filter_size;
int image_h;
int image_w;
double **filter;
int **image_in;
int **image_out;

int get_idx(int origin, int offset, int size) ;

int calc_pixel_value(int d1, int d2) ;

void read_filter(const char *path) {
    FILE *fd = fopen(path, "r");
    fscanf(fd, "%d\n", &filter_size);

    fprintf(stderr, "Reading filter of size %d\n", filter_size);

    filter = calloc(filter_size, sizeof(double *));
    for(int i = 0; i < filter_size; ++i) {
        filter[i] = calloc(filter_size, sizeof(double));
        for(int j = 0; j < filter_size; ++j) {
            fscanf(fd, "%lf ", &filter[i][j]);
        }
    }
    fclose(fd);
}

void read_image(const char *path) {
    // TODO handle comment lines

    FILE *fd = fopen(path, "r");
    int maxval = 0;
    fscanf(fd, "P2 %d %d %d", &image_w, &image_h, &maxval);

    if(maxval != MAX_PIX) {
        fprintf(stderr, "Unsupported color palette, max pixel value should be %d, got: %d\n", MAX_PIX, maxval);
        exit(1);
    }

    fprintf(stderr, "Reading image of size %dx%d\n", image_w, image_h);

    image_in = calloc(image_h, sizeof(int *));
    image_out = calloc(image_h, sizeof(int *));
    for(int i = 0; i < image_h; ++i) {
        image_in[i] = calloc(image_w, sizeof(int));
        image_out[i] = calloc(image_w, sizeof(int));
        for(int j = 0; j < image_w; ++j) {
            fscanf(fd, "%d ", &image_in[i][j]);
        }
    }
    fclose(fd);
}

void write_output(const char *path) {
    FILE *fd = fopen(path, "w");
    fprintf(fd, "P2\n%d %d\n%d\n", image_w, image_h, MAX_PIX);
    for(int i = 0; i < image_h; ++i) {
        for(int j = 0; j < image_w; ++j) {
            fprintf(fd, "%d ", image_out[i][j]);
        }
    }
    fprintf(fd, "\n");
}

void process_image(void) {
    for(int i = 0; i < image_h; ++i) {
        for(int j = 0; j < image_w; ++j) {
            image_out[i][j] = calc_pixel_value(i, j);
        }
    }
}


int calc_pixel_value(int d1, int d2) {
    double sum = 0;
    for(int i = 0; i < filter_size; ++i) {
        for(int j = 0; j < filter_size; ++j) {
            sum += image_in[get_idx(d1, i, image_h)][get_idx(d2, j, image_w)] * filter[i][j];
        }
    }
    return (int) lround(sum);
}

// calculates
// ceil(x/2)
int div2ceil(int x) {
    return x % 2 == 0 ? x / 2 : x / 2 + 1;
}


int get_idx(int origin, int offset, int size) {
    int idx = origin - div2ceil(filter_size) + offset;
    if(idx < 0) idx = 0;
    else if(idx >= size) idx = size - 1;
    return idx;
}


int main(int argc, char *argv[]) {
    if(argc != 5) {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }

    const int threads = atoi(argv[1]);
    const char *inpath = argv[2];
    const char *filterpath = argv[3];
    const char *outpath = argv[4];

    read_image(inpath);
    read_filter(filterpath);

    process_image();

    write_output(outpath);


    return 0;
}

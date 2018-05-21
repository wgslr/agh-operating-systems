// 2018-05
// Wojciech Geisler

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

const int MAX_PIX = 255;

int filter_size;
int image_h;
int image_w;
double** filter;
int** image_in;
int** image_out;

int threads;
int image_size;

int get_idx(int origin, int offset, int size);

int calc_pixel_value(int d1, int d2);

// calculates
// ceil(x/2)
int divceil(int x, int divisor) ;

void print_typespec(struct timespec start, struct timespec end) ;

void read_filter(const char* path) {
    FILE* fd = fopen(path, "r");
    fscanf(fd, "%d\n", &filter_size);

    filter = calloc(filter_size, sizeof(double*));
    for(int i = 0; i < filter_size; ++i) {
        filter[i] = calloc(filter_size, sizeof(double));
        for(int j = 0; j < filter_size; ++j) {
            fscanf(fd, "%lf ", &filter[i][j]);
        }
    }
    fclose(fd);
}

void read_image(const char* path) {
    // TODO handle comment lines

    FILE* fd = fopen(path, "r");
    int maxval = 0;
    fscanf(fd, "P2 %d %d %d", &image_w, &image_h, &maxval);

    image_size = image_h * image_w;

    if(maxval != MAX_PIX) {
        fprintf(stderr, "Unsupported color palette, max pixel value should be %d, got: %d\n", MAX_PIX, maxval);
        exit(1);
    }

    image_in = calloc(image_h, sizeof(int*));
    image_out = calloc(image_h, sizeof(int*));
    for(int i = 0; i < image_h; ++i) {
        image_in[i] = calloc(image_w, sizeof(int));
        image_out[i] = calloc(image_w, sizeof(int));
        for(int j = 0; j < image_w; ++j) {
            fscanf(fd, "%d ", &image_in[i][j]);
        }
    }
    fclose(fd);
}

void write_output(const char* path) {
    FILE* fd = fopen(path, "w");
    fprintf(fd, "P2\n%d %d\n%d\n", image_w, image_h, MAX_PIX);
    for(int i = 0; i < image_h; ++i) {
        for(int j = 0; j < image_w; ++j) {
            fprintf(fd, "%d ", image_out[i][j]);
        }
    }
    fprintf(fd, "\n");
}


void* process_part(int* args) {
    const int begin = args[0];
    const int end = args[1];

    for(int i = begin; i < end; ++i) {
        for(int j = 0; j < image_w; ++j) {
            image_out[i][j] = calc_pixel_value(i, j);
        }
    }

    free(args);
    return (void*) 0;
}


void process_image(void) {
    const int part_len = divceil(image_h, threads);
    pthread_t tids[threads];
    pthread_attr_t* attr = calloc(1, sizeof(pthread_attr_t));

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    for(int i = 0; i < threads; ++i) {
        pthread_attr_init(attr);

        int* params = calloc(2, sizeof(int));
        params[0] = i * part_len;
        params[1] = params[0] + part_len <= image_h ? params[0] + part_len : image_h;

        pthread_create(tids + i, attr, (void* (*)(void*)) &process_part, params);

        pthread_attr_destroy(attr);
    }

    for(int i = 0; i < threads; ++i) {
        pthread_join(tids[i], NULL);
    }

    clock_gettime(CLOCK_REALTIME, &end_time);
    print_typespec(start_time, end_time);
}


void print_typespec(struct timespec start, struct timespec end) {
    unsigned long long ms = (end.tv_sec - start.tv_sec) * 1000000;
    ms += (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%d; %llu; %d; %d\n", threads, ms, image_size, filter_size * filter_size);
}


int calc_pixel_value(const int d1, const int d2) {
    double sum = 0;
    for(int i = 0; i < filter_size; ++i) {
        for(int j = 0; j < filter_size; ++j) {
            sum += image_in[get_idx(d1, i, image_h)][get_idx(d2, j, image_w)] * filter[i][j];
        }
    }
    return (int) lround(sum);
}


// calculates ceil(x/y)
int divceil(int x, int divisor) {
    return x % divisor == 0 ? x / divisor : x / divisor + 1;
}


int get_idx(int origin, int offset, int size) {
    int idx = origin - divceil(filter_size, 2) + offset;
    if(idx < 0) idx = 0;
    else if(idx >= size) idx = size - 1;
    return idx;
}


int main(int argc, char* argv[]) {
    if(argc != 5) {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }

    threads = atoi(argv[1]);
    const char* inpath = argv[2];
    const char* filterpath = argv[3];
    const char* outpath = argv[4];

    read_image(inpath);
    read_filter(filterpath);

    process_image();

    write_output(outpath);


    return 0;
}

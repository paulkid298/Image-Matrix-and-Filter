#include "global.h"

void grayscale(cv::Mat image, image_t *output, int width, int height);
float *gaussian_kernel(int size, float sigma);
void convolve(image_t *matrix_in, image_t *matrix_out, int height,
              int width, const float *kernel, int kernelSize);
void sobel_filter(image_t *, image_t *, float *, float *max_o,
                  int, int);
void non_max_suppression(const image_t *matrix_in, image_t *matrix_out, float *theta,
                         float max_in, float *max_out, int height, int width);
void threshold(image_t *matrix_in, image_t *matrix_out, int height, int width,
               float lowThreshold, float highThreshold, int max);
void hysteresis(image_t *matrix_in, image_t *matrix_out, int height, int width);
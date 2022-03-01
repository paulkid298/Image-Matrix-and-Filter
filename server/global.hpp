#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <math.h>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>

#include "image_filter_parallel.hpp"
#define PORT 8080

/** values for edge detection algorithm */
#define LOW_THRESHOLD 0.05
#define HIGH_THRESHOLD 0.09
#define GAUSSIAN_SIZE 5
#define SIGMA 1

typedef unsigned char image_t;
/**
extern int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh, ind_hyster,
    ind_resize, ind_p_out;
extern int parallel;

extern int height, width;*/

static int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh,
    ind_hyster, ind_resize, ind_p_out;
static int parallel;
static int height, width;

#define NUM_THETA 3
#define NUM_IMAGES 9
#define NUM_OUTPUTS 3
#define SLEEP_TIME 100

static image_t *images[NUM_IMAGES];
static float *theta[NUM_IMAGES];
static cv::Mat out_images[3];

#define DOWN_WIDTH 128
#define DOWN_HEIGHT 96
#define SIZE_OUT DOWN_WIDTH *DOWN_HEIGHT

int ready(int, int);

#endif
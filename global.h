#ifndef GLOBAL_H
#define GLOBAL_H

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
#define SIZE_OUT 96 * 96

typedef unsigned char image_t;
int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh, ind_hyster, ind_p_out;
int parallel;

int height, width;

#define NUM_IMAGES 8
image_t *images[NUM_IMAGES];

int ready(int ind_curr, int ind_parent)
{
    if ((ind_curr + 1) % NUM_IMAGES == ind_parent)
    {
        return 0;
    }
    return 1;
}
#endif
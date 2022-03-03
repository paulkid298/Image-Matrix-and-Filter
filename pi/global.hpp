#ifndef GLOBALPI_HPP
#define GLOBALPI_HPP

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#define NUM_IMAGES 4

typedef unsigned char image_t;
static int ind_cam, ind_gray, ind_client;
static cv::Mat images[NUM_IMAGES];
static image_t *gray_images[NUM_IMAGES];
static int height, width;

int ready(int, int);

#define PIPELINE 0
#define PARALLEL 0
#define SLEEP_TIME 100
#define SIZE_OUT 640 * 480

#define PORT 8080
#define MAXBYTES 1024
#define SA struct sockaddr

#endif

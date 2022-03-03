#include "image_filter_sequential.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <unistd.h>
#include <iostream>
#include <math.h>
using namespace std;
#define DURATION 10000
#define char_end 255
#define SOBEL_SIZE 3
#define WEAK 25
#define STRONG 255

const float kx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};
const float ky[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}};

// algorithm taken from
// https://stackoverflow.com/questions/17615963/standard-rgb-to-grayscale-conversion
void grayscale(cv::Mat image, image_t *output, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        for (int k = 0; k < width; k++)
        {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, k);

            float red = ((float)pixel[0]);
            float green = ((float)pixel[1]);
            float blue = ((float)pixel[2]);

            float out = red * 0.299 + green * 0.587 + blue * 0.114;
            output[i * width + k] = out;
            //*output++ = out;
            // cout << out << "\n";
        }
    }
}

float *gaussian_kernel(int size, float sigma)
{
    float x[size][size];
    float y[size][size];
    float *output = (float *)malloc(sizeof(float) * size * size);

    int tempSize = size / 2;

    for (int i = -tempSize; i < tempSize + 1; i++)
    {
        for (int k = -tempSize; k < tempSize + 1; k++)
        {
            x[i + tempSize][k + tempSize] = i;
            y[i + tempSize][k + tempSize] = k;
        }
    }

    float normal = 1 / (2.0 * M_PI * pow(sigma, 2));
    cout << normal << '\n';

    for (int i = 0; i < size; i++)
    {
        for (int k = 0; k < size; k++)
        {
            output[i * size + k] = exp(-((pow(x[i][k], 2) +
                                          pow(y[i][k], 2)) /
                                         (2.0 * pow(sigma, 2)))) *
                                   normal;
        }
    }
    return output;
}

void convolve(image_t *matrix_in, image_t *matrix_out, int height,
              int width, const float *kernel, int kernelSize)
{
    int orig_kernel_size = kernelSize;
    kernelSize = kernelSize / 2;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float output = 0;
            for (int k = -kernelSize; k < kernelSize + 1; k++)
            {
                for (int m = -kernelSize; m < kernelSize + 1; m++)
                {
                    if (i + k >= 0 && i + k < height &&
                        j + m >= 0 && j + m < width)
                    {
                        float mat_value = matrix_in[(i + k) * width + j + m];
                        float kernel_value = kernel[(k + kernelSize) * orig_kernel_size +
                                                    m + kernelSize];
                        output += kernel_value * mat_value;
                    }
                }
            }
            matrix_out[i * width + j] = output;
        }
    }
}

float sobel_filter(image_t *matrix_in, image_t *magnitude, float *theta,
                   float *max_o, int height, int width)
{
    // float temp_mag[height][width];

    int kernelSize = SOBEL_SIZE / 2;
    float max = 0;

    // perform double convolution
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float Tx = 0;
            float Ty = 0;
            for (int k = -kernelSize; k < kernelSize + 1; k++)
            {
                for (int m = -kernelSize; m < kernelSize + 1; m++)
                {
                    if (i + k >= 0 && i + k < height &&
                        j + m >= 0 && j + m < width)
                    {
                        float mat_value = matrix_in[(i + k) * width + j + m];
                        float kernel_value_x = kx[k + kernelSize][m + kernelSize];
                        float kernel_value_y = ky[k + kernelSize][m + kernelSize];
                        Tx += mat_value * kernel_value_x;
                        Ty += mat_value * kernel_value_y;
                    }
                }
            }
            float hypotenuse = hypot(Tx, Ty);
            // temp_mag[i][j] = hypotenuse;
            magnitude[i * width + j] = hypotenuse;
            if (hypotenuse > max)
            {
                max = hypotenuse;
            }
            theta[i * width + j] = atan2(Ty, Tx);
        }
    }

    *max_o = max;

    /**
    // use computed maximum to normalize pixels
    for (int i = 0; i < height; i++)
    {
        for (int k = 0; k < width; k++)
        {
            magnitude[i * width + k] = (temp_mag[i][k] / max) * 255;
        }
    }
    */
}

void non_max_suppression(const image_t *matrix_in, image_t *matrix_out, float *theta,
                         float max_in, float *max_out, int height, int width)
{
    float max = 0;
    for (int i = 0; i < height; i++)
    {
        for (int k = 0; k < width; k++)
        {
            int q = 255;
            int r = 255;
            float angle = theta[i * width + k] * 180 / M_PI;
            if (angle < 0)
            {
                angle += 180;
            }
            // theta[i * width + k] = angle;
            if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180))
            {
                q = (k + 1 < width)
                        ? (((float)matrix_in[i * width + k + 1]) / max_in) * 255
                        : 255;
                r = (k - 1 >= 0)
                        ? (((float)matrix_in[i * width + k - 1]) / max_in) * 255
                        : 255;
            }
            else if (angle >= 22.5 && angle < 67.5)
            {
                q = (i + 1 < height && k - 1 >= 0)
                        ? (((float)matrix_in[(i + 1) * width + k - 1]) / max_in) * 255
                        : 255;
                r = (i - 1 >= 0 && k + 1 < width)
                        ? (((float)matrix_in[(i - 1) * width + k + 1]) / max_in) * 255
                        : 255;
            }
            else if (angle >= 67.5 && angle < 112.5)
            {
                q = (i + 1 < height)
                        ? (((float)matrix_in[(i + 1) * width + k]) / max_in) * 255
                        : 255;
                r = (i - 1 >= 0)
                        ? (((float)matrix_in[(i - 1) * width + k]) / max_in) * 255
                        : 255;
            }
            else if (angle >= 112.5 && angle < 157.5)
            {
                q = (i - 1 >= 0 && k - 1 >= 0)
                        ? (((float)matrix_in[(i - 1) * width + k - 1]) / max_in * 255)
                        : 255;
                r = (i + 1 < height && k + 1 < width)
                        ? (((float)matrix_in[(i + 1) * width + k + 1]) / max_in) * 255
                        : 255;
            }

            image_t image_val = (((float)matrix_in[i * width + k]) / max_in) * 255;
            if (image_val >= q && image_val >= r)
            {
                matrix_out[i * width + k] = image_val;
            }
            else
            {
                matrix_out[i * width + k] = 0;
            }

            // find maximum
            if (matrix_out[i * width + k] > max)
            {
                max = (float)matrix_out[i * width + k];
            }
        }
    }
    *max_out = max;
}

// Double Threshold
void threshold(image_t *matrix_in, image_t *matrix_out, int height, int width,
               float lowThreshold, float highThreshold, int max)
{
    highThreshold = max * highThreshold;
    lowThreshold = highThreshold * lowThreshold;

    for (int i = 0; i < height; i++)
    {
        for (int k = 0; k < width; k++)
        {
            int matrix_ind = i * width + k;
            if (matrix_in[matrix_ind] >= highThreshold)
            {
                matrix_out[matrix_ind] = STRONG;
            }
            else if (matrix_in[matrix_ind] >= lowThreshold)
            {
                matrix_out[matrix_ind] = WEAK;
            }
            else
            {
                matrix_out[matrix_ind] = 0;
            }
        }
    }
}

// Edge Tracking by Hysteresis
void hysteresis(image_t *matrix_in, image_t *matrix_out, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        for (int k = 0; k < width; k++)
        {
            // determine if we should keep the week values
            if (matrix_in[i * width + k] == WEAK)
            {
                matrix_out[i * width + k] = 0;

                // check surroundings
                for (int a = -1; a <= 1; a++)
                {
                    for (int b = -1; b <= 1; b++)
                    {
                        // out of bounds check
                        if (i + a > 0 && i + a < height && k + b > 0 && k + b < width)
                        {
                            if (matrix_in[(i + a) * width + (k + b)] == STRONG)
                            {
                                matrix_out[i * width + k] = STRONG;
                                break;
                            }
                        }
                    }
                    // break if the image
                    if (matrix_out[i * width + k] == STRONG)
                    {
                        break;
                    }
                }
            }
            else
            {
                matrix_out[i * width + k] = matrix_in[i * width + k];
            }
        }
    }
}
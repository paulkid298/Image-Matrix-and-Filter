#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <unistd.h>
#include <time.h>
#include "webcam_test.hpp"
#include <iostream>
#include <math.h>
#include <vector>
#include "image_filter_parallel.hpp"
using namespace std;
#define DURATION 10000
#define char_end 255
#define SIGMA 1
#define GAUSSIAN_SIZE 5
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

pthread_mutex_t sobel_lock, suppression_lock;

class ParallelGrayscale : public ParallelLoopBody
{
public:
    ParallelGrayscale(Mat image, image_t *output, const int height,
                      const int width)
        : m_in(image), m_out(output), m_height(height), m_width(width)
    {
    }
    virtual void operator()(const Range &range) const CV_OVERRIDE
    {
        for (int r = range.start; r < range.end; r++)
        {
            int row = r / m_width;
            int col = r % m_width;
            cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);

            float red = ((float)pixel[0]);
            float green = ((float)pixel[1]);
            float blue = ((float)pixel[2]);

            float out = red * 0.299 + green * 0.587 + blue * 0.114;
            m_out[row * m_width + col] = out;
        }
    }
    ParallelGrayscale &operator=(const ParallelGrayscale &)
    {
        return *this;
    };

private:
    Mat m_in;
    image_t *m_out;
    int m_height;
    int m_width;
};

class ParallelConvolve : public ParallelLoopBody
{
public:
    ParallelConvolve(image_t *matrix_in, image_t *matrix_out, const int height,
                     const int width, const float *kernel, int kernelSize)
        : m_in(matrix_in), m_out(matrix_out), m_height(height), m_width(width),
          m_kernel(kernel), m_kernelSize(kernelSize)
    {
    }
    virtual void operator()(const Range &range) const CV_OVERRIDE
    {
        int orig_kernel_size = m_kernelSize;
        m_kernelSize = m_kernelSize / 2;
        for (int r = range.start; r < range.end; r++)
        {
            int row = r / m_width;
            int col = r % m_width;
            float output = 0; // sum of products
            for (int k = -m_kernelSize; k < m_kernelSize + 1; k++)
            {
                for (int m = -m_kernelSize; m < m_kernelSize + 1; m++)
                {
                    if (row + k >= 0 && row + k < m_height &&
                        col + m >= 0 && col + m < m_width)
                    {
                        float mat_value = m_in[(row + k) * m_width +
                                               col + m];
                        float kernel_value = m_kernel[(k + m_kernelSize) *
                                                          orig_kernel_size +
                                                      m + m_kernelSize];
                        output += kernel_value * mat_value;
                    }
                }
            }
            m_out[row * m_width + col] = output;
        }
    }
    ParallelConvolve &operator=(const ParallelConvolve &)
    {
        return *this;
    };

private:
    image_t *m_in;
    image_t *m_out;
    int m_height;
    int m_width;
    float *m_kernel;
    int m_kernelSize
};

// normalize magnitude afterwards
class ParallelSobel : public ParallelLoopBody
{
public:
    ParallelSobel(image_t *matrix_in, float *magnitude, float *theta,
                  int height, int width, float *tot_max)
        : m_in(matrix_in), m_mag(magnitude), m_theta(theta), m_height(height),
          m_width(width), m_max(tot_max)
    {
    }
    virtual void operator()(const Range &range) const CV_OVERRIDE
    {
        int kernelSize = SOBEL_SIZE / 2;
        float max = 0;
        for (int r = range.start; r < range.end; r++)
        {
            int row = r / m_width;
            int col = r % m_width;
            float Tx = 0;
            float Ty = 0;
            for (int k = -kernelSize; k < kernelSize + 1; k++)
            {
                for (int m = -kernelSize; m < kernelSize + 1; m++)
                {
                    if (row + k >= 0 && row + k < m_height &&
                        col + m >= 0 && col + m < m_width)
                    {
                        float mat_value = m_in[(i + k) * m_width + j + m];
                        float kernel_value_x = kx[k + kernelSize][m + kernelSize];
                        float kernel_value_y = ky[k + kernelSize][m + kernelSize];
                        Tx += mat_value * kernel_value_x;
                        Ty += mat_value * kernel_value_y;
                    }
                }
            }
            float hypotenuse = hypot(Tx, Ty);
            m_mag[i][j] = hypotenuse;
            if (hypotenuse > max)
            {
                max = hypotenuse;
            }
            m_theta[i * m_width + j] = atan2(Ty, Tx);
        }

        pthread_mutex_lock(&sobel_lock);
        if (*m_max < max)
        {
            *m_max = max;
        }
        pthread_mutex_unlock(&sobel_lock);
    }
    ParallelSobel &operator=(const ParallelSobel &)
    {
        return *this;
    };

private:
    image_t *m_in;
    float *m_mag;
    float *m_theta;
    int m_height;
    int m_width;
    float *m_max;
};

class ParallelSupression : public ParallelLoopBody
{
public:
    ParallelSupression(float *image_in, image_t *output, float *theta,
                       const int height, const int width, float max_in,
                       image_t *max_out)
        : m_in(image_in), m_out(output), m_theta(thetha), m_height(height),
          m_width(width), m_max_i(max_in), m_max_o(max_out)
    {
    }
    virtual void operator()(const Range &range) const CV_OVERRIDE
    {
        image_t max = 0;
        for (int r = range.start; r < range.end; r++)
        {
            int i = r / m_width;
            int k = r % m_width;
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
                q = (k + 1 < m_width)
                        ? (m_in[i * m_width + k + 1] / m_max_i) * 255
                        : 255;
                r = (k - 1 >= 0)
                        ? (m_in[i * m_width + k - 1] / m_max_i) * 255
                        : 255;
            }
            else if (angle >= 22.5 && angle < 67.5)
            {
                q = (i + 1 < m_height && k - 1 >= 0)
                        ? (m_in[(i + 1) * m_width + k - 1] / m_max_i) * 255
                        : 255;
                r = (i - 1 >= 0 && k + 1 < m_width)
                        ? (m_in[(i - 1) * m_width + k + 1] / m_max_i) * 255
                        : 255;
            }
            else if (angle >= 67.5 && angle < 112.5)
            {
                q = (i + 1 < m_height)
                        ? (m_in[(i + 1) * m_width + k] / m_max_i) * 255
                        : 255;
                r = (i - 1 >= 0)
                        ? (m_in[(i - 1) * m_width + k] / m_max_i) * 255
                        : 255;
            }
            else if (angle >= 112.5 && angle < 157.5)
            {
                q = (i - 1 >= 0 && k - 1 >= 0)
                        ? (m_in[(i - 1) * m_width + k - 1] / m_max_i) * 255
                        : 255;
                r = (i + 1 < m_height && k + 1 < m_width)
                        ? (m_in[(i + 1) * m_width + k + 1] / m_max_i) * 255
                        : 255;
            }

            image_t image_val = (m_in[i * m_width + k] / m_max_i) * 255;
            if (image_val >= q && image_val >= r)
            {
                m_out[i * m_width + k] = image_val;
            }
            else
            {
                m_out[i * m_width + k] = 0;
            }

            // find maximum
            if (m_out[i * m_width + k] > max)
            {
                max = m_out[i * m_width + k];
            }
        }

        pthread_mutex_lock(&suppression_lock);
        if (*m_max_o < max)
        {
            *m_max_o = max;
        }
        pthread_mutex_unlock(&suppression_lock);
    }
    ParallelSupression &operator=(const ParallelSupression &)
    {
        return *this;
    };

private:
    float *m_in;
    image_t *m_out;
    float *m_theta;
    int m_height;
    int m_width;
    float m_max_i;
    image_t *m_max_o;
};

class ParallelThreshold : public ParallelLoopBody
{
public:
    ParallelThreshold(image_t *matrix_in, image_t *matrix_out, int height,
                      int width, float lowThreshold, float highThreshold, int max)
        : m_in(matrix_in), m_out(matrix_out), m_height(height),
          m_width(width), m_low(lowThreshold), m_high(highThreshold),
          m_max(max)
    {
    }
    virtual void operator()(const Range &range) const CV_OVERRIDE
    {
        m_high = m_max * m_high;
        m_low = m_high * m_low;
        int kernelSize = SOBEL_SIZE / 2;
        float max = 0;
        for (int r = range.start; r < range.end; r++)
        {
            int i = r / m_width;
            int k = r % m_width;
            int matrix_ind = i * m_width + k;
            if (m_in[matrix_ind] >= m_high)
            {
                m_out[matrix_ind] = STRONG;
            }
            else if (m_in[matrix_ind] >= m_low)
            {
                m_out[matrix_ind] = WEAK;
            }
            else
            {
                m_out[matrix_ind] = 0;
            }
        }
    }
    ParallelThreshold &operator=(const ParallelThreshold &)
    {
        return *this;
    };

private:
    image_t *m_in;
    image_t *m_out;
    int m_height;
    int m_width;
    float m_low;
    float m_high;
    image_t m_max;
};

class ParallelHysteresis : public ParallelLoopBody
{
public:
    ParallelHysteresis(image_t *matrix_in, image_t *matrix_out, int height, int width)
        : m_in(matrix_in), m_out(matrix_out), m_height(height),
          m_width(width)
    {
    }
    virtual void operator()(const Range &range) const CV_OVERRIDE
    {
        for (int r = range.start; r < range.end; r++)
        {
            int i = r / m_width;
            int k = r % m_width;
            // determine if we should keep the week values
            if (m_in[i * m_width + k] == WEAK)
            {
                m_out[i * m_width + k] = 0;

                // check surroundings
                for (int a = -1; a <= 1; a++)
                {
                    for (int b = -1; b <= 1; b++)
                    {
                        // out of bounds check
                        if (i + a > 0 && i + a < m_height &&
                            k + b > 0 && k + b < m_width)
                        {
                            if (m_in[(i + a) * m_width + (k + b)] == STRONG)
                            {
                                m_out[i * m_width + k] = STRONG;
                                break;
                            }
                        }
                    }
                    // break if the image
                    if (m_out[i * m_width + k] == STRONG)
                    {
                        break;
                    }
                }
            }
            else
            {
                m_out[i * m_width + k] = m_in[i * m_width + k];
            }
        }
    }
    ParallelHysteresis &operator=(const ParallelHysteresis &)
    {
        return *this;
    };

private:
    image_t *m_in;
    image_t *m_out;
    int m_height;
    int m_width;
};
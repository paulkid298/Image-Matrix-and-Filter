int process_camera();
typedef unsigned char image_t;

void grayscale(cv::Mat image, image_t *output, int width, int height);
float *gaussian_kernel(int size, float sigma);
void convolve(image_t *matrix_in, image_t *matrix_out, int height,
              int width, const float *kernel, int kernelSize);
void sobel_filter(image_t *, image_t *, float *,
                  int, int);
image_t non_max_suppression(const image_t *matrix_in, image_t *matrix_out,
                            float *theta, int height, int width);
void threshold(image_t *matrix_in, image_t *matrix_out, int height, int width,
               float lowThreshold, float highThreshold, int max, vector<struct StrongEdge> &vec);

struct Strong_Edge
{
    int row;
    int col;
}
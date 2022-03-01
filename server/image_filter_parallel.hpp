/**
#ifndef PARALLEL_FILTER
#define PARALLEL_FILTER

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;

class ParallelConvolve : public ParallelLoopBody
{
public:
    ParallelConvolve(image_t *matrix_in, image_t *matrix_out, const int height,
                     const int width, const float *kernel, int kernelSize);
    virtual void operator()(const Range &range) const CV_OVERRIDE;
    ParallelConvolve &operator=(const ParallelConvolve &);
};

#endif
*/
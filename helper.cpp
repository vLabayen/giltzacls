#include <opencv2/core.hpp>
#include <helper.h>

cv::Mat bgr2rgb(cv::Mat src){
    uchar* d = src.data;
    uchar tmp;

    for (int i = 0; i < src.rows * src.cols; i++){
        tmp = d[3*i];
        d[3*i] = d[3*i + 2];
        d[3*i + 2] = tmp;
    }

    return src;
}

cv::Mat hflip(cv::Mat src){
    uchar* d = src.data;
    int tmpr, tmpg, tmpb;
    int cols = src.cols / 2;
    for (uint i = 0; i < (uint)src.rows; i++) {
        for (uint j = 0; j < (uint)cols; j++) {
            tmpr = d[3* (i*src.cols + j) + 0];
            tmpg = d[3* (i*src.cols + j) + 1];
            tmpb = d[3* (i*src.cols + j) + 2];

            d[3* (i*src.cols + j) + 0] = d[3* (i*src.cols + src.cols - 1 - j) + 0];
            d[3* (i*src.cols + j) + 1] = d[3* (i*src.cols + src.cols - 1 - j) + 1];
            d[3* (i*src.cols + j) + 2] = d[3* (i*src.cols + src.cols - 1 - j) + 2];

            d[3* (i*src.cols + src.cols - 1 - j) + 0] = tmpr;
            d[3* (i*src.cols + src.cols - 1 - j) + 1] = tmpg;
            d[3* (i*src.cols + src.cols - 1 - j) + 2] = tmpb;
        }
    }
    return src;
}

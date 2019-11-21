#ifndef HELPER_H
#define HELPER_H

#include <opencv2/core.hpp>
#include <sys/stat.h>

cv::Mat bgr2rgb(cv::Mat src);
cv::Mat hflip(cv::Mat src);
bool dir_exists (const char* name);
bool mkdir(const char* name);

#endif // HELPER_H



#ifndef STANDARDSCALER_H
#define STANDARDSCALER_H

#include <mainwindow.h>
#include <locale.h>
#include <math.h>

class StandardScaler
{
public:
    //StandardScaler();
    StandardScaler(int numFeatures);
    StandardScaler(const char* filename);
    ~StandardScaler();

    void fit(cv::Mat data);
    void transform(cv::Mat data);
    void save(const char* filename);
    //void load(const char* filename);

private:
    int numFeatures;
    std::vector<float> featureMean;
    std::vector<float> featureStd;
};

#endif // STANDARDSCALER_H

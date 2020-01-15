#ifndef FEATUREEXTRACTION_H
#define FEATUREEXTRACTION_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>
#include <locale.h>
#include <algorithm>
#include <iterator>

class MainWindow;
struct Features {
    int width;
    int height;
    int area;
    cv::Point center;
    std::vector<float> hProfile;
};

class FeatureExtraction : public QObject{
    Q_OBJECT

public:
    FeatureExtraction(MainWindow* parent);
    ~FeatureExtraction();

    void setup(void);
    int profileColumns = 20;
    int otherFeatures = 4;

    std::vector<float> extractFeatures_v1(cv::Mat src);
    std::vector<float> extractFeatures_v2(cv::Mat src);
    std::vector<float> extractFeatures_v3(cv::Mat src);
    std::vector<float> extractFeatures(cv::Mat src);

    bool showGrayscales = true;

private:
    MainWindow* parent;

    QString mainDir;
    std::vector<cv::Mat> segmentedKeys;
    int currentKeyIndex;

private slots:
    void loadDataset_onClick(void);
    void selectedClass_onChange(int index);
    void loadSegmentedImage_onClick(void);
    void selectedKey_onChange(int index);
    void extractFeatures_onClick(void);
    void exportCsv_onClick(void);
    void exportImages_onClick(void);
};

#endif // FEATUREEXTRACTION_H

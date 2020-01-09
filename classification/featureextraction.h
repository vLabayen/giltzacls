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
    int profileColumns = 10;

    std::vector<float> extractFeatures(cv::Mat src);

private:
    MainWindow* parent;

    QString mainDir;
    std::vector<cv::Mat> segmentedKeys;
    int currentKeyIndex;
    //cv::Mat extractedFeatures;

    //Features extractFeatures(cv::Mat src);
    //cv::Mat selectFeatures(Features f);

private slots:
    void loadDataset_onClick(void);
    void selectedClass_onChange(int index);
    void loadSegmentedImage_onClick(void);
    void selectedKey_onChange(int index);
    void extractFeatures_onClick(void);
    void exportCsv_onClick(void);
};

#endif // FEATUREEXTRACTION_H

#ifndef FEATUREEXTRACTION_H
#define FEATUREEXTRACTION_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>
#include <locale.h>

class MainWindow;
struct Features {
    int height;
    int width;
    int area;
    cv::Point center;
};

class FeatureExtraction : public QObject{
    Q_OBJECT

public:
    FeatureExtraction(MainWindow* parent);
    ~FeatureExtraction();

    void setup(void);

private:
    MainWindow* parent;

    cv::Mat segmentedImage;
    cv::Mat extractedFeatures;

    Features extractFeatures(void);
    cv::Mat selectFeatures(Features f);

private slots:
    void loadDataset_onClick(void);
    void loadSegmentedImage_onClick(void);
    void extractFeatures_onClick(void);
    void exportCsv_onClick(void);
};

#endif // FEATUREEXTRACTION_H

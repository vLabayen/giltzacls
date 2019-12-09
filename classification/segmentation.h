#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class Segmentation : public QObject {
    Q_OBJECT

public:
    Segmentation(MainWindow* parent);
    ~Segmentation();

    void setup(void);

private:
    MainWindow* parent;
    std::vector<cv::Rect>  boundRect;
public slots:
    cv::Mat thresholdingTrimmed(void);
    std::vector<cv::Rect> findBoundingBox1(void);
    void cropBoundingBox(std::vector<cv::Rect> boundRect);
    void List_BoundingBox(void);
    void show_BoundingBox(void);
    cv::Mat SecondthresholdingTrimmed(cv::Mat ImageCropped);
    void RefindBoundingBox(cv::Mat KeySelectedThresholded);
    void BotonSegmentarListener(void);
};

#endif // SEGMENTATION_H

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
    std::vector<cv::RotatedRect> rotatedRect;
public slots:
    cv::Mat thresholdingTrimmed(cv::Mat);
    std::vector<cv::Mat> performSegmentation(cv::Mat);
    std::vector<cv::RotatedRect> findBoundingBox1(cv::Mat);
    void List_BoundingBox(std::vector<cv::RotatedRect>);
    void show_BoundingBox(void);
    cv::Mat show_BoundingBoxOriented(int, std::vector<cv::RotatedRect>, cv::Mat);
    cv::Mat SecondthresholdingTrimmed(cv::Mat ImageCropped);
    void BotonSegmentarListener(void);
    void drawBoundingBox(std::vector<cv::RotatedRect>);
    void drawThresholdedImage(cv::Mat);
    void drawThresholdedkey(cv::Mat);
    void onSelectedIndexCrop(int);

};

#endif // SEGMENTATION_H

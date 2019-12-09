#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <helper.h>
#include <capture.h>
#include <loaddataset.h>
#include <segmentation.h>
#include <training.h>
#include <evaluation.h>
#include <demo.h>

#include <iostream>
#include <stdexcept>

#include <QMainWindow>
#include <QPixelFormat>
#include <QtMultimedia/QCamera>
#include <QTimer>

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/videoio/videoio.hpp>


namespace Ui {
    class MainWindow;
}

class Capture;
class LoadDataset;
class Segmentation;
class Training;
class Evaluation;
class Demo;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

<<<<<<< HEAD
    private:
        Ui::MainWindow *ui;
        cv::Mat imageSelected;
        std::vector<cv::Rect>  boundRect;

    public slots:
        void loadDataset_onClick(void);
        void selectedClass_onChange(int index);
        void previewImage_onClick(void);
        void showHSVplanes(void);
        void showHist(void);
        cv::Mat thresholding(void);
        cv::Mat thresholdingTrimmed(void);
        void findBoundingBox(void);
        std::vector<cv::Rect> findBoundingBox1(void);
        void cropBoundingBox(std::vector<cv::Rect> boundRect);
        void List_BoundingBox(void);
        void show_BoundingBox(void);
        cv::Mat SecondthresholdingTrimmed(cv::Mat ImageCropped);
        void RefindBoundingBox(cv::Mat KeySelectedThresholded);

    private slots:
=======
        Ui::MainWindow* ui;
        Capture* captureManager;
        LoadDataset* loadDatasetManager;
        Segmentation* segmentationManager;
        Training* trainingManager;
        Evaluation* evaluationManager;
        Demo* demoManager;
>>>>>>> master

};

#endif // MAINWINDOW_H

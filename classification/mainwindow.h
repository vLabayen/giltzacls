#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <helper.h>
#include <capture.h>
#include <loaddataset.h>
#include <segmentation.h>
#include <featureextraction.h>
#include <training.h>
#include <demo.h>
#include <standardscaler.h>

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
class FeatureExtraction;
class Training;
class Demo;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        Ui::MainWindow* ui;
        Capture* captureManager;
        LoadDataset* loadDatasetManager;
        Segmentation* segmentationManager;
        FeatureExtraction* featureExtractionManager;
        Training* trainingManager;
        Demo* demoManager;

};

#endif // MAINWINDOW_H

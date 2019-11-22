#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <stdexcept>

#include <helper.h>

#include <QMainWindow>
#include <QPixelFormat>
#include <QtMultimedia/QCamera>
#include <QTimer>

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include "opencv2/video.hpp"
#include <opencv2/video/video.hpp>
#include "opencv2/videoio/videoio.hpp"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        QTimer *timer;
        cv::VideoCapture cap;
        cv::Mat frame;
        //QImage qt_image;

    public slots:
        void startVideo_onClick(void);
        void stopVideo_onClick(void);
        void saveFrame_onClick(void);
        void update_window(void);
        int listCameras(void);

    private slots:
        void removeError_callback();
};

#endif // MAINWINDOW_H

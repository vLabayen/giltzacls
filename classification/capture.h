#ifndef CAPTURE_H
#define CAPTURE_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class Capture : public QObject {
    Q_OBJECT

public:
    Capture(MainWindow* parent);
    ~Capture();
    void setup(void);
    void saveFrame(void);
    void autocapture(void);

private:
    MainWindow* parent;
    QTimer* frameTimer;
    cv::VideoCapture cap;
    cv::Mat frame;
    int autocaptureTimeout;

    int frameRate = 20;
    int tryCameras = 5;


private slots:
    void startVideo_onClick(void);
    void stopVideo_onClick(void);
    void saveFrame_onClick(void);
    void autocapture_onToggle(void);

    void updateFrame(void);
    void autocaptureTimer(int timeout);
    void removeError(void);
};

#endif // CAPTURE_H

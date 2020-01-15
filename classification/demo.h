#ifndef DEMO_H
#define DEMO_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

#include <opencv2/ml/ml.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/dnn.hpp>
//#include <fdeep/fdeep.hpp>

using namespace cv;
using namespace std;

class MainWindow;
class StandardScaler;
struct performSegmentationResponse;

class Demo : public QObject {
    Q_OBJECT

public:
    Demo(MainWindow* parent);
    ~Demo();

    void setup(void);

private:
    MainWindow* parent;
    QTimer* frameTimer;
    cv::VideoCapture cap;
    cv::Mat frame;
    QString mainDir;
    bool autoclassifyFrame = false;

    StandardScaler* scaler;
    Ptr<ml::SVM> SVMmodel;
    cv::dnn::Net net;

    //const fdeep::model* cnnmodel;

    //Label display config
    int fontScale = 1;
    int thickness = 1;

    int tryCameras = 5;
    int frameTime = 20; //ms


    cv::Mat predict(performSegmentationResponse psr);

private slots:
    void searchCameras_onClick(void);
    void startVideo_onClick(void);
    void stopVideo_onClick(void);

    void loadDataset_onClick(void);
    void selectedClass_onChange(int index);
    void selectedImage_onChange(int index);
    void loadImage_onClick(void);

    void loadScaler_onClick(void);
    void loadModel_onClick(void);

    void continuousClassification_onToggle(bool state);
    void classifyFrame_onClick(void);

    void stopVideo(void);
    void updateFrame(void);

    void updateFocus(int valorFocus);
    void updateBrightness(int valorBrightness);
    void updateContrast(int valorContrast);
};

#endif // DEMO_H

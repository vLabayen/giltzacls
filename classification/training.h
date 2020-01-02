#ifndef TRAINING_H
#define TRAINING_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>
#include <standardscaler.h>
#include <opencv2/ml/ml.hpp>
#include <opencv2/ml.hpp>

using namespace cv;
using namespace std;

class MainWindow;
class StandardScaler;
struct Dataset {
    cv::Mat x;
    cv::Mat y;
};

class Training : public QObject {
    Q_OBJECT

public:
    Training(MainWindow* parent);
    ~Training();

    void setup(void);

private:
    MainWindow* parent;
    Dataset dataset;
    StandardScaler* scaler;
    Ptr<ml::SVM> SVMmodel;

    Dataset loadDataset(QString filepath);

private slots:
    void loadDataset_onClick(void);
    void standarizeDataset_onClick(void);
    void saveScaler_onClick(void);
    void trainSVM_onClick(void);
    void saveSVM_onClick(void);
};

#endif // TRAINING_H

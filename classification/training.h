#ifndef TRAINING_H
#define TRAINING_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;
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

    Dataset loadDataset(QString filepath);

private slots:
    void loadDataset_onClick(void);
};

#endif // TRAINING_H

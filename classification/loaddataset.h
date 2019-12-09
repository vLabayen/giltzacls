#ifndef LOADDATASET_H
#define LOADDATASET_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class LoadDataset : public QObject {
    Q_OBJECT

public:
    LoadDataset(MainWindow* parent);
    ~LoadDataset();

    void setup(void);
    void previewImage(int imageIndex);
    void setError(const char* msg);

    QString mainDir;
    QString selectedDir;
    cv::Mat selectedImage;

private:
    MainWindow* parent;

    int errorTimeout = 5000;

private slots:
    void loadDataset_onClick(void);
    void selectedClass_onChange(int index);
    void selectedImage_onChange(int index);
};

#endif // LOADDATASET_H

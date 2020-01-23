#ifndef LOADDATASET_H
#define LOADDATASET_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

struct getClassesResult {
    bool dirFound;
    QString dir;
    int classesFound;
};
struct getImagesResult {
    QString dir;
    int imagesFound;
};

class MainWindow;
class LoadDataset : public QObject {
    Q_OBJECT

public:
    LoadDataset(MainWindow* parent);
    ~LoadDataset();

    void setup(void);
    getClassesResult getClasses(QComboBox* dst_cb, QString mainPath);
    getClassesResult getClasses(QComboBox* dst_cb);
    getImagesResult getImages(QString dir, QString folder, QComboBox* dst_cb);

    QString mainDir;
    QString selectedDir;
    cv::Mat selectedImage;

private:
    MainWindow* parent;
    int errorTimeout = 5000;

    void previewImage(QString dir, int imageIndex);
    void setError(const char* msg);

private slots:
    void loadDataset_onClick(void);
    void selectedClass_onChange(int index);
    void selectedImage_onChange(int index);
};

#endif // LOADDATASET_H

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
#include <opencv2/video.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/videoio/videoio.hpp>


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

    public slots:
        void loadDataset_onClick(void);
        void selectedClass_onChange(int index);
        void previewImage_onClick(void);

    private slots:

};

#endif // MAINWINDOW_H

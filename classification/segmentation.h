#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class Segmentation : public QObject {
    Q_OBJECT

public:
    Segmentation(MainWindow* parent);
    ~Segmentation();

    void setup(void);

private:
    MainWindow* parent;

};

#endif // SEGMENTATION_H

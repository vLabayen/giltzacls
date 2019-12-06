#ifndef TRAINING_H
#define TRAINING_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class Training : public QObject {
    Q_OBJECT

public:
    Training(MainWindow* parent);
    ~Training();

    void setup(void);

private:
    MainWindow* parent;

};

#endif // TRAINING_H

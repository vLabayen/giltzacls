#ifndef EVALUATION_H
#define EVALUATION_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class Evaluation : public QObject {
    Q_OBJECT

public:
    Evaluation(MainWindow* parent);
    ~Evaluation();

    void setup(void);

private:
    MainWindow* parent;

};

#endif // EVALUATION_H

#ifndef DEMO_H
#define DEMO_H

#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <helper.h>

class MainWindow;

class Demo : public QObject {
    Q_OBJECT

public:
    Demo(MainWindow* parent);
    ~Demo();

    void setup(void);

private:
    MainWindow* parent;

};

#endif // DEMO_H

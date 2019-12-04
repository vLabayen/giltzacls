#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    captureManager(new Capture(this))
{
    ui->setupUi(this);
    captureManager->setup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

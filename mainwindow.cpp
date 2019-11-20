#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->startVideo_pushButton, SIGNAL(pressed()), this, SLOT(startVideo_onClick()));
    connect(ui->stopVideo_pushButton, SIGNAL(pressed()), this, SLOT(stopVideo_onClick()));
    connect(ui->saveFrame_pushButton, SIGNAL(pressed()), this, SLOT(saveFrame_onClick()));
}

MainWindow::~MainWindow()
{
    delete actualFrame;
    delete ui;
}

void MainWindow::startVideo_onClick(){
    //ui->rowsInfo_label->setText(QString("Rows : %1").arg(QString::number()));
    //ui->colsInfo_label->setText(QString("Cols : %1").arg(QString::number()));
}

void MainWindow::stopVideo_onClick(){
    ui->rowsInfo_label->setText(QString("Rows : "));
    ui->colsInfo_label->setText(QString("Cols : "));
}

void MainWindow::saveFrame_onClick(){
    QString file = QString("%1/%2/%3").arg(
        ui->dirInput_lineEdit,
        ui->keyInput_lineEdit,
        ui->imageInput_lineEdit
    );
    cv::imwrite(file.toStdString(), actualFrame);
}

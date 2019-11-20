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

     timer = new QTimer(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startVideo_onClick(){
    cap.open(0); // <--- CAMBIAR POR IS AVAILABLE O YO QUE SE QUE OSTIAS

    if(cap.isOpened()){
        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(20);
    }
}

void MainWindow::stopVideo_onClick(){
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    cap.release();
    cv::Mat image = cv::Mat::zeros(frame.size(),CV_8UC3);
    qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qt_image));
    ui->label->resize(ui->label->pixmap()->size());
}

void MainWindow::update_window(){
    cap >> frame;
    cvtColor(frame, frame, CV_BGR2RGB);
    qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qt_image));
    ui->label->resize(ui->label->pixmap()->size());
}


void MainWindow::saveFrame_onClick(){
    QString file = QString("%1/%2/%3").arg(
        ui->dirInput_lineEdit->text(),
        ui->keyInput_lineEdit->text(),
        ui->imageInput_lineEdit->text()
    );

    printf("Ruta: %s", file.toStdString().c_str());
    cvtColor(frame, frame, CV_RGB2BGR);
    cv::imwrite(file.toStdString(), frame);
}

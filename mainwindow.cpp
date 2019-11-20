#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->loadImage_pushButton, SIGNAL(pressed()), this, SLOT(loadImage_onClick()));
    connect(ui->showImage_pushButton, SIGNAL(pressed()), this, SLOT(showImage_onClick()));
    connect(ui->flipImage_pushButton, SIGNAL(pressed()), this, SLOT(flipImage_onClick()));
    connect(ui->toGrayScale_pushButton, SIGNAL(pressed()), this, SLOT(toGrayScale_onClick()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadImage_onClick(){
    QString imagePath = ui->dirInput_lineEdit->text();
    QString imageName = ui->imageInput_lineEdit->text();
    QString imageFile = QString("%1/%2").arg(imagePath, imageName);

    loadedImage = bgr2rgb(cv::imread(imageFile.toStdString()));
    imageFormat = QImage::Format_RGB888;

    ui->rowsInfo_label->setText(QString("Rows : %1").arg(QString::number(loadedImage.rows)));
    ui->colsInfo_label->setText(QString("Cols : %1").arg(QString::number(loadedImage.cols)));
}

void MainWindow::showImage_onClick(){
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(QImage((uchar*) loadedImage.data, loadedImage.cols, loadedImage.rows, loadedImage.step, imageFormat)));
}

void MainWindow::flipImage_onClick(){
    hflip(loadedImage);
}

void MainWindow::toGrayScale_onClick(){
    cv::cvtColor(loadedImage, loadedImage, CV_RGB2GRAY);
    imageFormat = QImage::Format_Grayscale8;
}

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
    ui->rowsInfo_label->setText(QString("Rows : "));
    ui->colsInfo_label->setText(QString("Cols : "));

    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    cap.release();

    cv::Mat image = cv::Mat::zeros(frame.size(),CV_8UC3);
    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    ui->imageDisplay_label->resize(ui->imageDisplay_label->pixmap()->size());
}

void MainWindow::update_window(){
    cap >> frame;
    cvtColor(frame, frame, CV_BGR2RGB);

    QImage qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    ui->imageDisplay_label->resize(ui->imageDisplay_label->pixmap()->size());

    ui->rowsInfo_label->setText(QString("Rows : %1").arg(QString::number(frame.rows)));
    ui->colsInfo_label->setText(QString("Cols : %1").arg(QString::number(frame.cols)));
}


void MainWindow::saveFrame_onClick(){
    ui->error_label->setText("");

    if (!dir_exists(ui->dirInput_lineEdit->text().toStdString().c_str())) {
        ui->error_label->setText("Directory not found");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }

    if (ui->keyInput_lineEdit->text() == QString("")){
        ui->error_label->setText("Key type not set");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }

    QString dir = QString("%1/%2").arg(ui->dirInput_lineEdit->text(), ui->keyInput_lineEdit->text());
    if (!dir_exists(dir.toStdString().c_str())) {
        if (!mkdir(dir.toStdString().c_str())) {
            ui->error_label->setText(QString("Cannot create directory for key %1").arg(ui->keyInput_lineEdit->text()));
            QTimer::singleShot(5000, this, SLOT(removeError_callback()));
            return;
        }
    }

    if (ui->imageInput_lineEdit->text() == QString("")) {
        ui->error_label->setText("Image name not set");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }
    if (ui->extensionInput_lineEdit->text() == QString("")) {
        ui->error_label->setText("Image extension not set");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }

    QString file = QString("%1/%2.%3").arg(dir, ui->imageInput_lineEdit->text(), ui->extensionInput_lineEdit->text());

    cvtColor(frame, frame, CV_RGB2BGR);
    cv::imwrite(file.toStdString(), frame);
    ui->imageInput_lineEdit->setText(QString::number(ui->imageInput_lineEdit->text().toInt() + 1));
}

int MainWindow::listCameras(){
    cv::VideoCapture temp_camera;
    int maxTested = 10;
    for (int i = 0; i < maxTested; i++){
      cv::VideoCapture temp_camera(i);
      bool res = (!temp_camera.isOpened());
      temp_camera.release();
      if (res)
      {
        return i;
      }
    }
    return maxTested;
}

void MainWindow::removeError_callback(){
    ui->error_label->setText("");
}

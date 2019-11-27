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
    connect(ui->autocapture_checkBox, SIGNAL(toggled(bool)), this, SLOT(autoCaptureShot()));

    timer = new QTimer(this);
    listCameras(5, ui->cameraDropdown_comboBox);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startVideo_onClick(){
    cap.open(ui->cameraDropdown_comboBox->currentData().toInt());

    if(cap.isOpened()){
        cap >> frame;
        ui->rowsInfo_label->setText(QString("Filas : %1").arg(QString::number(frame.rows)));
        ui->colsInfo_label->setText(QString("Columnas : %1").arg(QString::number(frame.cols)));

        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(20);
    }
}

void MainWindow::stopVideo_onClick(){
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    cap.release();

    cv::Mat image = cv::Mat::zeros(frame.size(),CV_8UC3);
    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    ui->imageDisplay_label->resize(ui->imageDisplay_label->pixmap()->size());

    ui->rowsInfo_label->setText(QString("Filas : "));
    ui->colsInfo_label->setText(QString("Columnas : "));
}

void MainWindow::update_window(){
    cap >> frame;
    cvtColor(frame, frame, CV_BGR2RGB);

    QImage qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    ui->imageDisplay_label->resize(ui->imageDisplay_label->pixmap()->size());
}


void MainWindow::saveFrame_onClick(){
    if (!cap.isOpened()) {
        ui->error_label->setText("Camara apagada");
        return;
    }

    canSaveImageResult result = canSaveImage(ui->dirInput_lineEdit, ui->keyInput_lineEdit, ui->imageInput_lineEdit, ui->extensionInput_lineEdit);
    if (!result.success){
        ui->error_label->setText(result.error);
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
    } else {
        cvtColor(frame, frame, CV_RGB2BGR);
        cv::imwrite(result.file.toStdString(), frame);

        ui->error_label->setText("");
        ui->imageInput_lineEdit->setText(QString::number(ui->imageInput_lineEdit->text().toInt() + 1));
    }
}

void MainWindow::autoCaptureShot(){
    bool active = ui->autocapture_checkBox->isChecked();
    if (active) {
        int timeout = ui->autocaptureInterval_spinBox->value();

        if (timeout <= 0) ui->autoCaptureDisplay_label->setText("El intervalo tiene que ser > 0");
        else {
            autocaptureTimeout = timeout;
            captureTimer();
        }
    }
}

void MainWindow::captureTimer() {
    if (autocaptureTimeout <= 0) {
        ui->autoCaptureDisplay_label->setText("");
        saveFrame_onClick();
        QTimer::singleShot(1000, this, SLOT(autoCaptureShot()));
    } else {
        ui->autoCaptureDisplay_label->setText(QString("Nueva imagen en %1 s").arg(QString::number(autocaptureTimeout)));
        QTimer::singleShot(1000, this, SLOT(captureTimer()));
        autocaptureTimeout--;
    }
}

void MainWindow::removeError_callback(){
    ui->error_label->setText("");
}

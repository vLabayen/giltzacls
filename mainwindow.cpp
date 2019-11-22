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
    listCameras();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startVideo_onClick(){
    cap.open(ui->cameraDropdown_comboBox->currentData().toInt());

    if(cap.isOpened()){
        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(20);
    }
}

void MainWindow::stopVideo_onClick(){
    ui->rowsInfo_label->setText(QString("Filas : "));
    ui->colsInfo_label->setText(QString("Columnas : "));

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

    ui->rowsInfo_label->setText(QString("Filas : %1").arg(QString::number(frame.rows)));
    ui->colsInfo_label->setText(QString("Columnas : %1").arg(QString::number(frame.cols)));
}


void MainWindow::saveFrame_onClick(){
    ui->error_label->setText("");

    if (!dir_exists(ui->dirInput_lineEdit->text().toStdString().c_str())) {
        ui->error_label->setText("El directorio no existe");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }

    if (ui->keyInput_lineEdit->text() == QString("")){
        ui->error_label->setText("Tipo de llave vacio");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }

    QString dir = QString("%1/%2").arg(ui->dirInput_lineEdit->text(), ui->keyInput_lineEdit->text());
    if (!dir_exists(dir.toStdString().c_str())) {
        if (!mkdir(dir.toStdString().c_str())) {
            ui->error_label->setText(QString("Imposible crear directorio para la llave %1").arg(ui->keyInput_lineEdit->text()));
            QTimer::singleShot(5000, this, SLOT(removeError_callback()));
            return;
        }
    }

    if (ui->imageInput_lineEdit->text() == QString("")) {
        ui->error_label->setText("Nombre de imagen vacío");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }
    if (ui->extensionInput_lineEdit->text() == QString("")) {
        ui->error_label->setText("Extension de imagen vacía");
        QTimer::singleShot(5000, this, SLOT(removeError_callback()));
        return;
    }

    QString file = QString("%1/%2_%3.%4").arg(dir, ui->keyInput_lineEdit->text(), ui->imageInput_lineEdit->text(), ui->extensionInput_lineEdit->text());

    cvtColor(frame, frame, CV_RGB2BGR);
    cv::imwrite(file.toStdString(), frame);
    ui->imageInput_lineEdit->setText(QString::number(ui->imageInput_lineEdit->text().toInt() + 1));
}

void MainWindow::autoCaptureShot(){
    bool active = ui->autocapture_checkBox->isChecked();
    if (active) {
        int timeout = ui->autocaptureInterval_spinBox->value();

        if (timeout <= 0) ui->autoCaptureDisplay_label->setText("El intervalo no puede ser 0");
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


void MainWindow::listCameras(){
    cv::VideoCapture tmp_camera;
    int maxTested = 5;
    for (int i = 0; i < maxTested; i++){
        bool res = false;
        try {
            tmp_camera.open(i);
            res = tmp_camera.isOpened();
            tmp_camera.release();
        }
        catch (...) {
            continue;
        }

        if (res) ui->cameraDropdown_comboBox->addItem(QString("Camara %1").arg(QString::number(i)), QVariant(i));
    }
}

void MainWindow::removeError_callback(){
    ui->error_label->setText("");
}

#include "capture.h"

Capture::Capture(MainWindow* parent) : parent(parent) {}
Capture::~Capture() {}

void Capture::setup(){
    listCameras(tryCameras, parent->ui->cameraDropdown_comboBox);

    connect(parent->ui->startVideo_pushButton, SIGNAL(pressed()), this, SLOT(startVideo_onClick()));
    connect(parent->ui->stopVideo_pushButton, SIGNAL(pressed()), this, SLOT(stopVideo_onClick()));
    connect(parent->ui->saveFrame_pushButton, SIGNAL(pressed()), this, SLOT(saveFrame_onClick()));
    connect(parent->ui->autocapture_checkBox, SIGNAL(toggled(bool)), this, SLOT(autocapture_onToggle()));

    frameTimer = new QTimer(this);
}

void Capture::saveFrame(){
    if (!cap.isOpened()) {
        parent->ui->error_label->setText("Camara apagada");
        return;
    }

    canSaveImageResult result = canSaveImage(
        parent->ui->dirInput_lineEdit,
        parent->ui->keyInput_lineEdit,
        parent->ui->imageInput_lineEdit,
        parent->ui->extensionInput_lineEdit
    );

    if (!result.success){
        parent->ui->error_label->setText(result.error);
        QTimer::singleShot(5000, this, SLOT(removeError));
        return;
    }

    cvtColor(frame, frame, CV_RGB2BGR);
    cv::imwrite(result.file.toStdString(), frame);

    parent->ui->error_label->setText("");
    parent->ui->imageInput_lineEdit->setText(QString::number(parent->ui->imageInput_lineEdit->text().toInt() + 1));
}

void Capture::autocapture(){
    if (!parent->ui->autocapture_checkBox->isChecked()) return;

    int timeout = parent->ui->autocaptureInterval_spinBox->value();
    if (timeout <= 0) parent->ui->autoCaptureDisplay_label->setText("El intervalo tiene que ser > 0");
    else autocaptureTimer(timeout);
}


void Capture::startVideo_onClick(){
    cap.open(parent->ui->cameraDropdown_comboBox->currentData().toInt());

    if(cap.isOpened()){
        cap >> frame;
        parent->ui->rowsInfo_label->setText(QString("Filas : %1").arg(QString::number(frame.rows)));
        parent->ui->colsInfo_label->setText(QString("Columnas : %1").arg(QString::number(frame.cols)));

        connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
        frameTimer->start(frameRate);
    }
}

void Capture::stopVideo_onClick(){
    disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    cap.release();

    cv::Mat image = cv::Mat::zeros(frame.size(),CV_8UC3);
    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    parent->ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    parent->ui->imageDisplay_label->resize(parent->ui->imageDisplay_label->pixmap()->size());

    parent->ui->rowsInfo_label->setText(QString("Filas : "));
    parent->ui->colsInfo_label->setText(QString("Columnas : "));
}

void Capture::saveFrame_onClick(){
    saveFrame();
}

void Capture::autocapture_onToggle(){
    autocapture();
}


void Capture::updateFrame(){
    cap >> frame;
    cvtColor(frame, frame, CV_BGR2RGB);

    QImage qt_image = QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    parent->ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    parent->ui->imageDisplay_label->resize(parent->ui->imageDisplay_label->pixmap()->size());
}

void Capture::autocaptureTimer(int timeout){
    if (timeout <= 0) {
        parent->ui->autoCaptureDisplay_label->setText("");
        QTimer::singleShot(1000, this, [this]()->void{autocapture();});
        saveFrame();
    } else {
        parent->ui->autoCaptureDisplay_label->setText(QString("Nueva imagen en %1 s").arg(QString::number(timeout)));
        QTimer::singleShot(1000, this, [this, timeout]()->void{autocaptureTimer(timeout - 1);});
    }
}

void Capture::removeError(){
    parent->ui->error_label->setText("");
}


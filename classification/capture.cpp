#include "capture.h"

Capture::Capture(MainWindow* parent) : parent(parent) {}
Capture::~Capture() {}

void Capture::setup(){
    listCameras(tryCameras, parent->ui->capture_cameraDropdown_comboBox);

    connect(parent->ui->capture_startVideo_pushButton, SIGNAL(pressed()), this, SLOT(startVideo_onClick()));
    connect(parent->ui->capture_stopVideo_pushButton, SIGNAL(pressed()), this, SLOT(stopVideo_onClick()));
    connect(parent->ui->capture_saveFrame_pushButton, SIGNAL(pressed()), this, SLOT(saveFrame_onClick()));
    connect(parent->ui->capture_autocapture_checkBox, SIGNAL(toggled(bool)), this, SLOT(autocapture_onToggle()));

    frameTimer = new QTimer(this);
}

void Capture::saveFrame(){
    if (!cap.isOpened()) {
        parent->ui->capture_saveError_label->setText("Camara apagada");
        return;
    }

    canSaveImageResult result = canSaveImage(
        parent->ui->capture_dir_lineEdit,
        parent->ui->capture_key_lineEdit,
        parent->ui->capture_image_lineEdit,
        parent->ui->capture_extension_lineEdit
    );

    if (!result.success){
        parent->ui->capture_saveError_label->setText(result.error);
        QTimer::singleShot(5000, this, SLOT(removeError()));
        return;
    }

    cvtColor(frame, frame, CV_RGB2BGR);
    cv::imwrite(result.file.toStdString(), frame);

    parent->ui->capture_saveError_label->setText("");
    parent->ui->capture_image_lineEdit->setText(QString::number(parent->ui->capture_image_lineEdit->text().toInt() + 1));
}

void Capture::autocapture(){
    if (!parent->ui->capture_autocapture_checkBox->isChecked()) return;

    int timeout = parent->ui->capture_autocapture_spinBox->value();
    if (timeout <= 0) parent->ui->capture_autocaptureDisplay_label->setText("El intervalo tiene que ser > 0");
    else autocaptureTimer(timeout);
}


void Capture::startVideo_onClick(){
    cap.open(parent->ui->capture_cameraDropdown_comboBox->currentData().toInt());

    if(cap.isOpened()){
        cap >> frame;
        parent->ui->capture_rowsInfo_label->setText(QString("Filas : %1").arg(QString::number(frame.rows)));
        parent->ui->capture_colsInfo_label->setText(QString("Columnas : %1").arg(QString::number(frame.cols)));

        connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
        frameTimer->start(frameRate);
    }
}

void Capture::stopVideo_onClick(){
    disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    cap.release();

    cv::Mat image = cv::Mat::zeros(frame.size(),CV_8UC3);
    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    parent->ui->capture_imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    //parent->ui->capture_imageDisplay_label->resize(parent->ui->capture_imageDisplay_label->pixmap()->size());

    parent->ui->capture_rowsInfo_label->setText(QString("Filas : "));
    parent->ui->capture_colsInfo_label->setText(QString("Columnas : "));
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
    parent->ui->capture_imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));
    //parent->ui->capture_imageDisplay_label->resize(parent->ui->capture_imageDisplay_label->pixmap()->size());
}

void Capture::autocaptureTimer(int timeout){
    if (timeout <= 0) {
        parent->ui->capture_autocaptureDisplay_label->setText("");
        QTimer::singleShot(1000, this, [this]()->void{autocapture();});
        saveFrame();
    } else {
        parent->ui->capture_autocaptureDisplay_label->setText(QString("Nueva imagen en %1 s").arg(QString::number(timeout)));
        QTimer::singleShot(1000, this, [this, timeout]()->void{autocaptureTimer(timeout - 1);});
    }
}

void Capture::removeError(){
    parent->ui->capture_saveError_label->setText("");
}


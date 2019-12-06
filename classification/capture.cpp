#include "capture.h"

Capture::Capture(MainWindow* parent) : parent(parent) {}
Capture::~Capture() {}

void Capture::setup(){
    connect(parent->ui->capture_searchCameras_pushButton, SIGNAL(pressed()), this, SLOT(searchCameras_onClick()));
    connect(parent->ui->capture_startVideo_pushButton, SIGNAL(pressed()), this, SLOT(startVideo_onClick()));
    connect(parent->ui->capture_stopVideo_pushButton, SIGNAL(pressed()), this, SLOT(stopVideo_onClick()));
    connect(parent->ui->capture_saveFrame_pushButton, SIGNAL(pressed()), this, SLOT(saveFrame_onClick()));
    connect(parent->ui->capture_autocapture_checkBox, SIGNAL(toggled(bool)), this, SLOT(autocapture_onToggle(bool)));

    frameTimer = new QTimer(this);
}

void Capture::searchCameras_onClick(){
    parent->ui->capture_cameraError_label->setText("");

    //Obtenemos las camaras disponibles. Mostramos un error de no haberlas
    if (listCameras(tryCameras, parent->ui->capture_cameraDropdown_comboBox) == 0){
        parent->ui->capture_cameraError_label->setText("No hay camaras disponibles");
        QTimer::singleShot(errorTimeout, this, [this]()->void{
            parent->ui->capture_cameraError_label->setText("");
        });
    }
}

void Capture::startVideo_onClick(){
    //Nos conectamos a la camara
    cap.open(parent->ui->capture_cameraDropdown_comboBox->currentData().toInt());

    //Mostramos un error si no esta disponible
    if(!cap.isOpened()){
        parent->ui->capture_cameraError_label->setText("Error al conectar");
        QTimer::singleShot(errorTimeout, this, [this]()->void{
            parent->ui->capture_cameraError_label->setText("");
        });
        return;
    }

    //Iniciamos el timer para refrescar la camara
    connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    frameTimer->start(frameRate);

    //Obtenemos el primer frame para mostrar la información sobre la resolución
    cap >> frame;
    parent->ui->capture_rowsInfo_label->setText(QString("Filas : %1").arg(QString::number(frame.rows)));
    parent->ui->capture_colsInfo_label->setText(QString("Columnas : %1").arg(QString::number(frame.cols)));
}

void Capture::stopVideo_onClick(){
    //Desconectamos el timer para el refresco de la camara y la liberamos
    disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    cap.release();

    //Eliminamos el ultimo frame mostrado y la información de la resolución
    parent->ui->capture_imageDisplay_label->clear();
    parent->ui->capture_rowsInfo_label->setText(QString("Filas : "));
    parent->ui->capture_colsInfo_label->setText(QString("Columnas : "));
}

void Capture::saveFrame_onClick(){
    saveFrame();
}

void Capture::autocapture_onToggle(bool active){
    if (active) autocapture();
}


void Capture::updateFrame(){
    //Leemos el frame en RGB
    cap >> frame;
    cvtColor(frame, frame, CV_BGR2RGB);

    //Mostramos el frame en el label
    parent->ui->capture_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888)));
}

void Capture::autocapture(){
    //Obtenemos el intervalo de capturas automáticas
    int timeout = parent->ui->capture_autocapture_spinBox->value();

    //Mostramos un error si es igual o menor que 0
    if (timeout <= 0) {
        parent->ui->capture_autocaptureDisplay_label->setText("El intervalo tiene que ser > 0");
        QTimer::singleShot(errorTimeout, this, [this]()->void{
            //RACE CONDITION
            //Puede quitar el texto de la cuenta atras si esta se lanza antes de que se quite el error
            parent->ui->capture_autocaptureDisplay_label->setText("");
        });
    //Lanzamos la autocaptura
    } else autocaptureTimer(timeout);
}

void Capture::autocaptureTimer(int timeout){
    //Si hemos terminado la cuenta atras
    if (timeout <= 0) {
        //Eliminamos el mensaje
        parent->ui->capture_autocaptureDisplay_label->setText("");
        //Volvemos a lanzar la autocaptura si sigue activada
        if (parent->ui->capture_autocapture_checkBox->isChecked()) QTimer::singleShot(1000, this, [this]()->void{autocapture();});
        //Guardamos la imagen
        saveFrame();
    } else {
        //Actualizamos la cuenta atras
        parent->ui->capture_autocaptureDisplay_label->setText(QString("Nueva imagen en %1 s").arg(QString::number(timeout)));
        QTimer::singleShot(1000, this, [this, timeout]()->void{autocaptureTimer(timeout - 1);});
    }
}

void Capture::saveFrame(){
    //Eliminamos posibles errores
    parent->ui->capture_saveError_label->setText("");

    //Lanzamos un error si no hay camara activa
    if (!cap.isOpened()) {
        parent->ui->capture_saveError_label->setText("Camara apagada");
        QTimer::singleShot(errorTimeout, this, [this]()->void{
            parent->ui->capture_saveError_label->setText("");
        });
        return;
    }

    //Comprobamos que las rutas y nombres son validos
    canSaveImageResult result = canSaveImage(
        parent->ui->capture_dir_lineEdit,
        parent->ui->capture_key_lineEdit,
        parent->ui->capture_image_lineEdit,
        parent->ui->capture_extension_lineEdit
    );

    //Lanzamos un error de no serlo
    if (!result.success){
        parent->ui->capture_saveError_label->setText(result.error);
        QTimer::singleShot(errorTimeout, this, [this]()->void{
            parent->ui->capture_saveError_label->setText("");
        });
        return;
    }


    //Guardamos el último frame mostrado
    cv::imwrite(result.file.toStdString(), frame);

    //Incrementamos el nombre de la imagen para evitar sobreescribirla en posteriores capturas
    parent->ui->capture_image_lineEdit->setText(QString::number(parent->ui->capture_image_lineEdit->text().toInt() + 1));
}

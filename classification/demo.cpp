#include "demo.h"

Demo::Demo(MainWindow* parent) : parent(parent) {}
Demo::~Demo() {
    delete scaler;
}

void Demo::setup(){
    parent->ui->demo_modelType_comboBox->addItem(QString("SVM"), QVariant(true));
    parent->ui->demo_modelType_comboBox->addItem(QString("CNN"), QVariant(false));

    connect(parent->ui->demo_searchCameras_pushButton, SIGNAL(pressed()), this, SLOT(searchCameras_onClick()));
    connect(parent->ui->demo_startVideo_pushButton, SIGNAL(pressed()), this, SLOT(startVideo_onClick()));
    connect(parent->ui->demo_stopVideo_pushButton, SIGNAL(pressed()), this, SLOT(stopVideo_onClick()));

    connect(parent->ui->demo_loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
    connect(parent->ui->demo_loadImage_pushButton, SIGNAL(pressed()), this, SLOT(loadImage_onClick()));

    connect(parent->ui->demo_loadScaler_pushButton, SIGNAL(pressed()), this, SLOT(loadScaler_onClick()));
    connect(parent->ui->demo_loadModel_pushButton, SIGNAL(pressed()), this, SLOT(loadModel_onClick()));

    connect(parent->ui->demo_continuousClassification_checkBox, SIGNAL(toggled(bool)), this, SLOT(continuousClassification_onToggle(bool)));
    connect(parent->ui->demo_classifyFrame_pushButton, SIGNAL(pressed()), this, SLOT(classifyFrame_onClick()));
    connect(parent->ui->demo_focus, SIGNAL(valueChanged(int)), this, SLOT(updateFocus(int)));
    connect(parent->ui->demo_brightness, SIGNAL(valueChanged(int)), this, SLOT(updateBrightness(int)));
    connect(parent->ui->demo_contrast, SIGNAL(valueChanged(int)), this, SLOT(updateContrast(int)));

    frameTimer = new QTimer(this);
}

void Demo::searchCameras_onClick(){
    listCameras(tryCameras, parent->ui->demo_cameraDropdown_comboBox);
}

void Demo::startVideo_onClick(){
    //Nos conectamos a la camara
    cap.open(parent->ui->demo_cameraDropdown_comboBox->currentData().toInt());

    if(!cap.isOpened()) return;

    //Iniciamos el timer para refrescar la camara
    connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    frameTimer->start(frameTime);
}
void Demo::updateFocus(int valorFocus){
    cap.set(cv::CAP_PROP_AUTOFOCUS, 0);
    cap.set(cv::CAP_PROP_FOCUS, valorFocus);
}

void Demo::updateBrightness(int valorBrightness){
    cap.set(cv::CAP_PROP_BRIGHTNESS, valorBrightness);
}

void Demo::updateContrast(int valorContrast){
    cap.set(cv::CAP_PROP_CONTRAST, valorContrast);
}

void Demo::stopVideo_onClick(){
    stopVideo();

    //Eliminamos el ultimo frame mostrado
    parent->ui->demo_cameraDisplay_label->clear();
}

void Demo::loadDataset_onClick(){
    //Desconectamos los callbacks y limpiamos el contenido de los combobox
    disconnect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    disconnect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
    parent->ui->demo_class_comboBox->clear();
    parent->ui->demo_image_comboBox->clear();

    //Cargamos clases e imagenes
    getClassesResult rc = parent->loadDatasetManager->getClasses(parent->ui->demo_class_comboBox, parent->ui->demo_datastePath_lineEdit->text());
    parent->loadDatasetManager->getImages(rc.dir, parent->ui->demo_class_comboBox->currentData().toString(), parent->ui->demo_image_comboBox);

    //Guardamos referencia al directorio utilizado
    mainDir = rc.dir;

    //Reconectamos los combobox
    connect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void Demo::selectedClass_onChange(int index){
    //Desconectamos y limpiamos
    disconnect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
    parent->ui->demo_image_comboBox->clear();

    //Volvemos a rellenar con las imagenes de la clase correspondiente
    parent->loadDatasetManager->getImages(mainDir, parent->ui->demo_class_comboBox->itemData(index).toString(), parent->ui->demo_image_comboBox);

    //Reconctamos
    connect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

//Automatizamos la previsualizacion de imagenes cuando se cambia la seleccionada
void Demo::selectedImage_onChange(int index){
    if (cap.isOpened()) return;

    loadImage_onClick();
}

void Demo::loadImage_onClick(){
    //Preparamos el path a la imagen
    QString image = QString("%1/%2/%3").arg(
        mainDir,
        parent->ui->demo_class_comboBox->currentData().toString(),
        parent->ui->demo_image_comboBox->currentData().toString()
    );

    //La cargamos en formato rgb
    frame = cv::imread(image.toStdString().c_str());
    cvtColor(frame, frame, CV_BGR2RGB);

    //Paramos el video si estaba activo y mostramos la imagen recien cargada
    if (cap.isOpened()) stopVideo();
    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888)));

    //Si esta el checkbox de clasificación automática procedemos a clasificar las llaves
    if (autoclassifyFrame) {
        performSegmentationResponse psr = parent->segmentationManager->performSegmentation(frame, isUmbralizedRequired(useSVM, useGrayscale));
        cv::Mat pred = predict(psr);
        for (int i = 0; i < pred.rows; i++) cv::putText(psr.unlabeledImage, QString::number(pred.at<float>(i, 0)).toStdString().c_str(), psr.labelsPosition[i] , CV_FONT_HERSHEY_PLAIN, fontScale, CV_RGB(0,255,0), thickness);

        parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (psr.unlabeledImage.data), psr.unlabeledImage.cols, psr.unlabeledImage.rows, QImage::Format_RGB888)));
    }
}

void Demo::loadScaler_onClick(){
    QString scalerPath = parent->ui->demo_scalerPath_lineEdit->text();
    scaler = new StandardScaler(scalerPath.toStdString().c_str());

    //https://stackoverflow.com/questions/15310846/creating-a-class-object-in-c
    //https://www.tutorialspoint.com/pointers-smart-pointers-and-shared-pointers-in-cplusplus
}

void Demo::loadModel_onClick(){
    QString modelPath = parent->ui->demo_modelPath_lineEdit->text();

    //Cargamos el tipo de modelo correspondiente. Se generaran errores si no es el adecuado
    try {
        useSVM = parent->ui->demo_modelType_comboBox->currentData().toBool();
        if (useSVM){
            SVMmodel = ml::SVM::create();
            SVMmodel = ml::SVM::load(modelPath.toStdString().c_str());
        } else {
            cnnnet = cv::dnn::readNetFromTensorflow(modelPath.toStdString().c_str());
        }
    } catch (...) {}
}

void Demo::continuousClassification_onToggle(bool state){
    autoclassifyFrame = state;
}

void Demo::classifyFrame_onClick(){
    //Paramos el video para que se mantenga el frame clasificado
    if (cap.isOpened()) stopVideo();

    //Segmentamos y clasificamos las llaves
    performSegmentationResponse psr = parent->segmentationManager->performSegmentation(frame, isUmbralizedRequired(useSVM, useGrayscale));
    cv::Mat pred = predict(psr);
    //Ponemos el label junto con cada una de las llaves detectadas
    for (int i = 0; i < pred.rows; i++) cv::putText(psr.unlabeledImage, QString::number(pred.at<float>(i, 0)).toStdString().c_str(), psr.labelsPosition[i] , CV_FONT_HERSHEY_PLAIN, fontScale, CV_RGB(0,255,0), thickness);

    //Mostramos el frame ya clasificado y con sus labels
    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (psr.unlabeledImage.data), psr.unlabeledImage.cols, psr.unlabeledImage.rows, QImage::Format_RGB888)));
}

void Demo::stopVideo(){
    //Desconectamos el timer para el refresco de la camara y la liberamos
    disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    cap.release();
}

cv::Mat Demo::predict(performSegmentationResponse psr){
    //Preparamos una matriz para los labels de las llaves
    cv::Mat pred((int)psr.keys.size(), 1, CV_32F);

    //Realizamos distinto procedimiento en funcion del clasificador a emplear
    if (useSVM){
        //Preparamos una matriz para las features de las llaves
        cv::Mat keysFeatures((int)psr.keys.size(), parent->featureExtractionManager->otherFeatures + parent->featureExtractionManager->profileColumns, CV_32F);

        //Extramos las features y las añadimos a la matriz. Cada fila seran las features de una llave
        for (int i = 0; i < (int)psr.keys.size(); i++){
            std::vector<float> features = parent->featureExtractionManager->extractFeatures(psr.keys[i]);
            for (int j = 0; j < (int)features.size(); j++) keysFeatures.at<float>(i, j) = features[j];
        }

        //Escalamos las features y utilizamos svm para clasificar. Los labels se escriben sobre la matriz preparada anteriormente
        scaler->transform(keysFeatures);
        SVMmodel->predict(keysFeatures, pred, ml::ROW_SAMPLE);
    } else {
        //Preparamos un vector de matrices para almacenar cada una de las llaves por separado
        std::vector<cv::Mat> dnnInput((int)psr.keys.size());

        //Redimensionamos a 90x30 cada una de las llaves y la guardamos en el vector de matrices
        for (int i = 0; i < (int)psr.keys.size(); i++){
            cv::Mat resizedImg;
            cv::resize(psr.keys[i], resizedImg, cv::Size(90,30), 0, 0, CV_INTER_AREA);
            dnnInput[i] = resizedImg;
        }
        //Generamos un blob de NxCxWxH, que es el input de la red neuronal
        Mat inputBlob = cv::dnn::blobFromImages(dnnInput);

        //Establecemos el input y ejecutamos la prediccion de la red
        cnnnet.setInput(inputBlob);
        cv::Mat netprob = cnnnet.forward();

        //De las distintas probabilidades para cada llave, nos quedamos con la mayor
        //Escribimos los resultados sobre la matriz preparada en un principio
        for (int i = 0; i < netprob.rows; i++){
            float max = 0;
            int maxIndex;
            for (int j = 0; j < netprob.cols; j++){
                float prob = netprob.at<float>(i, j);
                if (prob > max){
                    max = prob;
                    maxIndex = j;
                }
                pred.at<float>(i, 0) = (float) maxIndex + 1; //Sumamos 1 para tener llaves de 1 a 6 en vez de 0 a 5
            }
        }
    }

    return pred;
}

void Demo::updateFrame(){
    //Leemos el frame en RGB
    cap >> frame;
    cvtColor(frame, frame, CV_BGR2RGB);

    if (autoclassifyFrame){
        //Desconectamos el timer por si el tiempo de procesado es mayor que el de captura de imagenes
        disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));

        //Realizamos la segmentacion y clasificacion. Añadimos los labels sobre la imagen y reemplazamos el contenido de la imagen capturada
        performSegmentationResponse psr = parent->segmentationManager->performSegmentation(frame, isUmbralizedRequired(useSVM, useGrayscale));
        cv::Mat pred = predict(psr);
        for (int i = 0; i < pred.rows; i++) cv::putText(psr.unlabeledImage, QString::number(pred.at<float>(i, 0)).toStdString().c_str(), psr.labelsPosition[i] , CV_FONT_HERSHEY_PLAIN, fontScale, CV_RGB(0,255,0), thickness);
        frame = psr.unlabeledImage;

        //Nos reconectamos al timer para seguir con el video activo
        connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    }


    //Mostramos el frame en el display
    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888)));
}

bool Demo::isUmbralizedRequired(bool svm, bool grayscale){
    //Si estamos usando svm siempre queremos imagen umbralizada
    if (svm) return true;
    //En caso contrario, la querremos si queremos escala de grises
    return grayscale;
}

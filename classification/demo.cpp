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
    disconnect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    disconnect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    parent->ui->demo_class_comboBox->clear();
    parent->ui->demo_image_comboBox->clear();

    getClassesResult rc = parent->loadDatasetManager->getClasses(parent->ui->demo_class_comboBox, parent->ui->demo_datastePath_lineEdit->text());
    parent->loadDatasetManager->getImages(rc.dir, parent->ui->demo_class_comboBox->currentData().toString(), parent->ui->demo_image_comboBox);

    mainDir = rc.dir;

    connect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void Demo::selectedClass_onChange(int index){
    disconnect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    parent->ui->demo_image_comboBox->clear();
    parent->loadDatasetManager->getImages(mainDir, parent->ui->demo_class_comboBox->itemData(index).toString(), parent->ui->demo_image_comboBox);

    connect(parent->ui->demo_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void Demo::selectedImage_onChange(int index){
    if (cap.isOpened()) return;

    loadImage_onClick();
}

void Demo::loadImage_onClick(){
    QString image = QString("%1/%2/%3").arg(
        mainDir,
        parent->ui->demo_class_comboBox->currentData().toString(),
        parent->ui->demo_image_comboBox->currentData().toString()
    );

    frame = cv::imread(image.toStdString().c_str());
    cvtColor(frame, frame, CV_BGR2RGB);

    if (cap.isOpened()) stopVideo();
    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888)));

    if (autoclassifyFrame) {
        performSegmentationResponse psr = parent->segmentationManager->performSegmentation(frame);
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

    useSVM = parent->ui->demo_modelType_comboBox->currentData().toBool();
    if (useSVM){
        SVMmodel = ml::SVM::create();
        SVMmodel = ml::SVM::load(modelPath.toStdString().c_str());
    } else {
        cnnnet = cv::dnn::readNetFromTensorflow(modelPath.toStdString().c_str());
    }
}

void Demo::continuousClassification_onToggle(bool state){
    autoclassifyFrame = state;
}

void Demo::classifyFrame_onClick(){
    if (cap.isOpened()) stopVideo();

    performSegmentationResponse psr = parent->segmentationManager->performSegmentation(frame);
    cv::Mat pred = predict(psr);
    for (int i = 0; i < pred.rows; i++) cv::putText(psr.unlabeledImage, QString::number(pred.at<float>(i, 0)).toStdString().c_str(), psr.labelsPosition[i] , CV_FONT_HERSHEY_PLAIN, fontScale, CV_RGB(0,255,0), thickness);

    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (psr.unlabeledImage.data), psr.unlabeledImage.cols, psr.unlabeledImage.rows, QImage::Format_RGB888)));
}

void Demo::stopVideo(){
    //Desconectamos el timer para el refresco de la camara y la liberamos
    disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    cap.release();
}

cv::Mat Demo::predict(performSegmentationResponse psr){
    cv::Mat pred((int)psr.keys.size(), 1, CV_32F);

    if (useSVM){
        cv::Mat keysFeatures((int)psr.keys.size(), parent->featureExtractionManager->otherFeatures + parent->featureExtractionManager->profileColumns, CV_32F);

        for (int i = 0; i < (int)psr.keys.size(); i++){
            std::vector<float> features = parent->featureExtractionManager->extractFeatures(psr.keys[i]);
            for (int j = 0; j < (int)features.size(); j++) keysFeatures.at<float>(i, j) = features[j];
        }

        scaler->transform(keysFeatures);
        SVMmodel->predict(keysFeatures, pred, ml::ROW_SAMPLE);
    } else {
        std::vector<cv::Mat> dnnInput((int)psr.keys.size());

        for (int i = 0; i < (int)psr.keys.size(); i++){
            cv::Mat resizedImg;
            cv::resize(psr.keys[i], resizedImg, cv::Size(90,30), 0, 0, CV_INTER_AREA);
            dnnInput[i] = resizedImg;
        }
        Mat inputBlob = cv::dnn::blobFromImages(dnnInput);

        cnnnet.setInput(inputBlob);
        cv::Mat netprob = cnnnet.forward();

        for (int i = 0; i < netprob.rows; i++){
            float max = 0;
            int maxIndex;
            for (int j = 0; j < netprob.cols; j++){
                float prob = netprob.at<float>(i, j);
                if (prob > max){
                    max = prob;
                    maxIndex = j;
                }
                pred.at<float>(i, 0) = (float) maxIndex + 1;
            }
        }
    }

    return pred;
}

void Demo::updateFrame(){
    //Leemos el frame en RGB
    cap >> frame;
  
    if (autoclassifyFrame){
        disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));

        performSegmentationResponse psr = parent->segmentationManager->performSegmentation(frame);
        cv::Mat pred = predict(frame, psr);
        for (int i = 0; i < pred.rows; i++) cv::putText(psr.unlabeledImage, QString::number(pred.at<float>(i, 0)).toStdString().c_str(), psr.labelsPosition[i] , CV_FONT_HERSHEY_PLAIN, fontScale, CV_RGB(0,255,0), thickness);
        frame = psr.unlabeledImage;

        connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    }

    cvtColor(frame, frame, CV_BGR2RGB);
    //Mostramos el frame en el label
    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888)));

}

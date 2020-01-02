#include "demo.h"

Demo::Demo(MainWindow* parent) : parent(parent) {}
Demo::~Demo() {
    delete scaler;
}

void Demo::setup(){
    connect(parent->ui->demo_searchCameras_pushButton, SIGNAL(pressed()), this, SLOT(searchCameras_onClick()));
    connect(parent->ui->demo_startVideo_pushButton, SIGNAL(pressed()), this, SLOT(startVideo_onClick()));
    connect(parent->ui->demo_stopVideo_pushButton, SIGNAL(pressed()), this, SLOT(stopVideo_onClick()));

    connect(parent->ui->demo_loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->demo_loadImage_pushButton, SIGNAL(pressed()), this, SLOT(loadImage_onClick()));

    connect(parent->ui->demo_loadScaler_pushButton, SIGNAL(pressed()), this, SLOT(loadScaler_onClick()));
    connect(parent->ui->demo_loadModel_pushButton, SIGNAL(pressed()), this, SLOT(loadModel_onClick()));

    connect(parent->ui->demo_continuousClassification_checkBox, SIGNAL(toggled(bool)), this, SLOT(continuousClassification_onToggle(bool)));
    connect(parent->ui->demo_classifyFrame_pushButton, SIGNAL(pressed()), this, SLOT(classifyFrame_onClick()));

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

void Demo::stopVideo_onClick(){
    stopVideo();

    //Eliminamos el ultimo frame mostrado
    parent->ui->demo_cameraDisplay_label->clear();
    parent->ui->demo_predictedKey_label->setText("");
}

void Demo::loadDataset_onClick(){
    disconnect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    parent->ui->demo_class_comboBox->clear();
    parent->ui->demo_image_comboBox->clear();

    getClassesResult rc = parent->loadDatasetManager->getClasses(parent->ui->demo_class_comboBox, parent->ui->demo_datastePath_lineEdit->text());
    parent->loadDatasetManager->getImages(rc.dir, parent->ui->demo_class_comboBox->currentData().toString(), parent->ui->demo_image_comboBox);

    mainDir = rc.dir;

    connect(parent->ui->demo_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
}

void Demo::selectedClass_onChange(int index){
    parent->ui->demo_image_comboBox->clear();
    parent->loadDatasetManager->getImages(mainDir, parent->ui->demo_class_comboBox->itemData(index).toString(), parent->ui->demo_image_comboBox);
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
        cv::Mat pred = predict(frame);
        parent->ui->demo_predictedKey_label->setText(QString("LLave %1").arg(QString::number(pred.at<float>(0, 0))));
    } else parent->ui->demo_predictedKey_label->setText("");
}

void Demo::loadScaler_onClick(){
    QString scalerPath = parent->ui->demo_scalerPath_lineEdit->text();
    scaler = new StandardScaler(scalerPath.toStdString().c_str());

    //https://stackoverflow.com/questions/15310846/creating-a-class-object-in-c
    //https://www.tutorialspoint.com/pointers-smart-pointers-and-shared-pointers-in-cplusplus
}

void Demo::loadModel_onClick(){
    QString modelPath = parent->ui->demo_modelPath_lineEdit->text();

    SVMmodel = ml::SVM::create();
    SVMmodel = ml::SVM::load(modelPath.toStdString().c_str());
}

void Demo::continuousClassification_onToggle(bool state){
    autoclassifyFrame = state;
}

void Demo::classifyFrame_onClick(){
    if (cap.isOpened()) stopVideo();

    cv::Mat pred = predict(frame);
    parent->ui->demo_predictedKey_label->setText(QString("LLave %1").arg(QString::number(pred.at<float>(0, 0))));
}

void Demo::stopVideo(){
    //Desconectamos el timer para el refresco de la camara y la liberamos
    disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    cap.release();
}

cv::Mat Demo::predict(cv::Mat img){
    std::vector<cv::Mat> keys = parent->segmentationManager->performSegmentation(img);

    //cv::Mat keysFeatures((int)keys.size(), 5 + parent->featureExtractionManager->profileColumns, CV_32F);
    cv::Mat keysFeatures((int)keys.size(), parent->featureExtractionManager->profileColumns, CV_32F);
    for (int i = 0; i < (int)keys.size(); i++){
        std::vector<float> features = parent->featureExtractionManager->extractFeatures(keys[i]);
        for (int j = 0; j < (int)features.size(); j++) keysFeatures.at<float>(i, j) = features[j];
    }
    scaler->transform(keysFeatures);

    cv::Mat pred;
    SVMmodel->predict(keysFeatures, pred, ml::ROW_SAMPLE);
    return pred;
}

void Demo::updateFrame(){
    //Leemos el frame en RGB
    cap >> frame;

    if (autoclassifyFrame){
        disconnect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
        cv::Mat pred = predict(frame);
        parent->ui->demo_predictedKey_label->setText(QString("LLave %1").arg(QString::number(pred.at<float>(0, 0))));
        connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    } else parent->ui->demo_predictedKey_label->setText("");

    cvtColor(frame, frame, CV_BGR2RGB);
    //Mostramos el frame en el label
    parent->ui->demo_cameraDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (frame.data), frame.cols, frame.rows, QImage::Format_RGB888)));
}

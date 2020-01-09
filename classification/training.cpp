#include "training.h"

Training::Training(MainWindow* parent) : parent(parent) {}
Training::~Training() {
    delete scaler;
}

void Training::setup(){
    connect(parent->ui->training_loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->training_standarizeDataset_pushButton, SIGNAL(pressed()), this, SLOT(standarizeDataset_onClick()));
    connect(parent->ui->training_saveScaler_pushButton, SIGNAL(pressed()), this, SLOT(saveScaler_onClick()));
    connect(parent->ui->training_trainSVM_pushButton, SIGNAL(pressed()), this, SLOT(trainSVM_onClick()));
    connect(parent->ui->training_saveSVM_pushButton, SIGNAL(pressed()), this, SLOT(saveSVM_onClick()));

    //https://docs.opencv.org/ref/2.4/dd/d88/classCvSVM.html
    //https://stackoverflow.com/questions/14694810/using-opencv-and-svm-with-images
}

void Training::loadDataset_onClick(){
    QString datasetPath = parent->ui->training_datasetPath_lineEdit->text();
    dataset = loadDataset(datasetPath);
}

void Training::standarizeDataset_onClick(){
    scaler = new StandardScaler(dataset.x.cols);
    scaler->fit(dataset.x);
    scaler->transform(dataset.x);
}

void Training::saveScaler_onClick(){
    QString savePath = parent->ui->training_saveScalerPath_lineEdit->text();
    scaler->save(savePath.toStdString().c_str());
}

void Training::trainSVM_onClick(){
    SVMmodel = ml::SVM::create();
    SVMmodel->setType(ml::SVM::Types::C_SVC);
    SVMmodel->setC(0.7);
    SVMmodel->setKernel(ml::SVM::KernelTypes::LINEAR);
    SVMmodel->train(dataset.x, ml::ROW_SAMPLE, dataset.y);
}

void Training::saveSVM_onClick(){
    QString savePath = parent->ui->training_saveSVMpath_lineEdit->text();
    SVMmodel->save(savePath.toStdString().c_str());
}

Dataset Training::loadDataset(QString filepath){
    FILE* fid;
    char buf[1024];
    int n = 0;
    fid = fopen(filepath.toStdString().c_str(), "r");

    while (fgets(buf, 1024, fid)) n++;
    rewind(fid);

    Dataset d;
    //d.x = cv::Mat(n, 5 + parent->featureExtractionManager->profileColumns, CV_32F);
    d.x = cv::Mat(n, parent->featureExtractionManager->otherFeatures + parent->featureExtractionManager->profileColumns, CV_32F);
    d.y = cv::Mat(n, 1, CV_32S);

    int i = 0;
    while (fgets(buf, 1024, fid)){
        /*sscanf(buf, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            &(d.y.at<int>(i, 0)),
            &(d.x.at<float>(i, 0)), &(d.x.at<float>(i, 1)), &(d.x.at<float>(i, 2)), &(d.x.at<float>(i, 3)), &(d.x.at<float>(i, 4)),
            &(d.x.at<float>(i, 5)), &(d.x.at<float>(i, 6)), &(d.x.at<float>(i, 7)), &(d.x.at<float>(i, 8)), &(d.x.at<float>(i, 9)),
            &(d.x.at<float>(i, 10)), &(d.x.at<float>(i, 11)), &(d.x.at<float>(i, 12)), &(d.x.at<float>(i, 13)), &(d.x.at<float>(i, 14)),
            &(d.x.at<float>(i, 15)), &(d.x.at<float>(i, 16)), &(d.x.at<float>(i, 17)), &(d.x.at<float>(i, 18)), &(d.x.at<float>(i, 19)),
            &(d.x.at<float>(i, 20)), &(d.x.at<float>(i, 21)), &(d.x.at<float>(i, 22)), &(d.x.at<float>(i, 23)), &(d.x.at<float>(i, 24))
        );*/
        sscanf(buf, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            &(d.y.at<int>(i, 0)),
            &(d.x.at<float>(i, 0)), &(d.x.at<float>(i, 1)), &(d.x.at<float>(i, 2)), &(d.x.at<float>(i, 3)), &(d.x.at<float>(i, 4)),
            &(d.x.at<float>(i, 5)), &(d.x.at<float>(i, 6)), &(d.x.at<float>(i, 7)), &(d.x.at<float>(i, 8)), &(d.x.at<float>(i, 9)),
            &(d.x.at<float>(i, 10)), &(d.x.at<float>(i, 11)), &(d.x.at<float>(i, 12)), &(d.x.at<float>(i, 13))//, &(d.x.at<float>(i, 14)),
            //&(d.x.at<float>(i, 15)), &(d.x.at<float>(i, 16)), &(d.x.at<float>(i, 17)), &(d.x.at<float>(i, 18)), &(d.x.at<float>(i, 19))
        );
        i++;
    }
    fclose(fid);

    return d;
}


#include "training.h"

Training::Training(MainWindow* parent) : parent(parent) {}
Training::~Training() {}

void Training::setup(){
    connect(parent->ui->training_loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));

    //https://docs.opencv.org/ref/2.4/dd/d88/classCvSVM.html
    //https://stackoverflow.com/questions/14694810/using-opencv-and-svm-with-images
}

void Training::loadDataset_onClick(){
    QString datasetPath = parent->ui->training_datasetPath_lineEdit->text();
    dataset = loadDataset(datasetPath);

    std::cout << dataset.x;
    std::cout << dataset.y;
}

Dataset Training::loadDataset(QString filepath){
    FILE* fid;
    char buf[1024];
    int n = 0;
    fid = fopen(filepath.toStdString().c_str(), "r");

    while (fgets(buf, 1024, fid)) n++;
    rewind(fid);

    Dataset d;
    d.x = cv::Mat(n, 5 + parent->featureExtractionManager->profileColumns, CV_32FC1);
    d.y = cv::Mat(n, 1, CV_32FC1);

    int i = 0;
    while (fgets(buf, 1024, fid)){
        sscanf(buf, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            &(d.y.at<float>(i, 0)),
            &(d.x.at<float>(i, 0)), &(d.x.at<float>(i, 1)), &(d.x.at<float>(i, 2)), &(d.x.at<float>(i, 3)), &(d.x.at<float>(i, 4)),
            &(d.x.at<float>(i, 5)), &(d.x.at<float>(i, 6)), &(d.x.at<float>(i, 7)), &(d.x.at<float>(i, 8)), &(d.x.at<float>(i, 9)),
            &(d.x.at<float>(i, 10)), &(d.x.at<float>(i, 11)), &(d.x.at<float>(i, 12)), &(d.x.at<float>(i, 13)), &(d.x.at<float>(i, 14)),
            &(d.x.at<float>(i, 15)), &(d.x.at<float>(i, 16)), &(d.x.at<float>(i, 17)), &(d.x.at<float>(i, 18)), &(d.x.at<float>(i, 19)),
            &(d.x.at<float>(i, 20)), &(d.x.at<float>(i, 21)), &(d.x.at<float>(i, 22)), &(d.x.at<float>(i, 23)), &(d.x.at<float>(i, 24))
        );
        i++;
    }
    fclose(fid);

    return d;
}


#include "featureextraction.h"

FeatureExtraction::FeatureExtraction(MainWindow* parent) : parent(parent) {}
FeatureExtraction::~FeatureExtraction() {}

void FeatureExtraction::setup(){
    connect(parent->ui->featureextraction_loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->featureextraction_loadSegmentedImage_pushButton, SIGNAL(pressed()), this, SLOT(loadSegmentedImage_onClick()));
    connect(parent->ui->featureextraction_extractFeatures_pushButton, SIGNAL(pressed()), this, SLOT(extractFeatures_onClick()));
}

void FeatureExtraction::loadDataset_onClick(){
    parent->loadDatasetManager->loadDataset_onClick();

    //disconnect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT());
    parent->ui->featureextraction_class_comboBox->clear();
    for (int i = 0; i < parent->ui->loaddataset_foundClasses_comboBox->count(); i++){
        QString name = parent->ui->loaddataset_foundClasses_comboBox->itemText(i);
        QVariant data = parent->ui->loaddataset_foundClasses_comboBox->itemData(i);
        parent->ui->featureextraction_class_comboBox->addItem(name, data);
    }
    //connect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT());

    //disconnect(parent->ui->featureextraction_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT());
    parent->ui->featureextraction_image_comboBox->clear();
    for (int i = 0; i < parent->ui->loaddataset_foundImages_comboBox->count(); i++){
        QString name = parent->ui->loaddataset_foundImages_comboBox->itemText(i);
        QVariant data = parent->ui->loaddataset_foundImages_comboBox->itemData(i);
        parent->ui->featureextraction_image_comboBox->addItem(name, data);
    }
    //connect(parent->ui->featureextraction_image_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT());
}

void FeatureExtraction::loadSegmentedImage_onClick(){
    const char* image = QString("%1/%2/%3").arg(
        parent->loadDatasetManager->mainDir,
        parent->ui->featureextraction_class_comboBox->currentData().toString(),
        parent->ui->featureextraction_image_comboBox->currentData().toString()
    ).toStdString().c_str();

    //segmentedImage = TODO HACK
    segmentedImage = parent->segmentationManager->cropped;

    parent->ui->featureextraction_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (segmentedImage.data), segmentedImage.cols, segmentedImage.rows, segmentedImage.step, QImage::Format_Grayscale8)));
}

void FeatureExtraction::extractFeatures_onClick(){
    Features f = extractFeatures();
    extractedFeatures = selectFeatures(f);

    parent->ui->featureextraction_featureCenter_label->setText(QString("- Centro normalizado : (%1, %2)").arg(
        QString::number(extractedFeatures.at<float>(0, 3)),
        QString::number(extractedFeatures.at<float>(0, 4))
    ));
    parent->ui->featureextraction_featureArea_label->setText(QString("- Area : %1").arg(QString::number(extractedFeatures.at<float>(0, 2))));
    parent->ui->featureextraction_featureWidth_label->setText(QString("- Ancho : %1").arg(QString::number(extractedFeatures.at<float>(0, 0))));
    parent->ui->featureextraction_featureHeight_label->setText(QString("- Alto : %1").arg(QString::number(extractedFeatures.at<float>(0, 1))));

    cv::Mat testimg = segmentedImage.clone();
    cvtColor(testimg, testimg, CV_GRAY2RGB);
    cv::line(testimg, cv::Point(0, f.center.y), cv::Point(segmentedImage.cols - 1, f.center.y), cv::Scalar(0, 255, 0, 0), 1);
    cv::line(testimg, cv::Point(f.center.x, 0), cv::Point(f.center.x, segmentedImage.rows - 1), cv::Scalar(0, 255, 0, 0), 1);
    parent->ui->featureextraction_imageCenterDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (testimg.data), testimg.cols, testimg.rows, testimg.step, QImage::Format_RGB888)));
}

Features FeatureExtraction::extractFeatures(){
    //Variables para acceder a los datos de la imagen
    uchar* d = segmentedImage.data;
    uchar baseaddr = d[0];
    int xstep = segmentedImage.step[0];
    int ystep = segmentedImage.step[1];

    //Variables para obtener el varicentro de la llave
    int sumx = 0;
    int sumy = 0;
    int count = 0;

    //Variables para obtener el alto y el ancho
    int xmax = 0;
    int ymax = 0;
    int xmin = segmentedImage.cols - 1;
    int ymin = segmentedImage.rows - 1;

    //Variables para obtener el area
    int area = 0;

    for (int i = 0; i < segmentedImage.rows; i++){
        for (int j = 0; j < segmentedImage.cols; j++){
            if (d[baseaddr + xstep*i + ystep*j] > 0){
                //Calculo para el varicentro
                sumx += j;
                sumy += i;
                count++;

                //Calculo para el alto y el ancho
                if (i > ymax) ymax = i;
                if (i < ymin) ymin = i;
                if (j > xmax) xmax = j;
                if (j < xmin) xmin = j;

                //Calculo para el area
                area += 1;
            }
        }
    }

    //Construimos el struct y lo devolvemos
    Features f;
    f.height = ymax - ymin;
    f.width = xmax - xmin;
    f.area = area;
    f.center = cv::Point(sumx / count, sumy / count);
    return f;
}

cv::Mat FeatureExtraction::selectFeatures(Features f){   
    float centerx = (float)f.center.x / (float)f.width;
    float centery = (float)f.center.y / (float)f.height;

    cv::Mat features(1, 5, CV_32F);
    features.at<float>(0, 0) = (float)f.width;
    features.at<float>(0, 1) = (float)f.height;
    features.at<float>(0, 2) = (float)f.area;
    features.at<float>(0, 3) = (centerx < 0.5) ? (centerx / 0.5) : ((1 - centerx) / 0.5);
    features.at<float>(0, 4) = (centery < 0.5) ? (centery / 0.5) : ((1 - centery) / 0.5);

    return features;
}

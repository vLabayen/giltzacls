#include "featureextraction.h"

FeatureExtraction::FeatureExtraction(MainWindow* parent) : parent(parent) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");
}

FeatureExtraction::~FeatureExtraction() {}

void FeatureExtraction::setup(){
    connect(parent->ui->featureextraction_loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->featureextraction_loadSegmentedImage_pushButton, SIGNAL(pressed()), this, SLOT(loadSegmentedImage_onClick()));
    connect(parent->ui->featureextraction_extractFeatures_pushButton, SIGNAL(pressed()), this, SLOT(extractFeatures_onClick()));
    connect(parent->ui->featureextraction_exportCsv_pushButton, SIGNAL(pressed()), this, SLOT(exportCsv_onClick()));
    connect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
}

void FeatureExtraction::loadDataset_onClick(){
    disconnect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));

    parent->ui->featureextraction_class_comboBox->clear();
    parent->ui->featureextraction_image_comboBox->clear();

    getClassesResult rc = parent->loadDatasetManager->getClasses(parent->ui->featureextraction_class_comboBox);
    parent->loadDatasetManager->getImages(rc.dir, parent->ui->featureextraction_class_comboBox->currentData().toString(), parent->ui->featureextraction_image_comboBox);

    mainDir = rc.dir;

    connect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
}

void FeatureExtraction::selectedClass_onChange(int index){
    parent->ui->featureextraction_image_comboBox->clear();
    parent->loadDatasetManager->getImages(mainDir, parent->ui->featureextraction_class_comboBox->itemData(index).toString(), parent->ui->featureextraction_image_comboBox);
}

void FeatureExtraction::loadSegmentedImage_onClick(){
    QString image = QString("%1/%2/%3").arg(
        mainDir,
        parent->ui->featureextraction_class_comboBox->currentData().toString(),
        parent->ui->featureextraction_image_comboBox->currentData().toString()
    );

    cv::Mat rawImage = cv::imread(image.toStdString().c_str());
    segmentedKeys = parent->segmentationManager->performSegmentation(rawImage);

    disconnect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
    parent->ui->featureextraction_keys_comboBox->clear();
    for (int i = 0; i < (int)segmentedKeys.size(); i++){
        parent->ui->featureextraction_keys_comboBox->addItem(QString("LLave %1").arg(QString::number(i)));
    }
    connect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
    currentKeyIndex = parent->ui->featureextraction_keys_comboBox->currentIndex();

    parent->ui->featureextraction_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage(
        (const unsigned char*) (segmentedKeys[currentKeyIndex].data),
        segmentedKeys[currentKeyIndex].cols,
        segmentedKeys[currentKeyIndex].rows,
        segmentedKeys[currentKeyIndex].step,
        QImage::Format_Grayscale8
    )));
}

void FeatureExtraction::selectedKey_onChange(int index){
    currentKeyIndex = index;
    parent->ui->featureextraction_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage(
        (const unsigned char*) (segmentedKeys[currentKeyIndex].data),
        segmentedKeys[currentKeyIndex].cols,
        segmentedKeys[currentKeyIndex].rows,
        segmentedKeys[currentKeyIndex].step,
        QImage::Format_Grayscale8
    )));
}

void FeatureExtraction::extractFeatures_onClick(){
    std::vector<float> f = extractFeatures(segmentedKeys[currentKeyIndex]);
    //extractedFeatures = selectFeatures(f);

    /*parent->ui->featureextraction_featureCenter_label->setText(QString("- Centro normalizado : (%1, %2)").arg(
        QString::number(extractedFeatures.at<float>(0, 3)),
        QString::number(extractedFeatures.at<float>(0, 4))
    ));
    parent->ui->featureextraction_featureArea_label->setText(QString("- Area : %1").arg(QString::number(extractedFeatures.at<float>(0, 2))));
    parent->ui->featureextraction_featureWidth_label->setText(QString("- Ancho : %1").arg(QString::number(extractedFeatures.at<float>(0, 0))));
    parent->ui->featureextraction_featureHeight_label->setText(QString("- Alto : %1").arg(QString::number(extractedFeatures.at<float>(0, 1))));
    parent->ui->featureextraction_featureProfile_label->setText(QString("- Contorno : [%1, %2]").arg(
        QString("%1, %2, %3, %4, %5").arg(
            QString::number(extractedFeatures.at<float>(0, 5)),
            QString::number(extractedFeatures.at<float>(0, 6)),
            QString::number(extractedFeatures.at<float>(0, 7)),
            QString::number(extractedFeatures.at<float>(0, 8)),
            QString::number(extractedFeatures.at<float>(0, 9))
        ),
        QString("%1, %2, %3, %4, %5").arg(
            QString::number(extractedFeatures.at<float>(0, 10)),
            QString::number(extractedFeatures.at<float>(0, 11)),
            QString::number(extractedFeatures.at<float>(0, 12)),
            QString::number(extractedFeatures.at<float>(0, 13)),
            QString::number(extractedFeatures.at<float>(0, 14))
        )
    ));*/

    cv::Mat testimg = segmentedKeys[currentKeyIndex].clone();
    cvtColor(testimg, testimg, CV_GRAY2RGB);
    //cv::line(testimg, cv::Point(0, f.center.y), cv::Point(segmentedKeys[currentKeyIndex].cols - 1, f.center.y), cv::Scalar(0, 255, 0, 0), 1);
    //cv::line(testimg, cv::Point(f.center.x, 0), cv::Point(f.center.x, segmentedKeys[currentKeyIndex].rows - 1), cv::Scalar(0, 255, 0, 0), 1);
    parent->ui->featureextraction_imageCenterDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (testimg.data), testimg.cols, testimg.rows, testimg.step, QImage::Format_RGB888)));
}

void FeatureExtraction::exportCsv_onClick(){
    //TODO : PONER UN ERROR
    QString exportDir = parent->ui->featureextraction_exportCsv_lineEdit->text();
    if (!dir_exists(exportDir.toStdString().c_str())) return;
    FILE *fid = fopen(QString("%1/%2").arg(exportDir, "dataset.txt").toStdString().c_str(), "w");
    //fprintf(fid, "class,height,width,area,centerx,centery\n");

    for (int i = 0; i < parent->ui->featureextraction_class_comboBox->count(); i++){
        QString cls = parent->ui->featureextraction_class_comboBox->itemData(i).toString();
        if (cls == QString("fondo") || cls == QString("mix")) continue;
        parent->ui->featureextraction_image_comboBox->clear();
        parent->loadDatasetManager->getImages(mainDir, cls, parent->ui->featureextraction_image_comboBox);

        for (int j = 0; j < parent->ui->featureextraction_image_comboBox->count(); j++){
            QString image = QString("%1/%2/%3").arg(mainDir, cls, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
            cv::Mat rawImage = cv::imread(image.toStdString().c_str());
            std::vector<cv::Mat> keys = parent->segmentationManager->performSegmentation(rawImage);

            //Features f = extractFeatures(keys[0]);
            //cv::Mat feat = selectFeatures(f);
            std::vector<float> f = extractFeatures(keys[0]);

            fprintf(fid, "%s,%.0f,%.0f,%.0f,%.3f,%.3f", cls.toStdString().c_str(), f[0], f[1], f[2], f[3], f[4]);
            for (int c = 0; c < profileColumns; c++) fprintf(fid, ",%.3f", f[5 + c]);
            fprintf(fid, "\n");
        }
    }
    fclose(fid);
}

/*
Features FeatureExtraction::extractFeatures(cv::Mat src){
    //Variables para acceder a los datos de la imagen
    uchar* d = src.data;
    uchar baseaddr = d[0];
    int xstep = src.step[0];
    int ystep = src.step[1];

    //Variables para obtener el varicentro de la llave
    int sumx = 0;
    int sumy = 0;
    int count = 0;

    //Variables para obtener el alto y el ancho
    int xmax = 0;
    int ymax = 0;
    int xmin = src.cols - 1;
    int ymin = src.rows - 1;

    //Variables para obtener el area
    int area = 0;

    //Variables para obtener el contorno horizontal
    std::vector<float> hProfile(profileColumns, 0);
    int colWidth = ((src.cols % profileColumns) == 0) ? (int)(src.cols / profileColumns) : (int)(src.cols / profileColumns) + 1;
    int colArea = colWidth * src.rows;

    for (int i = 0; i < src.rows; i++){
        for (int j = 0; j < src.cols; j++){
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

                //Calculo del contorno
                hProfile[(int)(j / colWidth)] += 1;
            }
        }
    }
    for (int i = 0; i < profileColumns; i++) hProfile[i] = (float)hProfile[i] / (float)colArea;

    //Construimos el struct y lo devolvemos
    Features f;
    f.width = xmax - xmin;
    f.height = ymax - ymin;
    f.area = area;
    f.center = cv::Point(sumx / count, sumy / count);
    f.hProfile = hProfile;
    return f;
}*/

/*cv::Mat FeatureExtraction::selectFeatures(Features f){
    float centerx = (float)f.center.x / (float)f.width;
    float centery = (float)f.center.y / (float)f.height;
    if (centerx > 0.5) std::reverse(std::begin(f.hProfile), std::end(f.hProfile));

    cv::Mat features(1, 5 + profileColumns, CV_32F);
    features.at<float>(0, 0) = (float)f.width;
    features.at<float>(0, 1) = (float)f.height;
    features.at<float>(0, 2) = (float)f.area;
    features.at<float>(0, 3) = (centerx < 0.5) ? (centerx / 0.5) : ((1 - centerx) / 0.5);
    features.at<float>(0, 4) = (centery < 0.5) ? (centery / 0.5) : ((1 - centery) / 0.5);
    for (int i = 0; i < profileColumns; i++) features.at<float>(0, 5 + i) = f.hProfile[i];

    //std::copy(f.hProfile.begin(), f.hProfile.end(), std::ostream_iterator<float>(std::cout, " "));
    return features;
}*/

std::vector<float> FeatureExtraction::extractFeatures(cv::Mat src){
    //Variables para acceder a los datos de la imagen
    uchar* d = src.data;
    uchar baseaddr = d[0];
    int xstep = src.step[0];
    int ystep = src.step[1];

    //Variables para obtener el varicentro de la llave
    int sumx = 0;
    int sumy = 0;
    int count = 0;

    //Variables para obtener el alto y el ancho
    int xmax = 0;
    int ymax = 0;
    int xmin = src.cols - 1;
    int ymin = src.rows - 1;

    //Variables para obtener el area
    int area = 0;

    for (int i = 0; i < src.rows; i++){
        for (int j = 0; j < src.cols; j++){
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

    //Variables para obtener el contorno horizontal
    std::vector<float> hProfile(profileColumns, 0);
    int colWidth = ((src.cols % profileColumns) == 0) ? (int)(src.cols / profileColumns) : (int)(src.cols / profileColumns) + 1;
    int colArea = colWidth * src.rows;

    //Calculo del contorno
    if ((xmax - xmin) < ((float)src.cols / 2)){
        for (int i = 0; i < src.rows; i++)
            for (int j = 0; j < src.cols; j++)
                if (d[baseaddr + xstep*i + ystep*j] > 0)
                    hProfile[(int)(j / colWidth)] += 1;
    } else {
        for (int i = 0; i < src.rows; i++)
            for (int j = 0; j < src.cols; j++)
                if (d[baseaddr + xstep*i + ystep*(src.cols - 1 - j)] > 0)
                    hProfile[(int)(j / colWidth)] += 1;
    }

    //Construimos el vector y lo devolvemos
    std::vector<float> features(5 + profileColumns);

    features[0] = ymax - ymin;          //Altura
    features[1] = xmax - xmin;          //Anchura
    features[2] = area;                 //Area

    float centerx = ((float)sumx / (float)count) / src.cols;
    float centery = ((float)sumy / (float)count) / src.rows;
    features[3] = (centerx < 0.5) ? (centerx / 0.5) : ((1 - centerx) / 0.5);        //Centro en x
    features[4] = (centery < 0.5) ? (centery / 0.5) : ((1 - centery) / 0.5);        //Centro en y

    for (int i = 0; i < profileColumns; i++) features[5 + i] = (float)hProfile[i] / (float)colArea;     //Columnas
    return features;
}

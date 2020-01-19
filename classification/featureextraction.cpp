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
    connect(parent->ui->featureextraction_exportImages_pushButton, SIGNAL(pressed()), this, SLOT(exportImages_onClick()));
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
    segmentedKeys = parent->segmentationManager->performSegmentation(rawImage, showGrayscales).keys;

    disconnect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
    parent->ui->featureextraction_keys_comboBox->clear();
    for (int i = 0; i < (int)segmentedKeys.size(); i++){
        parent->ui->featureextraction_keys_comboBox->addItem(QString("LLave %1").arg(QString::number(i)));
    }
    connect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
    currentKeyIndex = parent->ui->featureextraction_keys_comboBox->currentIndex();

    if (currentKeyIndex >= 0){
        parent->ui->featureextraction_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage(
            (const unsigned char*) (segmentedKeys[currentKeyIndex].data),
            segmentedKeys[currentKeyIndex].cols,
            segmentedKeys[currentKeyIndex].rows,
            segmentedKeys[currentKeyIndex].step,
            (showGrayscales) ? QImage::Format_Grayscale8 : QImage::Format_RGB888
        )));
    }
}

void FeatureExtraction::selectedKey_onChange(int index){
    currentKeyIndex = index;
    parent->ui->featureextraction_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage(
        (const unsigned char*) (segmentedKeys[currentKeyIndex].data),
        segmentedKeys[currentKeyIndex].cols,
        segmentedKeys[currentKeyIndex].rows,
        segmentedKeys[currentKeyIndex].step,
        (showGrayscales) ? QImage::Format_Grayscale8 : QImage::Format_RGB888
    )));
}

void FeatureExtraction::extractFeatures_onClick(){
    std::vector<float> f = extractFeatures_v1(segmentedKeys[currentKeyIndex]);
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
    parent->ui->featureextraction_featureCenter_label->setText(QString("- Centro normalizado : (%1, %2)").arg(
        QString::number(f[3]), QString::number(f[4])
    ));
    parent->ui->featureextraction_featureWidth_label->setText(QString("- Ancho : %1").arg(QString::number(f[1])));
    parent->ui->featureextraction_featureHeight_label->setText(QString("- Alto : %1").arg(QString::number(f[0])));

    cv::Mat testimg = segmentedKeys[currentKeyIndex].clone();
    cvtColor(testimg, testimg, CV_GRAY2RGB);
    cv::line(testimg, cv::Point(0, f[4]), cv::Point(segmentedKeys[currentKeyIndex].cols - 1, f[4]), cv::Scalar(0, 255, 0, 0), 1);
    cv::line(testimg, cv::Point(f[3], 0), cv::Point(f[3], segmentedKeys[currentKeyIndex].rows - 1), cv::Scalar(0, 255, 0, 0), 1);


    //std::vector<float> feat = extractFeatures_v3(segmentedKeys[currentKeyIndex]);
    //int colWidth = ((segmentedKeys[currentKeyIndex].cols % profileColumns) == 0) ? (int)(segmentedKeys[currentKeyIndex].cols / profileColumns) : (int)(segmentedKeys[currentKeyIndex].cols / profileColumns) + 1;
    //printf("colWidth : %d\n", colWidth);
    //printf("Alto : %d\n", segmentedKeys[currentKeyIndex].rows);
    //for (int i = 0; i < 10; i++){
    //    printf("%d : %.3f --> %.3f\n", feat[4 + i] * colWidth * segmentedKeys[currentKeyIndex].rows, feat[4 + i]);
    //    if (i != 0) cv::line(testimg, cv::Point(colWidth * i, 0), cv::Point(colWidth * i , segmentedKeys[currentKeyIndex].rows - 1), cv::Scalar(0, 255, 0, 0), 1);
    //}

    parent->ui->featureextraction_imageCenterDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (testimg.data), testimg.cols, testimg.rows, testimg.step, QImage::Format_RGB888)));
}

void FeatureExtraction::exportCsv_onClick(){
    QString exportDir = parent->ui->featureextraction_exportCsv_lineEdit->text();

    //TODO : PONER UN ERROR
    if (!dir_exists(exportDir.toStdString().c_str())) return;
    QString exportFile = parent->ui->featureextraction_exportCsvFile_lineEdit->text();
    FILE *fid = fopen(QString("%1/%2").arg(exportDir, exportFile).toStdString().c_str(), "w");

    for (int i = 0; i < parent->ui->featureextraction_class_comboBox->count(); i++){
        QString cls = parent->ui->featureextraction_class_comboBox->itemData(i).toString();
        if (cls == QString("fondo") || cls == QString("mix")) continue;
        parent->ui->featureextraction_image_comboBox->clear();
        parent->loadDatasetManager->getImages(mainDir, cls, parent->ui->featureextraction_image_comboBox);

        for (int j = 0; j < parent->ui->featureextraction_image_comboBox->count(); j++){
            QString image = QString("%1/%2/%3").arg(mainDir, cls, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
            cv::Mat rawImage = cv::imread(image.toStdString().c_str());
            std::vector<cv::Mat> keys = parent->segmentationManager->performSegmentation(rawImage).keys;
            std::vector<float> f = extractFeatures(keys[0]);

            fprintf(fid, "%s", cls.toStdString().c_str());
            for (int k = 0; k < (int)f.size(); k++) fprintf(fid, ",%.3f", f[k]);
            fprintf(fid, "\n");
        }
    }
    fclose(fid);
}

void FeatureExtraction::exportImages_onClick(){
    QString exportDir = parent->ui->featureextraction_exportImages_lineEdit->text();
    if (!dir_exists(exportDir.toStdString().c_str())) return;

    for (int i = 0; i < parent->ui->featureextraction_class_comboBox->count(); i++){
        QString cls = parent->ui->featureextraction_class_comboBox->itemData(i).toString();
        if (cls == QString("fondo") || cls == QString("mix")) continue;
        parent->ui->featureextraction_image_comboBox->clear();
        parent->loadDatasetManager->getImages(mainDir, cls, parent->ui->featureextraction_image_comboBox);

        QString dstdir = QString("%1/%2").arg(exportDir, cls);
        if (!dir_exists(dstdir.toStdString().c_str())){
            if (!make_dir(dstdir.toStdString().c_str())) return;
        }

        for (int j = 0; j < parent->ui->featureextraction_image_comboBox->count(); j++){
            QString image = QString("%1/%2/%3").arg(mainDir, cls, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
            cv::Mat rawImage = cv::imread(image.toStdString().c_str());
            std::vector<cv::Mat> keys = parent->segmentationManager->performSegmentation(rawImage, showGrayscales).keys;

            if (keys.size() > 0){
                QString dstimage = QString("%1/%2").arg(dstdir, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
                cv::imwrite(dstimage.toStdString(), keys[0]);
            } else printf("Error en imagen : %s\n", image.toStdString().c_str());
        }
    }
}

std::vector<float> FeatureExtraction::extractFeatures_v1(cv::Mat src){
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

    //Construimos el vector y lo devolvemos
    //std::vector<float> features(5 + profileColumns);
    std::vector<float> features(5);

    features[0] = ymax - ymin;          //Altura
    features[1] = xmax - xmin;          //Anchura
    features[2] = area;                 //Area

    float centerx = ((float)sumx / (float)count) / src.cols;
    float centery = ((float)sumy / (float)count) / src.rows;
    features[3] = (centerx < 0.5) ? (centerx / 0.5) : ((1 - centerx) / 0.5);        //Centro en x
    features[4] = (centery < 0.5) ? (centery / 0.5) : ((1 - centery) / 0.5);        //Centro en y

    features[3] = (float)sumx / (float)count;        //Centro en x
    features[4] = (float)sumy / (float)count;        //Centro en y

    return features;
}

std::vector<float> FeatureExtraction::extractFeatures_v2(cv::Mat src){
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

    //Variables intermedias del centor en x e y. Se utiliza centerx para saber en que dirección recorrer la llave
    float centerx = ((float)sumx / (float)count) / src.cols;
    float centery = ((float)sumy / (float)count) / src.rows;

    //Calculo del contorno
    if (centerx <= 0.5){ //Nos aseguramos de empezar siempre por la cabeza de la llave
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
    features[3] = (centerx <= 0.5) ? (centerx / 0.5) : ((1 - centerx) / 0.5);        //Centro en x
    features[4] = (centery <= 0.5) ? (centery / 0.5) : ((1 - centery) / 0.5);        //Centro en y
    for (int i = 0; i < profileColumns; i++) features[5 + i] = (float)hProfile[i] / (float)colArea;     //Columnas

    return features;
}

std::vector<float> FeatureExtraction::extractFeatures_v3(cv::Mat src){
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

    //Variables intermedias del centor en x e y. Se utiliza centerx para saber en que dirección recorrer la llave
    float centerx = ((float)sumx / (float)count) / src.cols;
    float centery = ((float)sumy / (float)count) / src.rows;

    //Calculo del contorno
    if (centerx <= 0.5){ //Nos aseguramos de empezar siempre por la cabeza de la llave
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
    std::vector<float> features(4 + profileColumns);

    features[0] = (float)(ymax - ymin) / (float)area;           //Altura normalizada
    features[1] = (float)(xmax - xmin) / (float)area;           //Anchura normalizada
    features[2] = (centerx <= 0.5) ? (centerx / 0.5) : ((1 - centerx) / 0.5);        //Centro en x
    features[3] = (centery <= 0.5) ? (centery / 0.5) : ((1 - centery) / 0.5);        //Centro en y
    for (int i = 0; i < profileColumns; i++) features[4 + i] = (float)hProfile[i] / (float)colArea;     //Columnas

    return features;
}

std::vector<float> FeatureExtraction::extractFeatures(cv::Mat src){
    return extractFeatures_v3(src);
}

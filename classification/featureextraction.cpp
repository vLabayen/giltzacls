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
    //Desconectamos los callbacks ante cambios del combobox
    disconnect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));

    //Limpiamos el de clases e imágenes
    parent->ui->featureextraction_class_comboBox->clear();
    parent->ui->featureextraction_image_comboBox->clear();

    //Cargamos las clases e imágenes presentes en el directorio especificado en el tab : loadDataset
    getClassesResult rc = parent->loadDatasetManager->getClasses(parent->ui->featureextraction_class_comboBox);
    parent->loadDatasetManager->getImages(rc.dir, parent->ui->featureextraction_class_comboBox->currentData().toString(), parent->ui->featureextraction_image_comboBox);

    //Guardamos una referencia el directorio utilizado en el momento de carga de imágenes
    mainDir = rc.dir;

    //Volvemos a conectar el combobox
    connect(parent->ui->featureextraction_class_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
}

void FeatureExtraction::selectedClass_onChange(int index){
    //Limpiamos las imágenes de la clase anterior y cargamos las correspondientes
    parent->ui->featureextraction_image_comboBox->clear();
    parent->loadDatasetManager->getImages(mainDir, parent->ui->featureextraction_class_comboBox->itemData(index).toString(), parent->ui->featureextraction_image_comboBox);
}

void FeatureExtraction::loadSegmentedImage_onClick(){
    //Creamos la ruta completa a la imagen que queremos
    QString image = QString("%1/%2/%3").arg(
        mainDir,
        parent->ui->featureextraction_class_comboBox->currentData().toString(),
        parent->ui->featureextraction_image_comboBox->currentData().toString()
    );

    //Cargamos y segmentamos la imagen
    cv::Mat rawImage = cv::imread(image.toStdString().c_str());
    segmentedKeys = parent->segmentationManager->performSegmentation(rawImage, showGrayscales).keys;

    //Desconectamos el combobox de las llaves presentes y limpiamos su contenido
    disconnect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
    parent->ui->featureextraction_keys_comboBox->clear();

    //Añadimos las nuevas llaves encotradas
    for (int i = 0; i < (int)segmentedKeys.size(); i++){
        parent->ui->featureextraction_keys_comboBox->addItem(QString("LLave %1").arg(QString::number(i)));
    }

    //Reconctamos el combobox y guardamos una referencia del indice de la llave mostrada
    connect(parent->ui->featureextraction_keys_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedKey_onChange(int)));
    currentKeyIndex = parent->ui->featureextraction_keys_comboBox->currentIndex();

    //Si hay por lo menos una llave, la mostramos en el display
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
    //Actualizamos el indice de la llave y procedemos a mostrarla en el display
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
    //Extraemos las features correspondientes a la imagen mostrada
    std::vector<float> f = extractFeatures(segmentedKeys[currentKeyIndex]);

    //Mostramos los valores de las features, exceptuando las columnas
    parent->ui->featureextraction_featureCenter_label->setText(QString("- Centro normalizado : (%1, %2)").arg(
        QString::number(f[3]), QString::number(f[4])
    ));
    parent->ui->featureextraction_featureHeight_label->setText(QString("- Alto : %1").arg(QString::number(f[0])));
    parent->ui->featureextraction_featureWidth_label->setText(QString("- Ancho : %1").arg(QString::number(f[1])));

    //Generamos una copia de la imagen para poner líneas sobre ella y poder visualizar el centroide
    //Como este es invariante respecto al lado de la cabeza de la llave, se representará siempre sobre el mismo lado
    cv::Mat testing = segmentedKeys[currentKeyIndex].clone();
    cvtColor(testing, testing, CV_GRAY2RGB);

    //Generamos los puntos para dichas lineas
    cv::Point centroid = getCentroid(segmentedKeys[currentKeyIndex]);
    cv::Point leftPoint = cv::Point(0, centroid.y);
    cv::Point rightPoint = cv::Point(segmentedKeys[currentKeyIndex].cols - 1, centroid.y);
    cv::Point topPoint = cv::Point(centroid.x, 0);
    cv::Point bottomPoint = cv::Point(centroid.x, segmentedKeys[currentKeyIndex].rows - 1);

    //Creamos las lineas
    cv::line(testing, leftPoint, rightPoint, cv::Scalar(0, 255, 0, 0), 1);    //Linea horizontal
    cv::line(testing, topPoint, bottomPoint, cv::Scalar(0, 255, 0, 0), 1);    //Linea vertical

    //Mostramos la imagen
    parent->ui->featureextraction_imageCenterDisplay_label->setPixmap(QPixmap::fromImage(QImage(
        (const unsigned char*) (testing.data),
        testing.cols,
        testing.rows,
        testing.step,
        QImage::Format_RGB888
    )));
}

void FeatureExtraction::exportCsv_onClick(){
    //Obtenemos el directorio al que se quieren exportar las features
    //Comprobamos que dicho directorio exista
    //Obtenemos el nombre del fichero a crear y abrimos dicha ruta con permiso de escritura
    QString exportDir = parent->ui->featureextraction_exportCsv_lineEdit->text();
    if (!dir_exists(exportDir.toStdString().c_str())) return;
    QString exportFile = parent->ui->featureextraction_exportCsvFile_lineEdit->text();
    FILE *fid = fopen(QString("%1/%2").arg(exportDir, exportFile).toStdString().c_str(), "w");

    //Recorremos todas las clases presentes en el combox de tipos de llave
    for (int i = 0; i < parent->ui->featureextraction_class_comboBox->count(); i++){
        //Ignoramos los directorios de mix y fondo, pues o no son llaves o no tendremos el label de estas
        QString cls = parent->ui->featureextraction_class_comboBox->itemData(i).toString();
        if (cls == QString("fondo") || cls == QString("mix")) continue;

        //Vaciamos el contenido de las llaves y añadimos las de la clase en cuestion
        parent->ui->featureextraction_image_comboBox->clear();
        parent->loadDatasetManager->getImages(mainDir, cls, parent->ui->featureextraction_image_comboBox);

        //Para cada llave, extramos las features y las guardamos en el csv junto con su label como una linea independiente
        for (int j = 0; j < parent->ui->featureextraction_image_comboBox->count(); j++){
            QString image = QString("%1/%2/%3").arg(mainDir, cls, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
            cv::Mat rawImage = cv::imread(image.toStdString().c_str());
            std::vector<cv::Mat> keys = parent->segmentationManager->performSegmentation(rawImage).keys;

            if (keys.size() > 0) {                                  //Nos aseguramos de que hay por lo menos una llave
                std::vector<float> f = extractFeatures(keys[0]);    //Suponemos que solo hay una llave.

                fprintf(fid, "%s", cls.toStdString().c_str());
                for (int k = 0; k < (int)f.size(); k++) fprintf(fid, ",%.3f", f[k]);
                fprintf(fid, "\n");
            }
        }
    }
    fclose(fid);
}

void FeatureExtraction::exportImages_onClick(){
    //Obtenemos el directorio en el que se quieren guardar las imagenes y comprobamos que existe
    QString exportDir = parent->ui->featureextraction_exportImages_lineEdit->text();
    if (!dir_exists(exportDir.toStdString().c_str())) return;

    //Recorremos las clases de igual manera que en la funcion anterior
    for (int i = 0; i < parent->ui->featureextraction_class_comboBox->count(); i++){
        QString cls = parent->ui->featureextraction_class_comboBox->itemData(i).toString();
        if (cls == QString("fondo") || cls == QString("mix")) continue;
        parent->ui->featureextraction_image_comboBox->clear();
        parent->loadDatasetManager->getImages(mainDir, cls, parent->ui->featureextraction_image_comboBox);

        //Vemos si existe el directorio de la clase correspondiente, intentamos crearlo si no
        QString dstdir = QString("%1/%2").arg(exportDir, cls);
        if (!dir_exists(dstdir.toStdString().c_str())){
            if (!make_dir(dstdir.toStdString().c_str())) return;
        }

        //Recorremos para todas las llaves. Exportamos la imagen si se ha encontrado llave
        for (int j = 0; j < parent->ui->featureextraction_image_comboBox->count(); j++){
            QString image = QString("%1/%2/%3").arg(mainDir, cls, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
            cv::Mat rawImage = cv::imread(image.toStdString().c_str());
            std::vector<cv::Mat> keys = parent->segmentationManager->performSegmentation(rawImage, showGrayscales).keys;

            if (keys.size() > 0){   //Comprobamos que hay alguna llave y la guardamos suponiendo que solo es una
                QString dstimage = QString("%1/%2").arg(dstdir, parent->ui->featureextraction_image_comboBox->itemData(j).toString());
                cv::imwrite(dstimage.toStdString(), keys[0]);
            } else printf("Error en imagen : %s\n", image.toStdString().c_str());
        }
    }
}

//Estas funciones no son reutilizadas entre ellas para mejorar la velocidad a la hora de su utilizacion.
//Las versiones 1 y 2 se han mantenido únicamente para permitir la elaboración de la memoria
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

//Esta funcion solo tiene finalidad de permitir la representacion del centroide
cv::Point FeatureExtraction::getCentroid(cv::Mat src){
    //Variables para acceder a los datos de la imagen
    uchar* d = src.data;
    uchar baseaddr = d[0];
    int xstep = src.step[0];
    int ystep = src.step[1];

    //Variables para obtener el varicentro de la llave
    int sumx = 0;
    int sumy = 0;
    int count = 0;

    //Obtencion del centroide
    for (int i = 0; i < src.rows; i++){
        for (int j = 0; j < src.cols; j++){
            if (d[baseaddr + xstep*i + ystep*j] > 0){
                sumx += j;
                sumy += i;
                count++;
            }
        }
    }

    float centerx = (float)sumx / (float)count;
    float centery = (float)sumy / (float)count;
    return cv::Point(centerx, centery);
}

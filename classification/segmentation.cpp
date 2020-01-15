#include "segmentation.h"

Segmentation::Segmentation(MainWindow* parent) : parent(parent) {}
Segmentation::~Segmentation() {}

void Segmentation::setup(){
    //Connects y otras inicializaciones aqui
    connect(parent->ui->Segmentacion_botonSegmentar, SIGNAL(pressed()), this, SLOT(BotonSegmentarListener()));
    connect(parent->ui->foundImages_comboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectedIndexCrop(int)));
}

void Segmentation::BotonSegmentarListener(void){
    //Pruebas Watersheed (De momento lo dejo aparcado. La idea sería poder emplear los orificios de las llaves para los marcadores --> Pensar.)
    //watershedV3(parent->loadDatasetManager->selectedImage);
    //Primera segmentación
    imageThresholdedGeneral = thresholdingTrimmed(parent->loadDatasetManager->selectedImage);
    //Representación primera segmetación
    drawThresholdedImage(imageThresholdedGeneral);
    //Busqueda Bouding Box
    rotatedRect = findBoundingBox1(imageThresholdedGeneral);
    //Representación Bounding Box
    cv::Mat drawing = drawBoundingBox(rotatedRect, parent->loadDatasetManager->selectedImage.clone());
    representBoundigBox(drawing);
    //Listado de Fotos detectadas
    List_BoundingBox(rotatedRect);
}

performSegmentationResponse Segmentation::performSegmentation(cv::Mat srcImage, bool umbralized){
    cv::Mat imageThresholded = thresholdingTrimmed(srcImage);
    std::vector<cv::RotatedRect> rr = findBoundingBox1(imageThresholded);
    std::vector<cv::Mat> allKeysSegmented(rr.size());
    std::vector<cv::Point> labelsPoint(rr.size());
    performSegmentationResponse rPSR;
    cv::Point2f pts[4];
    for(int keyIndex = 0; keyIndex < rr.size(); keyIndex++){
           allKeysSegmented[keyIndex] =SecondthresholdingTrimmedV2(show_BoundingBoxOriented(keyIndex, rr, imageThresholded));
        if(umbralized == false){
            bitwise_and(show_BoundingBoxOriented(keyIndex, rr, srcImage),show_BoundingBoxOriented(keyIndex, rr, srcImage), allKeysSegmented[keyIndex]);
        }
        rr[keyIndex].points(pts); labelsPoint[keyIndex] = pts[0];
    }
    rPSR.keys = allKeysSegmented;
    rPSR.labelsPosition = labelsPoint;
    rPSR.unlabeledImage = drawBoundingBox(rr, srcImage);
    return rPSR;
}


cv::Mat Segmentation::thresholdingTrimmed(cv::Mat OriginalImage){
    cv::Mat imageThresholded;
    cv::Mat grad_x; cv::Mat grad_y;
    cvtColor(OriginalImage, imageThresholded, CV_BGR2GRAY);
    cv::Sobel( imageThresholded, grad_x, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_x, grad_x );
    Sobel( imageThresholded, grad_y, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_y, grad_y );
    imageThresholded = grad_x + grad_y + imageThresholded;
    threshold(imageThresholded, imageThresholded, 0, 255, cv::THRESH_OTSU);
    int sz1 = 2;
    cv::Mat ES1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz1-1, 2*sz1-1));
    morphologyEx(imageThresholded, imageThresholded, cv::MORPH_CLOSE, ES1);
    imageThresholded = limpiezaBordes(imageThresholded);
    //A partir de aqui es un cierre de huecos que da asco.
    cv::Mat imageThresholded1 = imageThresholded.clone();
    cv::floodFill(imageThresholded1, cv::Point(0,0), CV_RGB(255,255,255));
    bitwise_not(imageThresholded1, imageThresholded1);
    imageThresholded = (imageThresholded1 | imageThresholded);
    return imageThresholded;
}

void Segmentation::drawThresholdedImage(cv::Mat imageThresholded ){
    QImage qt_thresholded = QImage((const unsigned char*) (imageThresholded.data), imageThresholded.cols, imageThresholded.rows, imageThresholded.step, QImage::Format_Grayscale8);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(parent->ui->Segmentation_image1->width(), Qt::SmoothTransformation);
    parent->ui->Segmentation_image1->setPixmap(escaled);
    parent->ui->Segmentation_image1->resize(parent->ui->Segmentation_image1->pixmap()->size());


}

 std::vector<cv::RotatedRect> Segmentation::findBoundingBox1(cv::Mat imageThresholded){
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours( imageThresholded, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE  , cv::Point(0, 0));
    std::vector<cv::RotatedRect> box(contours.size());
    float maxAreaLocated = 0;
    for( size_t i = 0; i < contours.size(); i++ ){
        box[i] = minAreaRect(contours[i]);
        if(box[i].size.area() > maxAreaLocated){
           maxAreaLocated =  box[i].size.area();
        }
    }
    //Eliminamos aquellos rotatedRectangle parasitos si su area es menor que 1/3 del mayor detectado en la imagen.
    for( int i = contours.size()-1; i >= 0 ; i-- ){
        if(box[i].size.area() < maxAreaLocated/4){
            box.erase(box.begin()+i);
        }
    }
    return box;
}

 cv::Mat Segmentation::drawBoundingBox(std::vector<cv::RotatedRect> box, cv::Mat drawing){
     //Dibujar
     cv::Point2f vtx[4];
     for( size_t i = 0; i< box.size(); i++ ){
         box[i].points(vtx);
         for( int j = 0; j < 4; j++ ){
             line(drawing, vtx[j], vtx[(j+1)%4], cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
         }
     }
     return drawing;
 }

 void Segmentation::representBoundigBox(cv::Mat drawing){
     QImage qt_bounded = QImage((const unsigned char*) (drawing.data), drawing.cols, drawing.rows, drawing.step, QImage::Format_RGB888);
     QPixmap bounded = QPixmap::fromImage(qt_bounded).scaledToWidth(parent->ui->Segmentation_imageBig->width(), Qt::SmoothTransformation);
     parent->ui->Segmentation_imageBig->setPixmap(bounded);
     parent->ui->Segmentation_imageBig->resize(parent->ui->Segmentation_imageBig->pixmap()->size());
 }



void Segmentation::List_BoundingBox(std::vector<cv::RotatedRect> box ){
    disconnect(parent->ui->foundImages_comboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectedIndexCrop(int)));
    parent->ui->foundImages_comboBox_1->clear();
    for(int i = 0 ; i< (int)box.size(); i++){
        parent->ui->foundImages_comboBox_1->addItem(QString("Llave %1").arg(QString::number(i)), QVariant(i));
    }
    connect(parent->ui->foundImages_comboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectedIndexCrop(int)));
    onSelectedIndexCrop(parent->ui->foundImages_comboBox_1->currentIndex());
}

cv::Mat Segmentation::show_BoundingBoxOriented(int i, std::vector<cv::RotatedRect> rr, cv::Mat drawing){
    if(i >= 0){
        int diagonal = (int)sqrt(drawing.cols*drawing.cols+drawing.rows*drawing.rows);
        int newWidth = diagonal;
        int newHeight =diagonal;
        int offsetX = (newWidth - drawing.cols) / 2;
        int offsetY = (newHeight - drawing.rows) / 2;
        cv::Mat targetMat(newWidth, newHeight, drawing.type(), cv::Scalar(0,0,0));
        cv::Point2f src_center(targetMat.cols/2.0F, targetMat.rows/2.0F);
        float angle = rr[i].angle;
        cv::Size rect_size = rr[i].size;
        if(rect_size.width > rect_size.height){
            if((rr[i].center.x - rect_size.width) > 0){
               angle = 180+angle;
            }else{
                angle = -180+angle;
            }
        }else{
               angle += 90.0;
               cv::swap(rect_size.width, rect_size.height);
        }
        cv::Point2f correctedBoundingCenter(rr[i].center.x + offsetX, rr[i].center.y + offsetY );
        cv::Mat M = getRotationMatrix2D(correctedBoundingCenter, angle, 1.0);
        cv::Mat rotated, cropped;
        drawing.copyTo(targetMat(cv::Rect(offsetX, offsetY, drawing.cols, drawing.rows)));
        warpAffine(targetMat, rotated, M, targetMat.size(), cv::INTER_CUBIC);
        getRectSubPix(rotated, rect_size, correctedBoundingCenter, cropped);
        return cropped;
    }else{
        return drawing;
    }
}

cv::Mat Segmentation::limpiezaBordes(cv::Mat src){
    cv::Mat copiaSrc = src.clone();
    cv::Mat dstImg = copiaSrc > 127;
    rectangle(dstImg, Rect(0, 0, dstImg.cols, dstImg.rows), Scalar(255));
    floodFill(dstImg, cv::Point(0, 0), Scalar(0));
    copiaSrc = copiaSrc > 127;
    vector<vector<Point> > contours;
    findContours(copiaSrc, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++){
        Rect bounding_rect = boundingRect(contours[i]);
        Rect test_rect = bounding_rect & Rect(1, 1, copiaSrc.cols - 2, copiaSrc.rows - 2);
        if (bounding_rect != test_rect){
            drawContours(copiaSrc, contours, (int)i, Scalar(0),-1);
        }
    }
    return  copiaSrc;
}


void Segmentation::onSelectedIndexCrop(int i){
    //cv::Mat cropped = show_BoundingBoxOriented(i, rotatedRect, parent->loadDatasetManager->selectedImage.clone());
    cv::Mat cropped = show_BoundingBoxOriented(i, rotatedRect, imageThresholdedGeneral);
    cv::Mat finalKeySegmented = SecondthresholdingTrimmedV2(cropped); //<-- Cambio aquí
    drawThresholdedkey(finalKeySegmented);
}


cv::Mat Segmentation::SecondthresholdingTrimmed(cv::Mat ImageCropped){
    cv::Mat imageThresholded;
    cv::Mat grad_x; cv::Mat grad_y;
    cvtColor(ImageCropped, imageThresholded, CV_BGR2GRAY);
    cv::Sobel( imageThresholded, grad_x, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_x, grad_x );
    Sobel( imageThresholded, grad_y, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_y, grad_y );
    imageThresholded = grad_x + grad_y + imageThresholded;
    threshold(imageThresholded, imageThresholded, 0, 255, cv::THRESH_OTSU);
    int sz1 = 2;
    cv::Mat ES1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz1-1, 2*sz1-1));
    morphologyEx(imageThresholded, imageThresholded, cv::MORPH_CLOSE, ES1);
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours( imageThresholded, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
    std::vector<cv::Rect> boundRect( contours.size() );
    cv::RNG rng(12345);
    for( size_t i = 0; i< contours.size(); i++ ){
       approxPolyDP( contours[i], contours_poly[i], 1, true );
    }
    cv::Mat mask = cv::Mat::zeros(imageThresholded.rows, imageThresholded.cols, imageThresholded.type());
    for( size_t i = 0; i< contours.size(); i++ ){
       drawContours( mask, contours_poly, (int)i, 255, cv::FILLED);
    }
    return mask;
}

cv::Mat Segmentation::SecondthresholdingTrimmedV2(cv::Mat ImageCropped){
    //DOC: --> https://stackoverflow.com/questions/29108270/opencv-2-4-10-bwlabel-connected-components/30265609#30265609
    cv::Mat labels, stats, centroids;
    //Detectamos el numero de componentes conexas empleando una 8-conectividad. Obtenemos una imagen del mismo tamaño labeleada.
    int nLabels = connectedComponentsWithStats(ImageCropped, labels, stats, centroids, 8, CV_32S);
    //Detectamos cual es la región conexa de mayor area.
    int maxAreaLocated = 0;
    int indexMaxAreaLocated = 0;
    for(int i = 0; i <nLabels; i++){
        if(stats.at<int>(i, CC_STAT_AREA) > maxAreaLocated){
                maxAreaLocated = stats.at<int>(i, CC_STAT_AREA);
                indexMaxAreaLocated = i;
        }
    }
    //Filtramos todo aquellos que no sea la región conexa de mayor área.
    cv::Mat soloUnLabel;
    compare(labels, indexMaxAreaLocated, soloUnLabel, CMP_EQ);
    //Para evitar el ruido generado por la separación de componentes realizamos un and con la imagen original.
    soloUnLabel = soloUnLabel & ImageCropped;
    return soloUnLabel;
}



void Segmentation::drawThresholdedkey(cv::Mat mask){
    QImage qt_thresholded = QImage((const unsigned char*) (mask.data), mask.cols, mask.rows, mask.step, QImage::Format_Grayscale8);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(parent->ui->Segmentation_image2->width(), Qt::SmoothTransformation);
    parent->ui->Segmentation_image2->setPixmap(escaled);
    parent->ui->Segmentation_image2->resize(parent->ui->Segmentation_image2->pixmap()->size());
}



void Segmentation::watershedV2(cv::Mat src){
    cv::Mat imageFiltered;
    src.convertTo(src, CV_32F, 1.f/255);
    pow(src,0.3,src);
    src.convertTo(src, CV_32F, 1.f*255);
    src.convertTo(src, CV_8U);
    cv::pyrMeanShiftFiltering(src, imageFiltered, 21, 51);
    cvtColor(imageFiltered, imageFiltered, CV_BGR2GRAY);
    cv::Mat binaryImage;
    threshold(imageFiltered, binaryImage, 0, 255, cv::THRESH_OTSU);
    cv::Mat transformacionDistancia;
    cv::distanceTransform(binaryImage, transformacionDistancia, cv::DIST_L2, 3);
    cv::normalize(transformacionDistancia, transformacionDistancia, 1, 0, cv::NORM_INF);
    cv::Mat salida = transformacionDistancia.clone();
    cvtColor(salida, salida, CV_GRAY2RGB);
imshow("DistanceTransform",transformacionDistancia);
    cv::threshold(transformacionDistancia, transformacionDistancia, 0.5, 255, cv::THRESH_BINARY);
    cv::Mat E = cv::Mat::ones(9,9,CV_8U);//Generamos el elemento estructurante
imshow("DistanceTransformBinary", transformacionDistancia);
    cv::erode(transformacionDistancia,transformacionDistancia, E);
imshow("DistanceTransformBinaryAfterErode", transformacionDistancia);
    cv::Mat marcadores;
    transformacionDistancia.convertTo(transformacionDistancia, CV_8U, 1, 0);
    cv::connectedComponents(transformacionDistancia, marcadores);
    for (int i = 0; i < marcadores.rows; i++) {
        for (int j = 0; j < marcadores.cols; j++) {
            marcadores.ptr(i, j)[0] = marcadores.ptr(i, j)[0] + 1;
            if (transformacionDistancia.ptr(i, j)[0] == 255) {
                marcadores.ptr(i, j)[0] = 0;
            }
        }
    }

    cv::watershed(salida, marcadores);
    // draw barriers
    for (int i = 0; i < marcadores.rows; i++) {
        for (int j = 0; j < marcadores.cols; j++) {
            int index = marcadores.at<int>(i,j);
            if (index == -1) {
                salida.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,255);
            }
        }
    }
imshow("salida", salida);
}

void Segmentation::watershedV3(cv::Mat src){
    cv::Mat imageFiltered;
    src.convertTo(src, CV_32F, 1.f/255);
    pow(src,0.2,src);
    src.convertTo(src, CV_32F, 1.f*255);
    src.convertTo(src, CV_8U);
    cv::pyrMeanShiftFiltering(src, imageFiltered, 21, 51);
    cvtColor(imageFiltered, imageFiltered, CV_BGR2GRAY);
    cv::Mat salida = imageFiltered.clone();
    cvtColor(salida, salida, CV_GRAY2RGB);
    cv::Mat binaryImage;
    threshold(imageFiltered, binaryImage, 0, 255, cv::THRESH_OTSU);

    cv:: Mat complementariaPequena; cv:: Mat complementaria; cv::bitwise_not(binaryImage, complementaria);
    /*
    int sz1 = 15;
    cv::Mat complementariaOriginal = complementaria.clone();
    imshow("Complementatia0", complementaria);
    cv::Mat ES1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz1-1, 2*sz1-1));
    cv::dilate(complementaria, complementaria, ES1);
    imshow("Complementatia1", complementaria);
    cv::bitwise_not(complementaria, complementariaPequena);
imshow("ComplementatiaPequena", complementariaPequena);
    cv::Mat complementariafinal ; cv::bitwise_or(complementariaPequena,complementariaOriginal, complementariafinal);
    imshow("ComplementatiaFinal", complementariafinal);
    */
    //Erosion, Dilatacion, Dilatación OBTENCION DEL FONDO
    cv::Mat E = cv::Mat::ones(3,3,CV_8U);//Generamos el elemento estructurante
    cv::Mat apertura; cv::Mat fondoLlaves; cv::Mat transformacionDistancia; cv::Mat frontLLaves; cv::Mat resultadoResta;
imshow("ImagenBinaria", binaryImage);
    cv::erode(binaryImage,binaryImage, E);
imshow("Erosion", binaryImage);
    cv::dilate(binaryImage, apertura, E);
    cv::dilate(apertura, fondoLlaves, E, cv::Point(-1, -1), 7);
imshow("Apertura", apertura);
imshow("fondoLlaves", fondoLlaves);

    //TransformacionDistancia
    cv::distanceTransform(apertura, transformacionDistancia, DIST_L1, 3);
    cv::normalize(transformacionDistancia, transformacionDistancia, 1, 0, cv::NORM_INF);
imshow("transDistancia", transformacionDistancia);

    //Nos quedamos con el primer plano
    int sz1 = 17;
    cv::Mat ES1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz1-1, 2*sz1-1));
    morphologyEx(transformacionDistancia, transformacionDistancia, cv::MORPH_OPEN, ES1);
//cv::erode(transformacionDistancia,transformacionDistancia, ES1);
imshow("transDistancia1", transformacionDistancia);
    cv::threshold(transformacionDistancia, frontLLaves, 0.1, 255, cv::THRESH_BINARY);
    frontLLaves.convertTo(frontLLaves, CV_8U, 1, 0);
    cv::subtract(fondoLlaves, frontLLaves, resultadoResta);
imshow("frontLlaves", frontLLaves);
imshow("resultadoResta", resultadoResta);

    //Generamos marcadores para las componentes conexas
    cv::Mat marcadores;
    cv::connectedComponents(frontLLaves, marcadores);
    for (int i = 0; i < marcadores.rows; i++) {
        for (int j = 0; j < marcadores.cols; j++) {
            marcadores.ptr(i, j)[0] = marcadores.ptr(i, j)[0] + 1;
            if (resultadoResta.ptr(i, j)[0] == 255) {
                marcadores.ptr(i, j)[0] = 0;
            }
        }
    }
    cv::watershed(salida, marcadores);
    for (int i = 0; i < marcadores.rows; i++) {
        for (int j = 0; j < marcadores.cols; j++) {
            int index = marcadores.at<int>(i,j);
            if (index == -1) {
                salida.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,255);
            }
        }
    }
imshow("salida", salida);
}




void Segmentation::watershed(cv::Mat src){
    cv::Mat imageGray;
    cvtColor(src, imageGray, CV_BGR2GRAY); //Transdormamos la imagen original a escalada de grises
    cv::Mat tst;
    cv::bilateralFilter(imageGray,tst,31,31,75);
    //cv::medianBlur(tst, imageGray,15);

//imshow("Bilateral", tst);
    tst.convertTo(imageGray, CV_32F);
    imageGray.convertTo(imageGray, CV_32F, 1.f/255);
    pow(imageGray,0.3,imageGray);
    imageGray.convertTo(imageGray, CV_32F, 1.f*255);
    imageGray.convertTo(imageGray, CV_8U);
//imshow("Potencia", imageGray);
    //cv::medianBlur(imageGray, imageGray,11);//Filtro de mediana de 5x5


cv::Mat salida = imageGray.clone();
cvtColor(salida, salida, CV_GRAY2RGB);

    threshold(imageGray, imageGray, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU); //Calculamos el threshold con OTSU
    cv::Mat E = cv::Mat::ones(33,33,CV_8U);//Generamos el elemento estructurante
    //Erosion, Dilatacion, Dilatación
    //cv::erode(imageGray,imageGray, E);
    cv::Mat apertura; cv::Mat fondoLlaves; cv::Mat transformacionDistancia; cv::Mat frontLLaves; cv::Mat resultadoResta;
//imshow("image gray", imageGray);

cv::erode(imageGray,imageGray, E);
//imshow("erode", imageGray);
    cv::Mat E1 = cv::Mat::ones(35,35,CV_8U);//Generamos el elemento estructurante
    cv::dilate(imageGray, apertura, E1);
//apertura = imageGray;
//imshow("Apertura", apertura);
    cv::dilate(apertura, fondoLlaves, E, cv::Point(-1, -1), 7);
    cv::distanceTransform(apertura, transformacionDistancia, cv::DIST_L2, 3);
    cv::normalize(transformacionDistancia, transformacionDistancia, 1, 0, cv::NORM_INF);
//imshow("TransDistancia", transformacionDistancia);
    //Nos quedamos con el primer plano
    cv::threshold(transformacionDistancia, frontLLaves, 0.7, 255, cv::THRESH_BINARY);
    //cv::erode(frontLLaves,frontLLaves, E);
    frontLLaves.convertTo(frontLLaves, CV_8U, 1, 0);
imshow("FrontLlaves", frontLLaves);
imshow("FondoLlaves", fondoLlaves);
    cv::subtract(fondoLlaves, frontLLaves, resultadoResta);
imshow("resultadoResta", resultadoResta);
    //Generamos marcadores para las componentes conexas
    cv::Mat marcadores;
    cv::connectedComponents(frontLLaves, marcadores);
    for (int i = 0; i < marcadores.rows; i++) {
        for (int j = 0; j < marcadores.cols; j++) {
            marcadores.ptr(i, j)[0] = marcadores.ptr(i, j)[0] + 1;
            if (resultadoResta.ptr(i, j)[0] == 255) {
                marcadores.ptr(i, j)[0] = 0;
            }
        }
    }
    cv::watershed(salida, marcadores);
    // draw barriers
    for (int i = 0; i < marcadores.rows; i++) {
        for (int j = 0; j < marcadores.cols; j++) {
            int index = marcadores.at<int>(i,j);
            if (index == -1) {
                salida.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,255);
            }
        }
    }
imshow("salida", salida);

}



// Probablemente basura:
void Segmentation::show_BoundingBox(void){
    /*int i = parent->ui->foundImages_comboBox_1->currentData().toInt();
    cv::Mat cropped = parent->loadDatasetManager->selectedImage.rowRange(boundRect[i].y, boundRect[i].y + boundRect[i].height+2).colRange(boundRect[i].x, boundRect[i].x + boundRect[i].width+2);
    QImage qt_cropped = QImage((const unsigned char*) (cropped.data), cropped.cols, cropped.rows, cropped.step, QImage::Format_RGB888);
    QPixmap escaled = QPixmap::fromImage(qt_cropped).scaledToHeight(parent->ui->Segmentation_image2->height(), Qt::SmoothTransformation);
    parent->ui->Segmentation_image2->setPixmap(escaled);
    parent->ui->Segmentation_image2->resize(parent->ui->Segmentation_image2->pixmap()->size());
    cv::Mat KeySelectedThresholded = SecondthresholdingTrimmed(cropped);
    */
}

/*
void Segmentation::cropBoundingBox(std::vector<cv::Rect> boundRect ){
    for (int i = 0 ; i < (int)boundRect.size(); i++){
    imshow(QString("Img: %1").arg(QString::number(i)).toStdString(), parent->loadDatasetManager->selectedImage.rowRange(boundRect[i].y, boundRect[i].y + boundRect[i].height+1).colRange(boundRect[i].x, boundRect[i].x + boundRect[i].width+1));
    }
}
*/

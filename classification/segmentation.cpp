#include "segmentation.h"

Segmentation::Segmentation(MainWindow* parent) : parent(parent) {}
Segmentation::~Segmentation() {}

void Segmentation::setup(){
    //Connects y otras inicializaciones aqui
    connect(parent->ui->Segmentacion_botonSegmentar, SIGNAL(pressed()), this, SLOT(BotonSegmentarListener()));
    connect(parent->ui->foundImages_comboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectedIndexCrop(int)));
}

void Segmentation::BotonSegmentarListener(void){
    //Primera segmentación
    cv::Mat imageThresholded = thresholdingTrimmed(parent->loadDatasetManager->selectedImage);
    //Representación primera segmetación
    drawThresholdedImage(imageThresholded);
    //Busqueda Bouding Box
    rotatedRect = findBoundingBox1(imageThresholded);
    //Representación Bounding Box
    drawBoundingBox(rotatedRect);
    //Listado de Fotos detectadas
    List_BoundingBox(rotatedRect);
}

std::vector<cv::Mat> Segmentation::performSegmentation(cv::Mat srcImage){
    cv::Mat imageThresholded = thresholdingTrimmed(srcImage);
    std::vector<cv::RotatedRect> rr = findBoundingBox1(imageThresholded);
    std::vector<cv::Mat> allKeysSegmented;
    for(int keyIndex = 0; keyIndex < rr.size(); keyIndex++){
        allKeysSegmented[keyIndex] =SecondthresholdingTrimmed(show_BoundingBoxOriented(keyIndex, rr));
    }
    return allKeysSegmented;
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
    findContours( imageThresholded, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<cv::RotatedRect> box(contours.size());

    //cv::Mat tmp = drawing.clone();

    float maxAreaLocated = 0;
    for( size_t i = 0; i < contours.size(); i++ ){
        //approxPolyDP( contours[i], contours_poly[i], 3, true );
        box[i] = minAreaRect(contours[i]);
        if(box[i].size.area() > maxAreaLocated){
           maxAreaLocated =  box[i].size.area();
        }
        //char k=cv::waitKey();
        //cv::drawMarker(tmp, box[i].center, cv::Scalar(0, 0, 255, 0),cv::MARKER_CROSS, 60, 5, 8);
        //imshow("Ventana: Que cojones", tmp);
    }
    //Eliminamos aquellos rotatedRectangle parasitos si su area es menor que 1/3 del mayor detectado en la imagen.
    for( int i = contours.size()-1; i >= 0 ; i-- ){
        if(box[i].size.area() < maxAreaLocated/3){
            box.erase(box.begin()+i);
        }
    }
    return box;
}

 void Segmentation::drawBoundingBox(std::vector<cv::RotatedRect> box){

     //Dibujar
     cv::Point2f vtx[4];
     cv::Mat drawing = parent->loadDatasetManager->selectedImage.clone();
     for( size_t i = 0; i< box.size(); i++ ){
         box[i].points(vtx);
         for( int j = 0; j < 4; j++ ){
             line(drawing, vtx[j], vtx[(j+1)%4], cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
         }
        //cv::Scalar color = cv::Scalar( rng.uniform(0, 256),rng.uniform(0,256), rng.uniform(0,256) );
        //drawContours( drawing, contours_poly, (int)i, color );
     }
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

cv::Mat Segmentation::show_BoundingBoxOriented(int i, std::vector<cv::RotatedRect> rr){
    cv::Mat drawing = parent->loadDatasetManager->selectedImage.clone();
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
}

void Segmentation::onSelectedIndexCrop(int i){
    cv::Mat cropped = show_BoundingBoxOriented(i, rotatedRect);
    cv::Mat finalKeySegmented = SecondthresholdingTrimmed(cropped);
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
       //cv::Scalar color = cv::Scalar( rng.uniform(0, 256),rng.uniform(0,256), rng.uniform(0,256) );
       drawContours( mask, contours_poly, (int)i, 255, cv::FILLED);
    }

    //cv::Scalar color = cv::Scalar( rng.uniform(0, 256),rng.uniform(0,256), rng.uniform(0,256) );
    //drawContours( drawing, contours_poly, (int)i, color );
    /*
    //TODO: Repensar el llenado de huecos porque esto de aqui abajo da pena. Buscar un imfill en condiciones.
    cv::Mat imageThresholded1 = imageThresholded.clone();
    cv::floodFill(imageThresholded1, cv::Point((imageThresholded1.rows)/2,(imageThresholded1.cols)/2), cvScalar(255));
    //imshow("prueba", imageThresholded1);
    bitwise_not(imageThresholded1, imageThresholded1);
    //imshow("prueba1", imageThresholded1);
    imageThresholded = (imageThresholded1 | imageThresholded);
    //imshow("prueba2", imageThresholded);
    */

    return mask;
}

void Segmentation::drawThresholdedkey(cv::Mat mask){
    QImage qt_thresholded = QImage((const unsigned char*) (mask.data), mask.cols, mask.rows, mask.step, QImage::Format_Grayscale8);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(parent->ui->Segmentation_image2->width(), Qt::SmoothTransformation);
    parent->ui->Segmentation_image2->setPixmap(escaled);
    parent->ui->Segmentation_image2->resize(parent->ui->Segmentation_image2->pixmap()->size());
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


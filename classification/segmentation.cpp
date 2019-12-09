#include "segmentation.h"

Segmentation::Segmentation(MainWindow* parent) : parent(parent) {}
Segmentation::~Segmentation() {}

void Segmentation::setup(){
    //Connects y otras inicializaciones aqui
    connect(parent->ui->Segmentacion_botonSegmentar, SIGNAL(pressed()), this, SLOT(BotonSegmentarListener()));
    connect(parent->ui->foundImages_comboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(show_BoundingBox()));
}

void Segmentation::BotonSegmentarListener(void){
    thresholdingTrimmed();
    boundRect = findBoundingBox1();
    List_BoundingBox();
    //List_BoundingBox();
    //show_BoundingBox();
}

cv::Mat Segmentation::thresholdingTrimmed(){
    cv::Mat imageThresholded;
    cv::Mat grad_x; cv::Mat grad_y;
    cvtColor(parent->loadDatasetManager->selectedImage, imageThresholded, CV_BGR2GRAY);
    cv::Sobel( imageThresholded, grad_x, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_x, grad_x );
    Sobel( imageThresholded, grad_y, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_y, grad_y );
    imageThresholded = grad_x + grad_y + imageThresholded;
    threshold(imageThresholded, imageThresholded, 0, 255, cv::THRESH_OTSU);
    int sz1 = 2;
    cv::Mat ES1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz1-1, 2*sz1-1));
    morphologyEx(imageThresholded, imageThresholded, cv::MORPH_CLOSE, ES1);
    cv::Mat imageThresholded1 = imageThresholded.clone();
    cv::floodFill(imageThresholded1, cv::Point(0,0), CV_RGB(255,255,255));
    bitwise_not(imageThresholded1, imageThresholded1);
    imageThresholded = (imageThresholded1 | imageThresholded);
    QImage qt_thresholded = QImage((const unsigned char*) (imageThresholded.data), imageThresholded.cols, imageThresholded.rows, imageThresholded.step, QImage::Format_Grayscale8);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(parent->ui->Segmentation_image1->width(), Qt::SmoothTransformation);
    parent->ui->Segmentation_image1->setPixmap(escaled);
    parent->ui->Segmentation_image1->resize(parent->ui->Segmentation_image1->pixmap()->size());
    //List_BoundingBox();
    return imageThresholded;
}

std::vector<cv::Rect> Segmentation::findBoundingBox1(){
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat imageThresholded = thresholdingTrimmed();
    findContours( imageThresholded, contours, hierarchy, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
    std::vector<cv::Rect> boundRect( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ ){
        approxPolyDP( contours[i], contours_poly[i], 3, true );
        boundRect[i] = boundingRect( contours_poly[i] );
    }
    cv::Mat drawing = parent->loadDatasetManager->selectedImage.clone();
    for( size_t i = 0; i< contours.size(); i++ ){
        cv::Scalar color = cv::Scalar(255,0,0);
        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 1 );
    }
    parent->ui->Segmentation_imageBig->raise();
    QImage qt_thresholded = QImage((const unsigned char*) (drawing.data), drawing.cols, drawing.rows, drawing.step, QImage::Format_RGB888);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(parent->ui->Segmentation_imageBig->width(), Qt::SmoothTransformation);
    parent->ui->Segmentation_imageBig->setPixmap(escaled);
    parent->ui->Segmentation_imageBig->resize(parent->ui->Segmentation_imageBig->pixmap()->size());
    return boundRect;
}
void Segmentation::cropBoundingBox(std::vector<cv::Rect> boundRect ){
    for (int i = 0 ; i < (int)boundRect.size(); i++){
    imshow(QString("Img: %1").arg(QString::number(i)).toStdString(), parent->loadDatasetManager->selectedImage.rowRange(boundRect[i].y, boundRect[i].y + boundRect[i].height+1).colRange(boundRect[i].x, boundRect[i].x + boundRect[i].width+1));
    }
}

void Segmentation::List_BoundingBox(void){
    parent->ui->foundImages_comboBox_1->clear();
    for(int i = 0 ; i< (int)boundRect.size(); i++){
        parent->ui->foundImages_comboBox_1->addItem(QString("Llave %1").arg(QString::number(i)), QVariant(i));
    }
}

void Segmentation::show_BoundingBox(void){
    int i = parent->ui->foundImages_comboBox_1->currentData().toInt();
    cv::Mat cropped = parent->loadDatasetManager->selectedImage.rowRange(boundRect[i].y, boundRect[i].y + boundRect[i].height+2).colRange(boundRect[i].x, boundRect[i].x + boundRect[i].width+2);
    QImage qt_cropped = QImage((const unsigned char*) (cropped.data), cropped.cols, cropped.rows, cropped.step, QImage::Format_RGB888);
    QPixmap escaled = QPixmap::fromImage(qt_cropped).scaledToHeight(parent->ui->Segmentation_image2->height(), Qt::SmoothTransformation);
    parent->ui->Segmentation_image2->setPixmap(escaled);
    parent->ui->Segmentation_image2->resize(parent->ui->Segmentation_image2->pixmap()->size());
    cv::Mat KeySelectedThresholded = SecondthresholdingTrimmed(cropped);
    //RefindBoundingBox(KeySelectedThresholded);
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
    //TODO: Repensar el llenado de huecos porque esto de aqui abajo da pena. Buscar un imfill en condiciones.
    /*cv::Mat imageThresholded1 = imageThresholded.clone();
    cv::floodFill(imageThresholded1, cv::Point(0,0), cvScalar(255));
    imshow("prueba", imageThresholded1);
    bitwise_not(imageThresholded1, imageThresholded1);
    //imshow("prueba1", imageThresholded1);
    imageThresholded = (imageThresholded1 | imageThresholded);
    */
    /*
    QImage qt_thresholded = QImage((const unsigned char*) (imageThresholded.data), imageThresholded.cols, imageThresholded.rows, imageThresholded.step, QImage::Format_Grayscale8);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(ui->Segmentation_image1->width(), Qt::SmoothTransformation);
    ui->Segmentation_image1->setPixmap(escaled);
    ui->Segmentation_image1->resize(ui->Segmentation_image1->pixmap()->size());
    */
    return imageThresholded;
}
void Segmentation::RefindBoundingBox(cv::Mat KeySelectedThresholded){
    imshow("a", KeySelectedThresholded);
    //imwrite( "/home/jesus/Imágenes/tmp/tmp.png", KeySelectedThresholded );
    cv::RNG rng(12345);
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours( KeySelectedThresholded, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
    std::vector<cv::Rect> boundRect( contours.size() );
    std::vector<cv::Point2f>centers( contours.size() );
    std::vector<float>radius( contours.size());
    cv::Mat drawing = cv::Mat::zeros( KeySelectedThresholded.size(), CV_8UC3 );
    cvtColor(KeySelectedThresholded, KeySelectedThresholded, CV_GRAY2RGB);
        for( size_t i = 0; i < contours.size(); i++ ){
        approxPolyDP( contours[i], contours_poly[i], 3, true );
        boundRect[i] = boundingRect( contours_poly[i] );
        minEnclosingCircle( contours_poly[i], centers[i], radius[i] );
    }
        for( size_t i = 0; i< contours.size(); i++ )
           {
               cv::Scalar color = cv::Scalar( rng.uniform(0, 256),rng.uniform(0,256), rng.uniform(0,256) );
               drawContours( drawing, contours_poly, (int)i, color );
               rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2 );
               //circle( drawing, centers[i], (int)radius[i], color, 2 );
           }
           imshow( "Contours", drawing );


}


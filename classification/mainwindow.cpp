#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    captureManager(new Capture(this)),
    loadDatasetManager(new LoadDataset(this)),
    segmentationManager(new Segmentation(this)),
    trainingManager(new Training(this)),
    evaluationManager(new Evaluation(this)),
    demoManager(new Demo(this))
{
    ui->setupUi(this);
    captureManager->setup();
    loadDatasetManager->setup();
    segmentationManager->setup();
    trainingManager->setup();
    evaluationManager->setup();
    demoManager->setup();

<<<<<<< HEAD
    connect(ui->loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(ui->previewImage_pushButton, SIGNAL(pressed()), this, SLOT(previewImage_onClick()));
    connect(ui->previewImage_pushButton1, SIGNAL(pressed()), this, SLOT(show_BoundingBox()));
    connect(ui->foundImages_comboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(show_BoundingBox()));
=======
>>>>>>> master


}

MainWindow::~MainWindow()
{
    delete ui;
}

<<<<<<< HEAD
void MainWindow::loadDataset_onClick(){
    QString mainDir = ui->datasetPath_lineEdit->text();

    int classes = get_dirs(mainDir.toStdString().c_str(), ui->foundClasses_comboBox);
    ui->foundClasses_label->setText(QString("Clases encontradas : %1").arg(QString::number(classes)));

    QString currentDir = QString("%1/%2").arg(mainDir, ui->foundClasses_comboBox->currentData().toString());

    int images = get_files(currentDir.toStdString().c_str(), ui->foundImages_comboBox);
    ui->foundImages_label->setText(QString("Imagenes encontradas : %1").arg(QString::number(images)));
}

void MainWindow::selectedClass_onChange(int index){
    QString currentDir = QString("%1/%2").arg(ui->datasetPath_lineEdit->text(), ui->foundClasses_comboBox->itemData(index).toString());

    int images = get_files(currentDir.toStdString().c_str(), ui->foundImages_comboBox);
    ui->foundImages_label->setText(QString("Imagenes encontradas : %1").arg(QString::number(images)));
}

void MainWindow::previewImage_onClick(){
    QString selectedImage = QString("%1/%2/%3").arg(ui->datasetPath_lineEdit->text(), ui->foundClasses_comboBox->currentData().toString(), ui->foundImages_comboBox->currentData().toString());

    cv::Mat image = cv::imread(selectedImage.toStdString());
    cvtColor(image, image, CV_BGR2RGB);
    imageSelected = image.clone();

    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));

    ui->imageInfoRows_label->setText(QString("Filas : %1").arg(QString::number(image.rows)));
    ui->imageInfoCols_label->setText(QString("Columnas : %1").arg(QString::number(image.cols)));
    //cropBoundingBox(findBoundingBox1());
    List_BoundingBox();
}
void MainWindow::showHSVplanes(){
       cv::Mat hsvimage;
       cvtColor(imageSelected, hsvimage, CV_RGB2HSV);
       std::vector<cv::Mat> channels;
       split(hsvimage, channels);
       QImage qt_imageH = QImage((const unsigned char*) (channels[0].data), channels[0].cols, channels[0].rows, QImage::Format_Grayscale8);
       ui->Segmentation_image1->setPixmap(QPixmap::fromImage(qt_imageH));
       QImage qt_imageS = QImage((const unsigned char*) (channels[1].data), channels[1].cols, channels[1].rows, QImage::Format_Grayscale8);
       ui->Segmentation_image2->setPixmap(QPixmap::fromImage(qt_imageS));
       QImage qt_imageV = QImage((const unsigned char*) (channels[2].data), channels[2].cols, channels[2].rows, QImage::Format_Grayscale8);
       //ui->Segmentation_image3->setPixmap(QPixmap::fromImage(qt_imageV));
}
void MainWindow::showHist(){
        cv::Mat grayImage;
        cvtColor(imageSelected, grayImage, CV_BGR2HSV);
       int hbins = 255; int histSize[] = {hbins};
        float hranges[] = { 0, 255 };
        const float* ranges[] = {hranges};
        cv::Mat hist;
        int channels[] = {0};
        calcHist(&grayImage, 1, channels, cv::Mat(), hist, 1, histSize, ranges, true, false);
        double max_val=0;
        minMaxLoc(hist, 0, &max_val);
        int const hist_height = 256;
        cv::Mat3b histImg = cv::Mat3b::zeros(hist_height, hbins);
            // visualize each bin
        //std::cout<<histImg;
            for(int b = 0; b < hbins; b++) {
                float const binVal = hist.at<float>(b);
                int   const height = cvRound(binVal*hist_height/max_val);
                cv::line
                    ( histImg
                    , cv::Point(b, hist_height-height), cv::Point(b, hist_height)
                    , cv::Scalar::all(255)
                    );
            }
        QImage qt_Hist = QImage((const unsigned char*) (histImg.data), histImg.cols, histImg.rows, histImg.step, QImage::Format_RGB888);
        //ui->Segmentation_image4->heightForWidth(histImg.rows/histImg.cols);
        //ui->Segmentation_image4->setPixmap(QPixmap::fromImage(qt_Hist));
        //ui->Segmentation_image4->resize(ui->Segmentation_image4->pixmap()->size());

}

cv::Mat MainWindow::thresholding(){
    cv::Mat imageThresholded;
    cvtColor(imageSelected, imageThresholded, CV_BGR2GRAY);
    //int type = imageThresholded.type();
    //std::cout<<type;
    //imageThresholded.convertTo(imageThresholded, CV_32F);
    //pow(imageThresholded,0.7,imageThresholded);
    //imageThresholded.convertTo(imageThresholded, CV_8U);
    int sz = 50;
    cv::Mat ES = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz-1, 2*sz-1));
    //std::cout << ES;
   // cv::Mat ES = getStructuringElement(cv::MORPH_RECT, cv::Size(20,20), cv::Point(-1,-1));
    morphologyEx(imageThresholded, imageThresholded, cv::MORPH_TOPHAT, ES);
    threshold(imageThresholded, imageThresholded, 0, 255, cv::THRESH_OTSU);
    int sz1 = 2;
    cv::Mat ES1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*sz1-1, 2*sz1-1));
    morphologyEx(imageThresholded, imageThresholded, cv::MORPH_CLOSE, ES1);

    cv::Mat imageThresholded1 = imageThresholded.clone();
        cv::floodFill(imageThresholded1, cv::Point(0,0), CV_RGB(255,255,255));
        bitwise_not(imageThresholded1, imageThresholded1);
        imageThresholded = (imageThresholded1 | imageThresholded);

    ui->Segmentation_imageBig->raise();
    QImage qt_thresholded = QImage((const unsigned char*) (imageThresholded.data), imageThresholded.cols, imageThresholded.rows, imageThresholded.step, QImage::Format_Grayscale8);
    ui->Segmentation_imageBig->setPixmap(QPixmap::fromImage(qt_thresholded));
    ui->Segmentation_imageBig->resize(ui->Segmentation_imageBig->pixmap()->size());
    return imageThresholded;

}
cv::Mat MainWindow::thresholdingTrimmed(){
    cv::Mat imageThresholded;
    cv::Mat grad_x; cv::Mat grad_y;
    cvtColor(imageSelected, imageThresholded, CV_BGR2GRAY);
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
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(ui->Segmentation_image1->width(), Qt::SmoothTransformation);
    ui->Segmentation_image1->setPixmap(escaled);
    ui->Segmentation_image1->resize(ui->Segmentation_image1->pixmap()->size());
    return imageThresholded;
}

void MainWindow::findBoundingBox(){
    cv::RNG rng(12345);
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat imageThresholded = thresholdingTrimmed();
    findContours( imageThresholded, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
    std::vector<cv::Rect> boundRect( contours.size() );
    std::vector<cv::Point2f>centers( contours.size() );
    std::vector<float>radius( contours.size());
    int area = 0;
    int idx;
    for(int i=0; i<contours.size();i++) {
        if(area < contours[i].size())
            idx = i;
    }
    //cv::Mat drawing = cv::Mat::zeros( imageThresholded.size(), CV_8UC3 );
    cvtColor(imageThresholded, imageThresholded, CV_GRAY2RGB);
    approxPolyDP( contours[idx], contours_poly[idx], 3, true );
    boundRect[idx] = boundingRect( contours_poly[idx] );
    //drawContours( drawing, contours_poly, (int)idx, cv::Scalar(255,0,0) );
    rectangle( imageThresholded, boundRect[idx].tl(), boundRect[idx].br(), cv::Scalar(0,0,255), 2 );
      imshow( "Contours", imageThresholded );
    /*
        for( size_t i = 0; i < contours.size(); i++ ){
        approxPolyDP( contours[i], contours_poly[i], 3, true );
        boundRect[i] = boundingRect( contours_poly[i] );
        //minEnclosingCircle( contours_poly[i], centers[i], radius[i] );
    }
        cv::Mat drawing = cv::Mat::zeros( imageThresholded.size(), CV_8UC3 );
        for( size_t i = 0; i< contours.size(); i++ )
           {
               cv::Scalar color = cv::Scalar( rng.uniform(0, 256),rng.uniform(0,256), rng.uniform(0,256) );
               drawContours( drawing, contours_poly, (int)i, color );
               rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2 );
               //circle( drawing, centers[i], (int)radius[i], color, 2 );
           }
           imshow( "Contours", drawing );
*/

}
std::vector<cv::Rect> MainWindow::findBoundingBox1(){
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
    cv::Mat drawing = imageSelected.clone();
    for( size_t i = 0; i< contours.size(); i++ ){
        cv::Scalar color = cv::Scalar(255,0,0);
        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 1 );
    }
    ui->Segmentation_imageBig->raise();
    QImage qt_thresholded = QImage((const unsigned char*) (drawing.data), drawing.cols, drawing.rows, drawing.step, QImage::Format_RGB888);
    QPixmap escaled = QPixmap::fromImage(qt_thresholded).scaledToWidth(ui->Segmentation_imageBig->width(), Qt::SmoothTransformation);
    ui->Segmentation_imageBig->setPixmap(escaled);
    ui->Segmentation_imageBig->resize(ui->Segmentation_imageBig->pixmap()->size());
    return boundRect;
}

void MainWindow::cropBoundingBox(std::vector<cv::Rect> boundRect ){
    for (int i = 0 ; i < (int)boundRect.size(); i++){
    imshow(QString("Img: %1").arg(QString::number(i)).toStdString(), imageSelected.rowRange(boundRect[i].y, boundRect[i].y + boundRect[i].height+1).colRange(boundRect[i].x, boundRect[i].x + boundRect[i].width+1));
    }
}

void MainWindow::List_BoundingBox(void){
    boundRect = findBoundingBox1();
    ui->foundImages_comboBox_1->clear();
    for(int i = 0 ; i< (int)boundRect.size(); i++){
        ui->foundImages_comboBox_1->addItem(QString("Llave %1").arg(QString::number(i)), QVariant(i));
    }
}

void MainWindow::show_BoundingBox(void){
    int i = ui->foundImages_comboBox_1->currentData().toInt();
    cv::Mat cropped = imageSelected.rowRange(boundRect[i].y, boundRect[i].y + boundRect[i].height+2).colRange(boundRect[i].x, boundRect[i].x + boundRect[i].width+2);
    QImage qt_cropped = QImage((const unsigned char*) (cropped.data), cropped.cols, cropped.rows, cropped.step, QImage::Format_RGB888);
    QPixmap escaled = QPixmap::fromImage(qt_cropped).scaledToHeight(ui->Segmentation_image2->height(), Qt::SmoothTransformation);
    ui->Segmentation_image2->setPixmap(escaled);
    ui->Segmentation_image2->resize(ui->Segmentation_image2->pixmap()->size());
    cv::Mat KeySelectedThresholded = SecondthresholdingTrimmed(cropped);
    RefindBoundingBox(KeySelectedThresholded);
}

cv::Mat MainWindow::SecondthresholdingTrimmed(cv::Mat ImageCropped){
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

void MainWindow::RefindBoundingBox(cv::Mat KeySelectedThresholded){
    imshow("a", KeySelectedThresholded);
    imwrite( "/home/jesus/Imágenes/tmp/tmp.png", KeySelectedThresholded );
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

=======
>>>>>>> master

}

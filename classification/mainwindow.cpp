#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(ui->previewImage_pushButton, SIGNAL(pressed()), this, SLOT(previewImage_onClick()));
    /*connect(ui->saveFrame_pushButton, SIGNAL(pressed()), this, SLOT(saveFrame_onClick()));
    connect(ui->autocapture_checkBox, SIGNAL(toggled(bool)), this, SLOT(autoCaptureShot()));
    */

}

MainWindow::~MainWindow()
{
    delete ui;
}

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

    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));

    ui->imageInfoRows_label->setText(QString("Filas : %1").arg(QString::number(image.rows)));
    ui->imageInfoCols_label->setText(QString("Columnas : %1").arg(QString::number(image.cols)));
}

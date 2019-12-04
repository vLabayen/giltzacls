#include "loaddataset.h"

LoadDataset::LoadDataset(MainWindow* parent) : parent(parent) {}
LoadDataset::~LoadDataset() {}

void LoadDataset::setup(){
    connect(parent->ui->loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->previewImage_pushButton, SIGNAL(pressed()), this, SLOT(previewImage_onClick()));
}

void LoadDataset::loadDataset_onClick(){
    QString mainDir = parent->ui->datasetPath_lineEdit->text();

    int classes = get_dirs(mainDir.toStdString().c_str(), parent->ui->foundClasses_comboBox);
    parent->ui->foundClasses_label->setText(QString("Clases encontradas : %1").arg(QString::number(classes)));

    QString currentDir = QString("%1/%2").arg(mainDir, parent->ui->foundClasses_comboBox->currentData().toString());

    int images = get_files(currentDir.toStdString().c_str(), parent->ui->foundImages_comboBox);
    parent->ui->foundImages_label->setText(QString("Imagenes encontradas : %1").arg(QString::number(images)));
}

void LoadDataset::selectedClass_onChange(int index){
    QString currentDir = QString("%1/%2").arg(parent->ui->datasetPath_lineEdit->text(), parent->ui->foundClasses_comboBox->itemData(index).toString());

    int images = get_files(currentDir.toStdString().c_str(), parent->ui->foundImages_comboBox);
    parent->ui->foundImages_label->setText(QString("Imagenes encontradas : %1").arg(QString::number(images)));
}

void LoadDataset::previewImage_onClick(){
    QString selectedImage = QString("%1/%2/%3").arg(parent->ui->datasetPath_lineEdit->text(), parent->ui->foundClasses_comboBox->currentData().toString(), parent->ui->foundImages_comboBox->currentData().toString());

    cv::Mat image = cv::imread(selectedImage.toStdString());
    cvtColor(image, image, CV_BGR2RGB);

    QImage qt_image = QImage((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    parent->ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));

    parent->ui->imageInfoRows_label->setText(QString("Filas : %1").arg(QString::number(image.rows)));
    parent->ui->imageInfoCols_label->setText(QString("Columnas : %1").arg(QString::number(image.cols)));
}

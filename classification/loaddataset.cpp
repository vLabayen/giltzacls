#include "loaddataset.h"

LoadDataset::LoadDataset(MainWindow* parent) : parent(parent) {}
LoadDataset::~LoadDataset() {}

void LoadDataset::setup(){
    connect(parent->ui->loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::loadDataset_onClick(){
    disconnect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    disconnect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    QString mainDir = parent->ui->datasetPath_lineEdit->text();

    int classes = get_dirs(mainDir.toStdString().c_str(), parent->ui->foundClasses_comboBox);
    parent->ui->foundClasses_label->setText(QString("Clases encontradas : %1").arg(QString::number(classes)));

    QString currentDir = QString("%1/%2").arg(mainDir, parent->ui->foundClasses_comboBox->currentData().toString());

    int images = get_files(currentDir.toStdString().c_str(), parent->ui->foundImages_comboBox);
    parent->ui->foundImages_label->setText(QString("Imagenes encontradas : %1").arg(QString::number(images)));

    previewImage(parent->ui->foundClasses_comboBox->currentIndex(), parent->ui->foundImages_comboBox->currentIndex());

    connect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::selectedClass_onChange(int index){
    disconnect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    QString currentDir = QString("%1/%2").arg(parent->ui->datasetPath_lineEdit->text(), parent->ui->foundClasses_comboBox->itemData(index).toString());

    int images = get_files(currentDir.toStdString().c_str(), parent->ui->foundImages_comboBox);
    parent->ui->foundImages_label->setText(QString("Imagenes encontradas : %1").arg(QString::number(images)));

    previewImage(index, parent->ui->foundImages_comboBox->currentIndex());

    connect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::selectedImage_onChange(int index){
    previewImage(parent->ui->foundClasses_comboBox->currentIndex(), index);
}

void LoadDataset::previewImage(const int classIndex, const int imageIndex){
    QString selectedImagePath = QString("%1/%2/%3").arg(
                parent->ui->datasetPath_lineEdit->text(),
                parent->ui->foundClasses_comboBox->itemData(classIndex).toString(),
                parent->ui->foundImages_comboBox->itemData(imageIndex).toString()
    );

    cv::Mat image = cv::imread(selectedImagePath.toStdString());
    selectedImage = cv::imread(selectedImagePath.toStdString());
    cvtColor(selectedImage, selectedImage, CV_BGR2RGB);

    QImage qt_image = QImage((const unsigned char*) (selectedImage.data), selectedImage.cols, selectedImage.rows, QImage::Format_RGB888);
    parent->ui->imageDisplay_label->setPixmap(QPixmap::fromImage(qt_image));

    parent->ui->imageInfoRows_label->setText(QString("Filas : %1").arg(QString::number(selectedImage.rows)));
    parent->ui->imageInfoCols_label->setText(QString("Columnas : %1").arg(QString::number(selectedImage.cols)));
}

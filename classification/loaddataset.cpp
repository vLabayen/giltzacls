#include "loaddataset.h"

LoadDataset::LoadDataset(MainWindow* parent) : parent(parent) {}
LoadDataset::~LoadDataset() {}

void LoadDataset::setup(){
    connect(parent->ui->loadDataset_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::loadDataset_onClick(){
    //Disconnect para evitar que se llamen al modificar el contenido de los comboBox
    disconnect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    disconnect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    //Comprobamos que existe el directorio especificado
    if (!dir_exists(parent->ui->datasetPath_lineEdit->text().toStdString().c_str())){
        setError("El directorio no existe");
        parent->ui->foundClasses_label->setText("Clases encontradas :");
        parent->ui->foundImages_label->setText("Imágenes encontradas :");
        return;
    }
    mainDir = parent->ui->datasetPath_lineEdit->text();

    //Obtenemos las subcarpetas (clases) dentro del directorio
    int classes = get_dirs(mainDir.toStdString().c_str(), parent->ui->foundClasses_comboBox);
    parent->ui->foundClasses_label->setText(QString("Clases encontradas : %1").arg(QString::number(classes)));
    if (classes == 0){
        setError("No hay carpetas en el directorio");
        parent->ui->foundImages_label->setText("Imágenes encontradas :");
        return;
    }

    //Obtenemos las imagenes en la subcarpeta que se seleccione
    selectedDir = QString("%1/%2").arg(mainDir, parent->ui->foundClasses_comboBox->currentData().toString());
    int images = get_files(selectedDir.toStdString().c_str(), parent->ui->foundImages_comboBox);
    parent->ui->foundImages_label->setText(QString("Imágenes encontradas : %1").arg(QString::number(images)));

    //De haber alguna imagen la mostramos, en caso contrario limpiamos la posible imagen que se este mostrando
    if (images > 0) previewImage(parent->ui->foundImages_comboBox->currentIndex());
    else parent->ui->imageDisplay_label->clear();

    //Reconectamos los comboBox
    connect(parent->ui->foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::selectedClass_onChange(int index){
    //Disconect del comboBox de las imagenes para que no se llame al modificar su contenido
    disconnect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    //Obtenemos el directorio que se ha seleccionado y las imagenes dentro de el
    selectedDir = QString("%1/%2").arg(mainDir, parent->ui->foundClasses_comboBox->itemData(index).toString());
    int images = get_files(selectedDir.toStdString().c_str(), parent->ui->foundImages_comboBox);
    parent->ui->foundImages_label->setText(QString("Imágenes encontradas : %1").arg(QString::number(images)));

    //De haber alguna imagen la mostramos, en caso contrario limpiamos la posible imagen que se este mostrando
    if (images > 0) previewImage(parent->ui->foundImages_comboBox->currentIndex());
    else parent->ui->imageDisplay_label->clear();

    //Reconectamos el comboBox
    connect(parent->ui->foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::selectedImage_onChange(int index){
    //Mostramos la imagen
    previewImage(index);
}

void LoadDataset::previewImage(const int imageIndex){
    //Leemos la imagen seleccionada en formato RGB
    cvtColor(cv::imread(QString("%1/%2").arg(selectedDir, parent->ui->foundImages_comboBox->itemData(imageIndex).toString()).toStdString()), selectedImage, CV_BGR2RGB);

    //Mostramos la imagen en el label
    parent->ui->imageDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (selectedImage.data), selectedImage.cols, selectedImage.rows, QImage::Format_RGB888)));

    //Mostramos la informacion sobre la resolución de la imagen
    parent->ui->imageInfoRows_label->setText(QString("Filas : %1").arg(QString::number(selectedImage.rows)));
    parent->ui->imageInfoCols_label->setText(QString("Columnas : %1").arg(QString::number(selectedImage.cols)));
}

void LoadDataset::setError(const char* msg){
    //Escribimos el error y establecemos un timer para eliminarlo en "errorTimeout"
    parent->ui->loadDataset_loadError_label->setText(msg);
    QTimer::singleShot(errorTimeout, this, SLOT(removeError()));

    //Limpiamos la posible imagen que se este mostrando y el contenido de los comboBox
    parent->ui->imageDisplay_label->clear();
    parent->ui->foundClasses_comboBox->clear();
    parent->ui->foundImages_comboBox->clear();
}

void LoadDataset::removeError(){
    //Eliminamos el error
    parent->ui->loadDataset_loadError_label->setText("");
}

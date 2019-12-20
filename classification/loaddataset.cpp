#include "loaddataset.h"

LoadDataset::LoadDataset(MainWindow* parent) : parent(parent) {}
LoadDataset::~LoadDataset() {}

void LoadDataset::setup(){
    connect(parent->ui->loaddataset_load_pushButton, SIGNAL(pressed()), this, SLOT(loadDataset_onClick()));
    connect(parent->ui->loaddataset_foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->loaddataset_foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

getClassesResult LoadDataset::getClasses(QComboBox* dst_cb){
    getClassesResult r;
    r.dir = parent->ui->loaddataset_datasetPath_lineEdit->text();

    if (!dir_exists(r.dir.toStdString().c_str())) {
        r.dirFound = false;
        return r;
    }

    r.dirFound = true;
    r.classesFound = get_dirs(r.dir.toStdString().c_str(), dst_cb);
    return r;
}

getImagesResult LoadDataset::getImages(QString dir, QString folder, QComboBox* dst_cb){
    getImagesResult r;
    r.dir = QString("%1/%2").arg(dir, folder);
    r.imagesFound = get_files(r.dir.toStdString().c_str(), dst_cb);
    return r;
}

void LoadDataset::loadDataset_onClick(){
    //Disconnect para evitar que se llamen al modificar el contenido de los comboBox
    disconnect(parent->ui->loaddataset_foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    disconnect(parent->ui->loaddataset_foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    //Intentamos cargar las distintas clases
    getClassesResult rc = getClasses(parent->ui->loaddataset_foundClasses_comboBox);
    if (!rc.dirFound){
        setError("El directorio no existe");
        parent->ui->loaddataset_foundClasses_label->setText("Clases encontradas :");
        parent->ui->loaddataset_foundImages_label->setText("Imágenes encontradas :");
        return;
    }
    parent->ui->loaddataset_foundClasses_label->setText(QString("Clases encontradas : %1").arg(QString::number(rc.classesFound)));
    if (rc.classesFound == 0){
        setError("No hay carpetas en el directorio");
        parent->ui->loaddataset_foundImages_label->setText("Imágenes encontradas :");
        return;
    }
    //Intentamos cargar las imágenes de la clase seleccionada
    getImagesResult ri = getImages(rc.dir, parent->ui->loaddataset_foundClasses_comboBox->currentData().toString(), parent->ui->loaddataset_foundImages_comboBox);
    parent->ui->loaddataset_foundImages_label->setText(QString("Imágenes encontradas : %1").arg(QString::number(ri.imagesFound)));

    //Guardamos las rutas necesarias
    mainDir = rc.dir;
    selectedDir = ri.dir;

    //De haber alguna imagen la mostramos, en caso contrario limpiamos la posible imagen que se este mostrando
    if (ri.imagesFound > 0) previewImage(ri.dir, parent->ui->loaddataset_foundImages_comboBox->currentIndex());
    else parent->ui->loaddataset_imageDisplay_label->clear();

    //Reconectamos los comboBox
    connect(parent->ui->loaddataset_foundClasses_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedClass_onChange(int)));
    connect(parent->ui->loaddataset_foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::selectedClass_onChange(int index){
    //Disconect del comboBox de las imagenes para que no se llame al modificar su contenido
    disconnect(parent->ui->loaddataset_foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));

    //Obtenemos el directorio que se ha seleccionado y las imagenes dentro de el
    getImagesResult ri = getImages(mainDir, parent->ui->loaddataset_foundClasses_comboBox->itemData(index).toString(), parent->ui->loaddataset_foundImages_comboBox);
    parent->ui->loaddataset_foundImages_label->setText(QString("Imágenes encontradas : %1").arg(QString::number(ri.imagesFound)));

    //De haber alguna imagen la mostramos, en caso contrario limpiamos la posible imagen que se este mostrando
    if (ri.imagesFound > 0) previewImage(ri.dir, parent->ui->loaddataset_foundImages_comboBox->currentIndex());
    else parent->ui->loaddataset_imageDisplay_label->clear();

    //Guardamos las rutas necesarias
    selectedDir = ri.dir;

    //Reconectamos el comboBox
    connect(parent->ui->loaddataset_foundImages_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedImage_onChange(int)));
}

void LoadDataset::selectedImage_onChange(int index){
    //Mostramos la imagen
    previewImage(selectedDir, index);
}

void LoadDataset::previewImage(QString dir, const int imageIndex){
    //Leemos la imagen seleccionada en formato RGB
    cvtColor(cv::imread(QString("%1/%2").arg(dir, parent->ui->loaddataset_foundImages_comboBox->itemData(imageIndex).toString()).toStdString()), selectedImage, CV_BGR2RGB);
    //Mostramos la imagen en el label
    parent->ui->loaddataset_imageDisplay_label->setPixmap(QPixmap::fromImage(QImage((const unsigned char*) (selectedImage.data), selectedImage.cols, selectedImage.rows, QImage::Format_RGB888)));
    //Mostramos la informacion sobre la resolución de la imagen
    parent->ui->loaddataset_imageInfoRows_label->setText(QString("Filas : %1").arg(QString::number(selectedImage.rows)));
    parent->ui->loaddataset_imageInfoCols_label->setText(QString("Columnas : %1").arg(QString::number(selectedImage.cols)));
}

void LoadDataset::setError(const char* msg){
    //Escribimos el error y establecemos un timer para eliminarlo en "errorTimeout"
    parent->ui->loaddataset_loadError_label->setText(msg);
    QTimer::singleShot(errorTimeout, this, [this]()->void{
        parent->ui->loaddataset_loadError_label->setText("");
    });

    //Limpiamos la posible imagen que se este mostrando y el contenido de los comboBox
    parent->ui->loaddataset_imageDisplay_label->clear();
    parent->ui->loaddataset_foundClasses_comboBox->clear();
    parent->ui->loaddataset_foundImages_comboBox->clear();
}

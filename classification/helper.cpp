#include <helper.h>

bool dir_exists (const char* name) {
  struct stat buffer;
  return (stat (name, &buffer) == 0);
}

bool make_dir(const char* name) {
    return (mkdir(name, 0755) != -1);
}

void listCameras(int maxTested, QComboBox* dropdown){
    cv::VideoCapture tmp_camera;
    for (int i = 0; i < maxTested; i++){
        bool res = false;
        try {
            tmp_camera.open(i);
            res = tmp_camera.isOpened();
            tmp_camera.release();
        }
        catch (...) {
            continue;
        }

        if (res) dropdown->addItem(QString("Camara %1").arg(QString::number(i)), QVariant(i));
    }
}

canSaveImageResult canSaveImage(QLineEdit* dir, QLineEdit* key, QLineEdit* image, QLineEdit* extension){
    canSaveImageResult result;

    if (!dir_exists(dir->text().toStdString().c_str())) {
        result.success = false;
        result.error = QString("El directorio no existe");
        return result;
    }

    if (key->text() == QString("")){
        result.success = false;
        result.error = QString("Tipo de llave vacio");
        return result;
    }

    QString keydir = QString("%1/%2").arg(dir->text(), key->text());
    if (!dir_exists(keydir.toStdString().c_str())) {
        if (!make_dir(keydir.toStdString().c_str())) {
            result.success = false;
            result.error = QString("Imposible crear directorio %1").arg(key->text());
            return result;
        }
    }

    if (image->text() == QString("")) {
        result.success = false;
        result.error = QString("Nombre de imagen vacio");
        return result;
    }
    if (extension->text() == QString("")) {
        result.success = false;
        result.error = QString("Extension de imagen vacia");
        return result;
    }

    result.success = true;
    result.file = QString("%1/JV-%2_%3.%4").arg(keydir, key->text(), image->text(), extension->text());
    return result;
}

int get_dirs(const char* name, QComboBox* cb){
    cb->clear();
    if (!dir_exists(name)) return 0;

    DIR* d = opendir(name);
    struct dirent *dirp;
    int foundDirs = 0;

    while ((dirp = readdir(d)) != NULL) {
        if (dirp->d_type == DT_DIR && strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
            cb->addItem(QString(dirp->d_name), QVariant(dirp->d_name));
            foundDirs++;
        }
    }

    return foundDirs;
}

int get_files(const char *name, QComboBox *cb){
    cb->clear();
    if (!dir_exists(name)) return 0;

    DIR* d = opendir(name);
    struct dirent *dirp;
    int foundFiles = 0;

    while ((dirp = readdir(d)) != NULL) {
        if (dirp->d_type == DT_REG) {
            cb->addItem(QString(dirp->d_name), QVariant(dirp->d_name));
            foundFiles++;
        }
    }

    return foundFiles;
}

#include <helper.h>

bool dir_exists (const char* name) {
  struct stat buffer;
  return (stat (name, &buffer) == 0);
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

    /*if (foundDirs > 1){
        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(cb);
        proxy->setSourceModel(cb->model());
        cb->model()->setParent(proxy);
        cb->setModel(proxy);
        cb->model()->sort(0);
    }*/
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

    /*if (foundFiles > 1){
        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(cb);
        proxy->setSourceModel(cb->model());
        cb->model()->setParent(proxy);
        cb->setModel(proxy);
        cb->model()->sort(0);
    }*/
    return foundFiles;
}

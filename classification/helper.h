#ifndef HELPER_H
#define HELPER_H

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>

#include <QComboBox>
#include <QLineEdit>
#include <QTimer>

#include <opencv2/core.hpp>
#include <opencv2/videoio/videoio.hpp>

struct canSaveImageResult {
    bool success;
    QString error;
    QString file;
};

bool dir_exists (const char* name);
bool make_dir(const char* name);
void listCameras(int maxTested, QComboBox* dropdown);
canSaveImageResult canSaveImage(QLineEdit* dir, QLineEdit* key, QLineEdit* image, QLineEdit* extension);
int get_dirs(const char* name, QComboBox* cb);
int get_files(const char* name, QComboBox* cb);

#endif // HELPER_H



#ifndef HELPER_H
#define HELPER_H

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>

#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QTimer>

#include <opencv2/core.hpp>
#include <opencv2/videoio/videoio.hpp>

bool dir_exists (const char* name);
int get_dirs(const char* name, QComboBox* cb);
int get_files(const char* name, QComboBox* cb);

#endif // HELPER_H



#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    captureManager(new Capture(this)),
    loadDatasetManager(new LoadDataset(this)),
    segmentationManager(new Segmentation(this)),
    featureExtractionManager(new FeatureExtraction(this)),
    trainingManager(new Training(this)),
    demoManager(new Demo(this))
{
    ui->setupUi(this);
    captureManager->setup();
    loadDatasetManager->setup();
    segmentationManager->setup();
    featureExtractionManager->setup();
    trainingManager->setup();
    demoManager->setup();
}

MainWindow::~MainWindow()
{
    delete ui;
}



#include "standardscaler.h"

//Constructor estandar sabiendo el numero de features
StandardScaler::StandardScaler(int numFeatures) : numFeatures(numFeatures) {
    featureMean = std::vector<float>(numFeatures, 0);
    featureStd = std::vector<float>(numFeatures, 0);

    setlocale(LC_NUMERIC, "en_US.UTF-8");
}

//Constructor a partir de fichero guardado con la funcion save
//Se lee el archivo y se crean los vectores ya inicializados con dichos valores
StandardScaler::StandardScaler(const char *filename){
    FILE* fid = fopen(filename, "r");
    char buf[1024];

    fgets(buf, 1024, fid);
    sscanf(buf, "%d\n", &numFeatures);

    featureMean = std::vector<float>(numFeatures, 0);
    featureStd = std::vector<float>(numFeatures, 0);
    for (int i = 0; i < numFeatures; i++){
        fgets(buf, 1024, fid);
        sscanf(buf, "%f,%f\n", &(featureMean[i]), &(featureStd[i]));
    }

    fclose(fid);
}

StandardScaler::~StandardScaler() {}

void StandardScaler::fit(cv::Mat data){
    //Calculamos suma y suma de cuadrados
    //para calcular media y desviacion tipica en una iteracion del dataset
    std::vector<float> sum(numFeatures, 0);
    std::vector<float> squaredSum(numFeatures, 0);
    for (int i = 0; i < data.rows; i++){
        for (int j = 0; j < data.cols; j++){
            float val = data.at<float>(i, j);
            sum[j] += val;
            squaredSum[j] += (val * val);
        }
    }

    //Establecemos los valores de la respectiva feature
    for (int j = 0; j < numFeatures; j++){
        featureMean[j] = (sum[j] / data.rows);
        featureStd[j] = (float)sqrt((squaredSum[j] / data.rows) - (featureMean[j] * featureMean[j]));
    }
}

void StandardScaler::transform(cv::Mat data){
    //Recorremos los valores del dataset y los estandarizamos con los valores de media y std almacenados
    for (int i = 0; i < data.rows; i++){
        for (int j = 0; j < data.cols; j++){
            data.at<float>(i, j) = (data.at<float>(i, j) - featureMean[j]) / featureStd[j];
        }
    }
}

void StandardScaler::save(const char* filename){
    //Guardamos los valores de media y std en un fichero
    //La primera linea tendra el numero de features consideradas
    //Se generará una linea para cada feature con formato mean,std\n

    FILE* fid = fopen(filename, "w");

    fprintf(fid, "%d\n", numFeatures);
    for (int i = 0; i < numFeatures; i++) fprintf(fid, "%.5f,%.5f\n", featureMean[i], featureStd[i]);

    fclose(fid);
}

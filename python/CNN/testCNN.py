import sys

try: modelfile = sys.argv[1]
except: sys.exit('Model file required as first argument')
try: imgs_dir = sys.argv[2]
except: sys.exit('Images dir required as first argument')

import os
if not os.path.isfile(modelfile): sys.exit('{} not found'.format(modelfile))
if not os.path.isdir(imgs_dir): sys.exit('{} is not a dir'.format(imgs_dir))

import numpy as np
import cv2

#List images
def listImages(imgs_dir):
        cls_dirs = [d for d in os.listdir(imgs_dir) if os.path.isdir('{}/{}'.format(imgs_dir, d))]
        images = []
        labels = []
        for cls in cls_dirs:
                fulldir = '{}/{}'.format(imgs_dir, cls)
                for file in os.listdir(fulldir):
                        fullpath = '{}/{}'.format(fulldir, file)
                        if os.path.isfile(fullpath):
                                images.append(fullpath)
                                labels.append(cls)

        return images, labels

def getLabelName(label, class_set):
        index = np.argmax(label)
        return '{}'.format(class_set[index])

#Read images
def readImages(files, image_res = (90, 30)):
        images = []
        for file in files:
                original_image = cv2.imread(file, 0)
                resized_image = cv2.resize(original_image, image_res, interpolation=cv2.INTER_AREA)
                images.append(resized_image)

        blob = cv2.dnn.blobFromImages(images)
        return blob

model = cv2.dnn.readNetFromTensorflow(modelfile)

images, labels = listImages(imgs_dir)
X = readImages(images)

class_set = sorted(list(set(labels)))

model.setInput(X)
pred = model.forward()
for i,p in enumerate(pred):
        y_true = labels[i]
        y_pred = getLabelName(p, class_set)

        print('true : {} - pred : {}'.format(y_true, y_pred))

import sys

try: imgs_dir = sys.argv[1]
except: sys.exit('Images dir required in first arg')
epochs = int(sys.argv[sys.argv.index('--epochs') + 1]) if '--epochs' in sys.argv else 50
iterations = int(sys.argv[sys.argv.index('-i') + 1]) if '-i' in sys.argv else 1
exportPath = sys.argv[sys.argv.index('-e') + 1] if '-e' in sys.argv else None
exportModel = sys.argv[sys.argv.index('-m') + 1] if '-m' in sys.argv else None
exportHistory = sys.argv[sys.argv.index('-h') + 1] if '-h' in sys.argv else None
color = '-c' in sys.argv
flipEval = '-ff' in sys.argv
flipTrain = '-f' in sys.argv or flipEval

import os
if not os.path.isdir(imgs_dir): sys.exit('{} is not a dir'.format(imgs_dir))
if not epochs > 0: sys.exit('Epochs must be greater than 0')
if not iterations > 0: sys.exit('Iterations must be greater than 0')

import cv2
import numpy as np
from keras.models import Sequential
from keras.layers import Dense, Conv2D, MaxPooling2D, AveragePooling2D, GlobalAveragePooling2D, Flatten
from keras.utils import to_categorical
import random

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

def getCategoricalLabels(labels, class_set = None, flip = False):
	if flip: labels = [l for l in labels for _ in (0,1,2,3)]
	if class_set is None:
		class_set = sorted(list(set(labels)))
		y = to_categorical([class_set.index(cls) for cls in labels])
		return y, class_set
	else:
		y = to_categorical([class_set.index(cls) for cls in labels])
		return y

def getLabelName(label, class_set):
	index = np.argmax(label)
	return '{}'.format(class_set[index])

#Read images
def readImages(files, image_res = (90, 30), color = False, flip = False):
	images = []
	for file in files:
		original_image = cv2.imread(file, 0) if not color else cv2.imread(file)
		resized_image = cv2.resize(original_image, image_res, interpolation=cv2.INTER_AREA)
		images.append(resized_image)
		if flip:
			images.append(cv2.flip(resized_image, -1))
			images.append(cv2.flip(resized_image, 0))
			images.append(cv2.flip(resized_image, 1))

	blob = cv2.dnn.blobFromImages(images)
	return np.transpose(blob, (0, 2, 3, 1)) #Esta es la magia

def split(X, y, train_rate = 0.5, eval_rate = 0.3, test_rate = 0.2):
#def split(X, y, train_rate = 0.6, eval_rate = 0.4, test_rate = 0):
	idx_bylabel = {}
	for i,cls in enumerate(y):
		if cls not in idx_bylabel: idx_bylabel[cls] = [i]
		else: idx_bylabel[cls].append(i)

	for cls in idx_bylabel.keys(): random.shuffle(idx_bylabel[cls])
	clslen = {cls : len(arr) for cls,arr in idx_bylabel.items()}

	train_lastindex = {cls : int(clen * train_rate) for cls,clen in clslen.items()}
	eval_lastindex = {cls : train_lastindex[cls] + int(clen * eval_rate) for cls,clen in clslen.items()}

	train_index = [i for cls,arr in idx_bylabel.items() for i in arr[0:train_lastindex[cls]]]
	eval_index = [i for cls,arr in idx_bylabel.items() for i in arr[train_lastindex[cls] + 1:eval_lastindex[cls]]]
	test_index = [i for cls,arr in idx_bylabel.items() for i in arr[eval_lastindex[cls] + 1:]]

	random.shuffle(train_index)
	random.shuffle(eval_index)
	random.shuffle(test_index)

	X_train, y_train, X_eval, y_eval, X_test, y_test = [],[],[],[],[],[]
	for i in train_index:
		X_train.append(X[i])
		y_train.append(y[i])
	for i in eval_index:
		X_eval.append(X[i])
		y_eval.append(y[i])
	for i in test_index:
		X_test.append(X[i])
		y_test.append(y[i])

	return X_train, y_train, X_eval, y_eval, X_test, y_test

#Build model
def buildModel(num_cls, image_res = (90, 30), channels = 1):
	model = Sequential()
	model.add(Conv2D(32, kernel_size=3, activation='relu', input_shape=(image_res[1], image_res[0], channels)))
	model.add(Conv2D(32, kernel_size=3, activation='relu'))
	model.add(AveragePooling2D(pool_size=(2,2), strides=(1,1)))
	model.add(Conv2D(64, kernel_size=3, activation='relu'))
	model.add(Conv2D(64, kernel_size=3, activation='relu'))
	model.add(AveragePooling2D(pool_size=(2,2), strides=(1,1)))
	model.add(Conv2D(128, kernel_size=3, activation='relu'))
	model.add(Conv2D(128, kernel_size=3, activation='relu'))
	model.add(AveragePooling2D(pool_size=(2,2), strides=(1,1)))
	model.add(GlobalAveragePooling2D())
	model.add(Dense(num_cls, activation='softmax'))
	model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])
	return model

images, labels = listImages(imgs_dir)

if exportPath is not None: outf = open(exportPath, 'w')
for iter in range(iterations):
	print('Starting iteration : {}/{}'.format(iter, iterations))

	train_images, train_labels, eval_images, eval_labels, test_images, test_labels = split(images, labels)

	X_train = readImages(train_images, color = color, flip = flipTrain)
	y_train, class_set = getCategoricalLabels(train_labels, flip = flipTrain)
	X_eval, y_eval = readImages(eval_images, color = color, flip = flipEval), getCategoricalLabels(eval_labels, class_set, flip = flipEval)
	X_test, y_test = readImages(test_images, color = color), getCategoricalLabels(test_labels, class_set)

	model = buildModel(num_cls = len(class_set), channels = (3 if color else 1))
	hist = model.fit(X_train, y_train, validation_data=(X_eval, y_eval), epochs=epochs)
	pred = model.predict(X_test)

	for i,p in enumerate(pred):
		y_true = getLabelName(y_test[i], class_set)
		y_pred = getLabelName(p, class_set)

		outstr = '{},{},{}\n'.format(iter, y_true, y_pred)
		print(outstr, end='') if exportPath is None else outf.write(outstr)


	if exportModel is not None: model.save(exportModel, include_optimizer=False)
	if exportHistory is not None:
		histf = open(exportHistory, 'w')
		for e in range(epochs): histf.write('{},{},{},{}\n'.format(hist.history['loss'][e], hist.history['acc'][e], hist.history['val_loss'][e], hist.history['val_acc'][e]))
		histf.close()

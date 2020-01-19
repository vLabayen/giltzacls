#Load script params or default ones
import sys
try : dataset = sys.argv[1]
except : sys.exit('Dataset required as first argument')

train_split_ratio = float(sys.argv[sys.argv.index('-t') + 1]) if '-t' in sys.argv else 0.7
iterations = int(sys.argv[sys.argv.index('-i') + 1]) if '-i' in sys.argv else 100
use_scaler = '-s' in sys.argv
exportPath = sys.argv[sys.argv.index('-e') + 1] if '-e' in sys.argv else None

import os
if not os.path.isfile(dataset): sys.exit('{} not found'.format(dataset))
if not (0 < train_split_ratio < 1): sys.exit('Train split ratio (-t) must be between 0 and 1')
if not (0 < iterations): sys.exit('Iterations (-i) must be greater than 0')

#Import required modules
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import KMeans
from random import random

#Read dataset
def readDataset(filepath):
	data = {}
	with open(filepath, 'r') as f:
		for line in f:
			features = line[:-1].split(',')
			cls,ft = int(features[0]), [float(x) for x in features[1:]]

			if cls not in data: data[cls] = [ft]
			else: data[cls].append(ft)
	return data

#Split dataset
def splitDataset(data, tr = 0.7):
	X_train, y_train, X_test, y_test = [], [], [], []
	for cls,d in data.items():
		for f in d:
			if random() < tr:
				X_train.append(f)
				y_train.append(cls)
			else:
				X_test.append(f)
				y_test.append(cls)
	return X_train, y_train, X_test, y_test

#Train & test
def trainTest(X_train, y_train, X_test, y_test, use_scaler = False):
	if use_scaler:
		ss = StandardScaler().fit(X_train)
		X_train = ss.transform(X_train)
		X_test = ss.transform(X_test)

	km = KMeans(n_clusters = 6)
	km.fit(X_train)

	trainpred = km.predict(X_train)
	testpred = km.predict(X_test)

	return trainpred, testpred

#Cluster assigment
def clusterAssigment(y_train, train_pred):
	ca = {}
	for i,p in enumerate(train_pred):
		if p not in ca: ca[p] = [y_train[i]]
		else : ca[p].append(y_train[i])

	for p in ca: ca[p] = max(set(ca[p]), key=ca[p].count)
	return ca



data = readDataset(dataset)

if exportPath is not None: outf = open(exportPath, 'w')

for iter in range(iterations):
	X_train, y_train, X_test, y_test = splitDataset(data, train_split_ratio)
	trainpred, testpred = trainTest(X_train, y_train, X_test, y_test, use_scaler = use_scaler)
	clsAssigment = clusterAssigment(y_train, trainpred)

	for i,p in enumerate(testpred):
		outstr = '{},{},{}\n'.format(iter, y_test[i], clsAssigment[p])
		print(outstr, end='') if exportPath is None else outf.write(outstr)

if exportPath is not None: outf.close()

import sys
try: dataset = sys.argv[1]
except: sys.exit('Dataset required as first argument')

train_split_ratio = float(sys.argv[sys.argv.index('-t') + 1]) if '-t' in sys.argv else 0.7
iterations = int(sys.argv[sys.argv.index('-i') + 1]) if '-i' in sys.argv else 100
use_scaler = '-s' in sys.argv
use_optimizer = '-o' in sys.argv
exportPath = sys.argv[sys.argv.index('-e') + 1] if '-e' in sys.argv else None
use_leaveOneOut = '-l' in sys.argv

import os
if not os.path.isfile(dataset): sys.exit('{} not found'.format(dataset))
if not (0 < train_split_ratio < 1): sys.exit('Train split ratio (-t) must be between 0 and 1')
if not (0 < iterations): sys.exit('Iterations (-i) must be greater than 0')

from sklearn.preprocessing import StandardScaler
from sklearn.multiclass import OneVsRestClassifier
from sklearn import svm
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

#Join dataset
def joinDataset(data):
	X, y = [], []
	for cls,d in data.items():
		for f in d:
			X.append(f)
			y.append(cls)
	return X,y

#Optimize hyperparams
def optimize(data, train_split_ratio, iterations = 100, use_scaler = False):
	res = {}
	for i in range(iterations):
		print('Performing optimization... {}/{}'.format(i, iterations), end='\r')

		X_train, y_train, X_test, y_test = splitDataset(data, train_split_ratio)
		if use_scaler:
			ss = StandardScaler().fit(X_train)
			X_train = ss.transform(X_train)
			X_test = ss.transform(X_test)

		for k in ['linear', 'poly', 'rbf', 'sigmoid']:
			for c in [0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.5, 2]:
				for df in ['ovo', 'ovr']:
					kargs = {'kernel' : k, 'C' : c, 'decision_function_shape' : df}
					sm = OneVsRestClassifier(svm.SVC(**kargs))
					sm.fit(X_train, y_train)
					pred = sm.predict(X_test)

					sum = 0
					for i,p in enumerate(pred):
						if p == y_test[i]: sum += 1
					iterkey = '{}-{}-{}'.format(k,c,df)
					res[iterkey] = sum

	best_res = max(res, key = res.get)
	k,c,df = best_res.split('-')
	return {'kernel' : k, 'C' : float(c), 'decision_function_shape' : df}

#Train & test
def trainTest(X_train, y_train, X_test, y_test, use_scaler = False, kargs = {}):
	if use_scaler:
		ss = StandardScaler().fit(X_train)
		X_train = ss.transform(X_train)
		X_test = ss.transform(X_test)

	sm = OneVsRestClassifier(svm.SVC(**kargs))
	sm.fit(X_train, y_train)
	pred = sm.predict(X_test)

	return pred


data = readDataset(dataset)

if use_optimizer:
	kargs = optimize(data, train_split_ratio, iterations = iterations, use_scaler = use_scaler)
	print(kargs)
else : kargs = {}

if exportPath is not None: outf = open(exportPath, 'w')

if not use_leaveOneOut:
	for iter in range(iterations):
		X_train, y_train, X_test, y_test = splitDataset(data, train_split_ratio)
		pred = trainTest(X_train, y_train, X_test, y_test, use_scaler = use_scaler, kargs = kargs)

		for i,p in enumerate(pred):
			outstr = '{},{},{}\n'.format(iter, y_test[i], p)
			print(outstr, end='') if exportPath is None else outf.write(outstr)
else:
	X, y = joinDataset(data)
	for i in range(len(X)):
		X_train, y_train = X[:i] + X[i+1:], y[:i] + y[i+1:]
		X_test, y_test = X[i], y[i]
		pred = trainTest(X_train, y_train, [X_test], [y_test], use_scaler = use_scaler, kargs = kargs)

		outstr = '{},{},{}\n'.format(i, y_test, pred[0])
		print(outstr, end='') if exportPath is None else outf.write(outstr)

if exportPath is not None: outf.close()

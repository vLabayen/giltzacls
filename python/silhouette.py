import sys

try: dataset = sys.argv[1]
except: sys.exit('Dataset required as first arg')

import os
if not os.path.isfile(dataset): sys.exit('{} not found'.format(dataset))

import matplotlib.pyplot as plt
import sklearn.metrics as metrics
from sklearn.preprocessing import StandardScaler

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

#Join dataset
def joinDataset(data):
	X, y = [], []
	for cls,d in data.items():
		for f in d:
			X.append(f)
			y.append(cls)
	return X,y

#Scale dataset
def scaleDataset(X):
	ss = StandardScaler().fit(X)
	X = ss.transform(X)
	return X

data = readDataset(dataset)
X,y = joinDataset(data)
X = scaleDataset(X)
scoef = metrics.silhouette_samples(X, y)

res = {}
for i,s in enumerate(scoef):
	if y[i] not in res: res[y[i]] = [s]
	else: res[y[i]].append(s)
for k in res.keys(): res[k] = sorted(res[k], reverse = True)

done = [False for _ in res.keys()]
sample_index = 0
keys = sorted(list(res.keys()))

fig, ax = plt.subplots()
while not all(done):
	for i,k in enumerate(keys):
		try: ax.barh(i - (0.01 * sample_index), res[k][sample_index], height = 0.01, color = 'b')
		except: done[i] = True
	sample_index += 1

ax.set(yticks = [i - 0.4 for i in range(len(keys))], yticklabels = ['Llave {}'.format(i + 1) for i in range(len(keys))])
plt.show()

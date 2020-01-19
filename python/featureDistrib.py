import sys

try: dataset = sys.argv[1]
except: sys.exit('Dataset required as first arg')
try: features_header = sys.argv[2]
except: sys.exit('Dataset header required as second arg')
use_scaler = '-s' in sys.argv

import os
if not os.path.isfile(dataset): sys.exit('{} not found'.format(dataset))

import matplotlib.pyplot as plt
from sklearn.preprocessing import StandardScaler
from numpy import arange as arange
from math import ceil

#read dataset
def readDataset(filepath, fheader, separator = ','):
	fheader = fheader.split(separator)
	data = {ffield : {} for ffield in fheader}

	with open(filepath, 'r') as f:
		for line in f:
			features = line[:-1].split(',')
			cls, f = features[0], [float(x) for x in features[1:]]

			for i,ffield in enumerate(fheader):
				if cls not in data[ffield]: data[ffield][cls] = [f[i]]
				else : data[ffield][cls].append(f[i])

	return data

#Standarize dataset
def standarize(data):
	X_data = []
	ffields = list(data.keys())
	cls = list(set([c for ffield in ffields for c in data[ffield].keys()]))
	for c in cls:
		for i in range(len(data[ffields[0]][c])):
			X_data.append([])
			for f in ffields:
				X_data[-1].append(data[f][c][i])

	ss = StandardScaler().fit(X_data)
	for c in cls:
		for i in range(len(data[ffields[0]][c])):
			X_vals = [[data[f][c][i] for f in ffields]]
			X_scaled = ss.transform(X_vals)
			for j,v in enumerate(X_scaled[0]): data[ffields[j]][c][i] = v

	return data

#group near samples
def groupSamples(data, num_groups = 15):
	g = {ffield : {} for ffield in data.keys()}
	values = {ffield : [x for arr in data[ffield].values() for x in arr] for ffield in data.keys()}
	valranges = {ffield : {'max' : max(values[ffield]), 'min' : min(values[ffield])} for ffield in data.keys()}

	for ffield in g.keys():
		group_inc = ceil(10000 * (valranges[ffield]['max'] - valranges[ffield]['min']) / num_groups) / 10000
		limits = [round(x + group_inc, 3) for x in arange(valranges[ffield]['min'], valranges[ffield]['max'], group_inc)]

		for cls,arr in data[ffield].items():
			if cls not in g[ffield]: g[ffield][cls] = [0] * num_groups
			for v in arr:
				for i,l in enumerate(limits):
					if v <= l:
						g[ffield][cls][i] += 1
						break

		g[ffield]['centers'] = [l + (group_inc / 2) for l in limits]
		g[ffield]['group_width'] = group_inc

	return g

data = readDataset(dataset, features_header)
if use_scaler: data = standarize(data)
groups = groupSamples(data)

fig, ax = plt.subplots(5, 1)

colors = ['b', 'g', 'r', 'c', 'm', 'y']
for i,ffield in enumerate(groups.keys()):
	for cls in sorted([k for k in groups[ffield].keys() if k != 'centers' and k != 'group_width']):
		ax[i].plot(groups[ffield]['centers'], groups[ffield][cls], label = 'Llave {}'.format(cls))
#		ax[i].bar(groups[ffield]['centers'], groups[ffield][cls], width = groups[ffield]['group_width'], fill = False, label = 'Llave {}'.format(cls), color = colors[i])
		ax[i].set_title(ffield)
		ax[i].set_ylabel('# Muestras')

	ax[i].legend()


fig.subplots_adjust(hspace=0.5)
plt.show()

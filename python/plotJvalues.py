import sys

try: datafile = sys.argv[1]
except: sys.exit('Datafile required as first argument')
try: xaxis = sys.argv[2]
except: sys.exit('X axis required as second argument')
substractFile = sys.argv[sys.argv.index('-s') + 1] if '-s' in sys.argv else None
exportPath = sys.argv[sys.argv.index('-e') + 1] if '-e' in sys.argv else None

import os
if not os.path.isfile(datafile): sys.exit('{} not found'.format(datafile))
if substractFile is not None and not os.path.isfile(substractFile): sys.exit('{} not found'.format(substractFile))
import matplotlib.pyplot as plt

def readData(file):
	j1, j2, j3 = [],[],[]
	with open(file, 'r') as f:
		for line in f:
			j1val,j2val,j3val = line[:-1].split(',')

			j1.append(float(j1val))
			j2.append(float(j2val))
			j3.append(float(j3val))
	return j1,j2,j3


xaxis = [int(x) for x in xaxis.split(',')]
j1,j2,j3 = readData(datafile)
if substractFile is not None:
	j1s, j2s, j3s = readData(substractFile)
	if len(j1s) != len(j1): sys.exit('La cantidad de valores en los distintos ficheros no coincide')

	j1 = [v - j1s[i] for i,v in enumerate(j1)]
	j2 = [v - j2s[i] for i,v in enumerate(j2)]
	j3 = [v - j3s[i] for i,v in enumerate(j3)]

if (len(j1) != len(xaxis)): sys.exit('Las longitudes de los datos y el eje no coinciden')

fig, ax = plt.subplots(3, 1)

ax[0].plot(xaxis, j1)
ax[0].set_title('Indice J1')
ax[1].plot(xaxis, j2)
ax[1].set_title('Indice J2')
ax[2].plot(xaxis, j3)
ax[2].set_title('Indice J3')
ax[2].set_xlabel('Cantidad de columnas utilizadas')

fig.subplots_adjust(hspace=0.5)
if exportPath is not None: plt.savefig(exportPath, dpi=350)
plt.show()

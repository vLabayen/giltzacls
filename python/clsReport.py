#!/usr/bin/env python3
import sys

try : datafile = sys.argv[1]
except IndexError: sys.exit('This scripts needs the results datafile')
substractFile = sys.argv[sys.argv.index('-s') + 1] if '-s' in sys.argv else None
latexFormat = '-l' in sys.argv
just_mean = '-avg' in sys.argv

import os
if not os.path.isfile(datafile): sys.exit('{} not found'.format(datafile))
if substractFile is not None and not os.path.isfile(substractFile): sys.exit('{} not found'.format(substractFile))

from sklearn.metrics import classification_report

#Read the file and store the data
def readData(path):
	with open(path, 'r') as src:
		res = {'index' : [], 'target' : [], 'pred' : []}
		for line in src:
			index, target, pred = line[:-1].split(',')
			res['index'].append(index)
			res['target'].append(target)
			res['pred'].append(pred)
	return res

#get report
def getReport(res, latexFormat):
	activities = ['Llave 1', 'Llave 2', 'Llave 3', 'Llave 4', 'Llave 5', 'Llave 6']
	ytrue = ['Llave {}'.format(x) for x in res['target']]
	ypred = ['Llave {}'.format(x) for x in res['pred']]
	return classification_report(ytrue, ypred, output_dict = latexFormat, digits = 4)


res = readData(datafile)
report = getReport(res, latexFormat)
if latexFormat:
	if substractFile is not None:
		sres = readData(substractFile)
		sreport = getReport(sres, True)
		for k in sreport.keys():
			for f,v in sreport[k].items():
				report[k][f] -= v

	print('\\begin{table}[H]')
	print('\\setlength{\\tabcolsep}{18pt}')
	print('\\centering')
	print('\\begin{tabular}{c*{3}{c}r}')
	print('Llave & Precisión & Sensibilidad & f1-score \\\\')
	print('\hline')
	print('1 & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['Llave 1']['precision'], report['Llave 1']['recall'], report['Llave 1']['f1-score']))
	print('2 & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['Llave 2']['precision'], report['Llave 2']['recall'], report['Llave 2']['f1-score']))
	print('3 & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['Llave 3']['precision'], report['Llave 3']['recall'], report['Llave 3']['f1-score']))
	print('4 & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['Llave 4']['precision'], report['Llave 4']['recall'], report['Llave 4']['f1-score']))
	print('5 & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['Llave 5']['precision'], report['Llave 5']['recall'], report['Llave 5']['f1-score']))
	print('6 & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['Llave 6']['precision'], report['Llave 6']['recall'], report['Llave 6']['f1-score']))
	print('\\hline')
	print('Media & {:.4f} & {:.4f} & {:.4f} \\\\'.format(report['macro avg']['precision'], report['macro avg']['recall'], report['macro avg']['f1-score']))
	print('\\end{tabular}')
	print('\\caption{Rellenar esto}')
	print('\\label{tab:Rellenar esto}')
	print('\\end{table}')

else :
	if not just_mean: print(report)
	else:
		values = report.split('\n')[10][17:].split('    ')[:-1]
		print(' '.join(values))

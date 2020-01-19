import sys

try: histfile = sys.argv[1]
except: sys.exit('History file required as first argument')
exportPath = sys.argv[sys.argv.index('-e') + 1] if '-e' in sys.argv else None
epochs_limit = int(sys.argv[sys.argv.index('-l') + 1]) if '-l' in sys.argv else None
error_limit = float(sys.argv[sys.argv.index('-err') + 1]) if '-err' in sys.argv else None

import os
if not os.path.isfile(histfile): sys.exit('{} not found'.format(histfile))
if epochs_limit is not None and not epochs_limit > 0: sys.exit('Epochs limit must be greater than 0')

import matplotlib.pyplot as plt

#Read data
def readHist(file):
	data = {'loss' : [], 'acc' : [], 'eval_loss' : [], 'eval_acc' : [], 'epoch' : []}
	with open(file, 'r') as f:
		for i,line in enumerate(f):
			if epochs_limit is not None and not i < epochs_limit: break
			d = line[:-1].split(',')
			loss,acc,eval_loss,eval_acc = [float(x) for x in d]
			data['loss'].append(loss)
			data['acc'].append(acc)
			data['eval_loss'].append(eval_loss)
			data['eval_acc'].append(eval_acc)
			data['epoch'].append(i)
	return data


hist = readHist(histfile)
fig, ax1 = plt.subplots()

ax1.set_xlabel('Época')
ax1.set_ylabel('Error')
line11, = ax1.plot(hist['epoch'], hist['loss'], linestyle = 'solid', color='red')
line12, = ax1.plot(hist['epoch'], hist['eval_loss'], linestyle = 'dashed', color='red')
ax1.tick_params(axis='y')

kargs = {'bottom' : -0.05}
if error_limit is not None: kargs['top'] = error_limit
ax1.set_ylim(**kargs)

ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
ax2.set_ylabel('Precisión')  # we already handled the x-label with ax1
line21, = ax2.plot(hist['epoch'], hist['acc'], linestyle = 'solid', color='blue')
line22, = ax2.plot(hist['epoch'], hist['eval_acc'], linestyle = 'dashed', color='blue')
ax2.tick_params(axis='y')
ax2.set_ylim(**{'bottom' : -0.05, 'top' : 1.05})

ax2.legend((line11, line12, line21, line22), ("Error de entrenamiento", "Error de evaluación", "Precisión de entrenamiento", "Precisión de evaluación"), loc = 'center right')
fig.tight_layout()  # otherwise the right y-label is slightly clipped

if exportPath is not None: plt.savefig(exportPath, dpi = 350)
plt.show()

#!/usr/bin/env python3
import sys

try : datafile = sys.argv[1]
except IndexError: sys.exit('This scripts needs the results datafile')
exportPath = sys.argv[sys.argv.index('-e') + 1] if '-e' in sys.argv else None

import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix
from sklearn.utils.multiclass import unique_labels
import numpy as np

def plot_confusion_matrix(y_true, y_pred, classes,
                          normalize=False,
                          title=None,
                          cmap=plt.cm.Blues):
    """
    This function prints and plots the confusion matrix.
    Normalization can be applied by setting `normalize=True`.
    """
    if not title:
        if normalize:
            title = 'Normalized confusion matrix'
        else:
            title = 'Confusion matrix, without normalization'

    # Compute confusion matrix
    cm = confusion_matrix(y_true, y_pred)
    # Only use the labels that appear in the data
    classes = classes[unique_labels(y_true, y_pred)]
    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalized confusion matrix")
    else:
        print('Confusion matrix, without normalization')

    print(cm)

    fig, ax = plt.subplots()
    im = ax.imshow(cm, interpolation='nearest', cmap=cmap)
    ax.figure.colorbar(im, ax=ax)
    # We want to show all ticks...
    ax.set(xticks=np.arange(cm.shape[1]),
           yticks=np.arange(cm.shape[0]),
           # ... and label them with the respective list entries
           xticklabels=classes, yticklabels=classes,
           title=title,
           ylabel='True label',
           xlabel='Predicted label')

    # Rotate the tick labels and set their alignment.
    plt.setp(ax.get_xticklabels(), rotation=45, ha="right",
             rotation_mode="anchor")

    # Loop over data dimensions and create text annotations.
    fmt = '.2f' if normalize else 'd'
    thresh = cm.max() / 2.
    for i in range(cm.shape[0]):
        for j in range(cm.shape[1]):
            ax.text(j, i, format(cm[i, j], fmt),
                    ha="center", va="center",
                    color="white" if cm[i, j] > thresh else "black")
    fig.tight_layout()
    return ax


#Read the file and store the data
with open(datafile, 'r') as src:
    res = {'index' : [], 'target' : [], 'pred' : []}
    for line in src:
        index, target, pred = line[:-1].split(',')
        res['index'].append(index)
        res['target'].append(target)
        res['pred'].append(pred)

activities = ['Llave 1', 'Llave 2', 'Llave 3', 'Llave 4', 'Llave 5', 'Llave 6']
ytrue = [int(activities.index('Llave {}'.format(x))) for x in res['target']]
ypred = [int(activities.index('Llave {}'.format(x))) for x in res['pred']]

plot_confusion_matrix(ytrue, ypred, np.array(activities), normalize=False, title='Confusion Matrix')
if exportPath is not None:
        plt.savefig(exportPath, dpi=350)
        print('Image saved at {}'.format(exportPath))
plt.show()

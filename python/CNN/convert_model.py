import sys

try: modelfile = sys.argv[1]
except: sys.exit('Model file required as first arg')
try: outf = sys.argv[2]
except: sys.exit('Out file required as second arg')

outpath, outfile = '/'.join(outf.split('/')[:-1]), outf.split('/')[-1]

import os
if not os.path.isfile(modelfile): sys.exit('{} not found'.format(modelfile))

import tensorflow as tf

def freeze_session(session, keep_var_names=None, output_names=None, clear_devices=True):
	"""
	Freezes the state of a session into a pruned computation graph.

	Creates a new computation graph where variable nodes are replaced by
	constants taking their current value in the session. The new graph will be
	pruned so subgraphs that are not necessary to compute the requested
	outputs are removed.
	@param session The TensorFlow session to be frozen.
	@param keep_var_names A list of variable names that should not be frozen,
		or None to freeze all the variables in the graph.
	@param output_names Names of the relevant graph outputs.
	@param clear_devices Remove the device directives from the graph for better portability.
	@return The frozen graph definition.
	"""
	graph = session.graph
	with graph.as_default():
		freeze_var_names = list(set(v.op.name for v in tf.global_variables()).difference(keep_var_names or []))
		output_names = output_names or []
		output_names += [v.op.name for v in tf.global_variables()]
		input_graph_def = graph.as_graph_def()
		if clear_devices:
			for node in input_graph_def.node:
				node.device = ''
	frozen_graph = tf.graph_util.convert_variables_to_constants(session, input_graph_def, output_names, freeze_var_names)
	return frozen_graph

model = tf.keras.models.load_model(modelfile)
model.summary()

frozen_graph = freeze_session(tf.keras.backend.get_session(), output_names=[out.op.name for out in model.outputs])
tf.train.write_graph(frozen_graph, outpath, outfile, as_text=False)

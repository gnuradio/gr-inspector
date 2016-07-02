#!/usr/bin/python

import freezegraph
import os
import tensorflow as tf

def save_graph(sess):

    temp = "/tmp/"

    checkpoint_prefix = os.path.join(temp, "saved_checkpoint")
    checkpoint_state_name = "checkpoint_state"
    input_graph_name = "input_graph.pb"
    output_graph_name = "output_graph.pb"

    saver = tf.train.Saver()
    saver.save(sess, checkpoint_prefix, global_step=0,latest_filename=checkpoint_state_name)

    tf.train.write_graph(sess.graph.as_graph_def(),temp,
                           input_graph_name)

    # We save out the graph to disk, and then call the const conversion
    # routine.
    input_graph_path = os.path.join(temp, input_graph_name)
    input_saver_def_path = ""
    input_binary = False
    input_checkpoint_path = checkpoint_prefix + "-0"
    output_node_names = "out"
    restore_op_name = "save/restore_all"
    filename_tensor_name = "save/Const:0"
    output_graph_path = os.path.join(temp, output_graph_name)
    clear_devices = False

    freezegraph.freeze_graph(input_graph_path, input_saver_def_path,
                              input_binary, input_checkpoint_path,
                              output_node_names, restore_op_name,
                              filename_tensor_name, output_graph_path,clear_devices, "")

def load_graph(output_graph_path):
    with tf.Graph().as_default():
        output_graph_def = tf.GraphDef()
        with open(output_graph_path, "rb") as f:
            output_graph_def.ParseFromString(f.read())
            _ = tf.import_graph_def(output_graph_def, name="")
    
        with tf.Session() as sess:
            n_input = sess.graph.get_tensor_by_name("inp:0")
            output = sess.graph.get_tensor_by_name("out:0")

            return (sess,n_input,output)

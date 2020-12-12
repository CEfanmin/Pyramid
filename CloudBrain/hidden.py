import tensorflow.python.platform
import numpy as np
import tensorflow as tf
import plot_boundary_on_data  
from sklearn import preprocessing


# Global variables.
NUM_LABELS = 2    # The number of labels.
BATCH_SIZE = 100  # The number of training examples to use per training step.

tf.app.flags.DEFINE_string('train', None,
                           'File containing the training data (labels & features).')
tf.app.flags.DEFINE_string('test', None,
                           'File containing the test data (labels & features).')
tf.app.flags.DEFINE_integer('num_epochs', 1,
                            'Number of passes over the training data.')
tf.app.flags.DEFINE_integer('num_hidden', 1,
                            'Number of nodes in the hidden layer.')
tf.app.flags.DEFINE_boolean('verbose', False, 'Produce verbose output.')
tf.app.flags.DEFINE_boolean('plot', False, 'Plot the final decision boundary on the data.')

FLAGS = tf.app.flags.FLAGS


# Extract numpy representations of the labels and features given rows consisting of:
#   label, feat_0, feat_1, ..., feat_n
def extract_data(filename):
    # Arrays to hold the labels and feature vectors.
    labels = []
    fvecs = []

    # Iterate over the rows, splitting the label from the features. Convert labels
    # to integers and features to floats.
    for line in file(filename):
        row = line.split(",")
        labels.append(float(row[0]))
        fvecs.append([float(x) for x in row[1:]])

    # Convert the array of float arrays into a numpy float matrix.
    fvecs_np = np.matrix(fvecs).astype(np.float32)
    fvecs_np = preprocessing.scale(fvecs_np)
    
    # Convert the array of int labels into a numpy array.
    labels_np = np.array(labels).astype(dtype=np.uint8)

    # Convert the int numpy array into a one-hot matrix.
    labels_onehot = (np.arange(NUM_LABELS) == labels_np[:, None]).astype(np.float32)

    # Return a pair of the feature matrix and the one-hot label matrix.
    return fvecs_np,labels_onehot

# Init weights 
def init_weights(shape, init_method='xavier', xavier_params = (None, None)):
    if init_method == 'zeros':
        return tf.Variable(tf.zeros(shape, dtype=tf.float32))
    elif init_method == 'uniform':
        return tf.Variable(tf.random_normal(shape, stddev=0.01, dtype=tf.float32))
    else: #xavier
        (fan_in, fan_out) = xavier_params
        low = -4*np.sqrt(6.0/(fan_in + fan_out)) # {sigmoid:4, tanh:1} 
        high = 4*np.sqrt(6.0/(fan_in + fan_out))
        return tf.Variable(tf.random_uniform(shape, minval=low, maxval=high, dtype=tf.float32))


def main(argv=None):
    # Be verbose?
    verbose = FLAGS.verbose

    # Plot? 
    plot = FLAGS.plot
    
    # Get the data.
    train_data_filename = FLAGS.train
    test_data_filename = FLAGS.test

    # load data
    train_data,train_labels = extract_data(train_data_filename)
    test_data, test_labels = extract_data(test_data_filename)

    # Get the shape of the training data.
    train_size,num_features = train_data.shape

    # Get the number of epochs for training.
    num_epochs = FLAGS.num_epochs

    # Get the size of layer one.
    num_hidden = FLAGS.num_hidden
 
    # This is where training samples and labels are fed to the graph.
    with tf.name_scope('inputLayers'):
        x = tf.placeholder("float", shape=[None, num_features], name='x_input')
        y_ = tf.placeholder("float", shape=[None, NUM_LABELS], name= 'y_input')
    
        # For the test data, hold the entire dataset in one constant node.
        test_data_node = tf.constant(test_data)


    # The hidden layer.
    with tf.name_scope('hiddenLayer'):
        w_hidden = init_weights(
            [num_features, num_hidden],'xavier',
            xavier_params=(num_features, num_hidden))

        b_hidden = init_weights([1,num_hidden],'zeros')
        hidden = tf.nn.tanh(tf.matmul(x,w_hidden) + b_hidden)


    # The output layer.
    with tf.name_scope('outputLayer'):
        # Initialize the output weights and biases.
        w_out = init_weights(
            [num_hidden, NUM_LABELS],'xavier',
            xavier_params=(num_hidden, NUM_LABELS))   

        b_out = init_weights([1,NUM_LABELS],'zeros')
        y = tf.nn.softmax(tf.matmul(hidden, w_out) + b_out)
    

    # Optimization.
    with tf.name_scope('loss'):
        cross_entropy = tf.reduce_mean(-tf.reduce_sum(y_*tf.log(y)))
        tf.summary.scalar('loss', cross_entropy)

    with tf.name_scope('train'):
        train_step = tf.train.GradientDescentOptimizer(0.01).minimize(cross_entropy)
    
    # Evaluation.
    predicted_class = tf.argmax(y,1);
    correct_prediction = tf.equal(tf.argmax(y,1), tf.argmax(y_,1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, "float"))

    # Create a local session to run this computation.
    with tf.Session() as sess:
        # Run all the initializers to prepare the trainable parameters.
        merged = tf.summary.merge_all()
        train_writer = tf.summary.FileWriter("logs/train", sess.graph)
        test_writer = tf.summary.FileWriter("logs/test", sess.graph)

    	if verbose:
    	    print ('Initialized!')
    	    print ('Training.')
    	    
    	# Iterate and train.
        Iteratation_num = num_epochs * train_size // BATCH_SIZE
        print(Iteratation_num)

        # initail all and run
        tf.global_variables_initializer().run()

    	for step in range(num_epochs * train_size // BATCH_SIZE):
    	    if verbose:
    	        print (step),
    	           
    	    offset = (step * BATCH_SIZE) % train_size
    	    batch_data = train_data[offset:(offset + BATCH_SIZE), :]
    	    batch_labels = train_labels[offset:(offset + BATCH_SIZE)]
    	    _,loss = sess.run([train_step, cross_entropy], feed_dict={x: batch_data, y_: batch_labels})

    	    if verbose and offset >= train_size-BATCH_SIZE:
                print('verbose and offset >= train_size-BATCH_SIZE')

            print('step %d: loss = %.2f' %(step, loss))
            if step % 50 == 0:
                # record loss
                train_result = sess.run(merged, feed_dict={x: batch_data, y_: batch_labels})
                test_result = sess.run(merged, feed_dict={x: test_data, y_: test_labels})
                train_writer.add_summary(train_result, step)
                test_writer.add_summary(test_result, step)


    	print ("Accuracy is :", accuracy.eval(feed_dict={x: test_data, y_: test_labels}))

        if plot:
            eval_fun = lambda X: predicted_class.eval(feed_dict={x:X});
            plot_boundary_on_data.plot(test_data, test_labels, eval_fun)
            

if __name__ == '__main__':
    tf.app.run()

import socket
import time
import struct
import io
import _thread
from PIL import Image

import cv2
import numpy as np
import os
import pathlib
import tensorflow as tf
import argparse
import time
import os

from object_detection.utils import label_map_util
from object_detection.utils import visualization_utils as viz_utils

import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings('ignore')   # Suppress Matplotlib warnings
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'    # Suppress TensorFlow logging (1)
tf.get_logger().setLevel('ERROR')           # Suppress TensorFlow logging (2)

from img2pdf import oneframe
from final_main import loadmodel

dictionary = {
  "1" : "6",
  "2" : "7",
  "3" : "8",
  "4" : "9",
  "5" : "10",
  "6" : "4",
  "7" : "3",
  "8" : "1",
  "9" : "2",
  "10" : "11",
  "11" : "12",
  "12" : "13",
  "13" : "14",
  "14" : "15",
  "15" : "5"
}


tf.get_logger().setLevel('ERROR')           # Suppress TensorFlow logging (2)

SERVER_HOST = '192.168.7.7'  # The server's hostname or IP address
SERVER_PORT = 8090        # The port used by the server
SOCKET_BUFFER_SIZE = 512 

class Client_Algorithm:
	def __init__(self, host = SERVER_HOST, port = SERVER_PORT):
		self.server_host = SERVER_HOST
		self.server_port = SERVER_PORT

		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		#self.socket.bind((self.host, self.port))

		self.socket.connect((self.server_host, self.server_port))
		#self.socket.listen(1)
		self.connection = self.socket.makefile('rb')

	def start(self):
		_thread.start_new_thread(self.testAlgo)
		_thread.start_new_thread(self.write, ())

	def testAlgo(self):
		while 1:
			print("testing")
			time.sleep(5)

	def read(self):
		message = self.socket.recv(SOCKET_BUFFER_SIZE).strip()
		print("From RPI: ")
		print("\t" + repr(message))
		return repr(message)

	def write(self, message):
		print("To RPI: ")
		print("\t" + message)
		self.socket.sendall(message.encode('utf-8'))

	def recieveImage(self, counter):
		image_len = struct.unpack('<L',self.connection.read(struct.calcsize('<L')))[0]
		image_stream = io.BytesIO()
		image_stream.write(self.connection.read(image_len))
		image_stream.seek(0)
		image = Image.open(image_stream)
		image.show()
		image.save("/Users/Darren/Desktop/NTU /y3s1/CZ3004 - MDP/real_pi/cv/lib/python3.7/site-packages/tensorflow/models/workspace/training_demo/images/" + str(counter) + ".jpeg")

	def imageprocessing(self, counter):
		image_path = 'images/' + str(counter) + '.jpeg'
		image = cv2.imread(image_path)
		image = cv2.resize(image, (0,0), fx=0.9, fy=0.9)		
		image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
		image_expanded = np.expand_dims(image_rgb, axis=0)

		# The input needs to be a tensor, convert it using `tf.convert_to_tensor`.
		input_tensor = tf.convert_to_tensor(image)
		# The model expects a batch of images, so add an axis with `tf.newaxis`.
		input_tensor = input_tensor[tf.newaxis, ...]

		# input_tensor = np.expand_dims(image_np, 0)
		detections = detect_fn(input_tensor)

		# All outputs are batches tensors.
		# Convert to numpy arrays, and take index [0] to remove the batch dimension.
		# We're only interested in the first num_detections.
		num_detections = int(detections.pop('num_detections'))
		detections = {key: value[0, :num_detections].numpy()
						for key, value in detections.items()}
		detections['num_detections'] = num_detections

		# detection_classes should be ints.
		detections['detection_classes'] = detections['detection_classes'].astype(np.int64)

		image_with_detections = image.copy()

		# SET MIN_SCORE_THRESH BASED ON YOU MINIMUM THRESHOLD FOR DETECTIONS
		viz_utils.visualize_boxes_and_labels_on_image_array(
				image_with_detections,
				detections['detection_boxes'],
				detections['detection_classes'],
				detections['detection_scores'],
				category_index,
				use_normalized_coordinates=True,
				max_boxes_to_draw=1,
				min_score_thresh=0.60,
				agnostic_mode=False)

		print('Done')
		# print ([category_index.get(value) for index,value in enumerate(detections['detection_classes'][0]) if detections['detection_scores'][0,index] > 0.55])

		# #Converting the image class to a string for passing out.
		#Converting the image class to a string for passing out.
		if (detections['detection_scores'][0] < 0.6):
			print('Nothing is detected')
			return "!"
		else:
			x = str(detections['detection_classes'][0])
			print(dictionary.get(x))
			cv2.imwrite(os.path.join('./processed_images/' , str(counter)+'.jpeg'), image_with_detections)
			return x



		# # DISPLAYS OUTPUT IMAGE
		# cv2.imshow('Object Detector', image_with_detections)
		# cv2.imwrite(os.path.join('./processed_images/' , str(counter)+'.jpeg'), image_with_detections)

		# # CLOSES WINDOW ONCE KEY IS PRESSED
		# cv2.waitKey(0)
		# CLEANUP
		# cv2.destroyAllWindows()

client = Client_Algorithm()
counter = 0
#Loading of the model and getting required variables
detect_fn , category_index = loadmodel()

running_program =  True 

while running_program:
	client.recieveImage(counter)
	result = client.imageprocessing(counter)
	counter = counter + 1
	client.write(result)

#while 1: 
#	if (counter <= 4):
#		client.recieveImage(counter)
#		client.imageprocessing(counter)
#		counter = counter + 1 
#	else:
#		break

#Concatenation of images
oneframe()
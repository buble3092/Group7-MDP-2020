import cv2
import numpy as np
import os
import pathlib
import tensorflow as tf
import argparse
import time

from object_detection.utils import label_map_util
from object_detection.utils import visualization_utils as viz_utils

from PIL import Image
import matplotlib.pyplot as plt
import warnings
warnings.filterwarnings('ignore')   # Suppress Matplotlib warnings
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'    # Suppress TensorFlow logging (1)
tf.get_logger().setLevel('ERROR')           # Suppress TensorFlow logging (2)

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
#importing functions from other scripts
from img2pdf import oneframe

parser = argparse.ArgumentParser()
parser.add_argument('--model', help='Folder that the Saved Model is Located In',
                    default='exported-models/my_mobilenet_model')
parser.add_argument('--labels', help='Where the Labelmap is Located',
                    default='exported-models/my_mobilenet_model/saved_model/label_map.pbtxt')
parser.add_argument('--image', help='Name of the single image to perform detection on',
                    default='images/train/9.jpeg')
parser.add_argument('--threshold', help='Minimum confidence threshold for displaying detected objects',
                    default=0.60)
                    
args = parser.parse_args()
# Enable GPU dynamic memory allocation
gpus = tf.config.experimental.list_physical_devices('GPU')
for gpu in gpus:
    tf.config.experimental.set_memory_growth(gpu, True)

# PROVIDE PATH TO IMAGE DIRECTORY
IMAGE_PATHS = args.image

# PROVIDE PATH TO MODEL DIRECTORY
PATH_TO_MODEL_DIR = args.model

# PROVIDE PATH TO LABEL MAP
PATH_TO_LABELS = args.labels

# PROVIDE THE MINIMUM CONFIDENCE THRESHOLD
MIN_CONF_THRESH = float(args.threshold)

#Defining the loading of model 
# LOAD THE MODEL
def loadmodel():
    
    PATH_TO_SAVED_MODEL = PATH_TO_MODEL_DIR + "/saved_model"

    print('Loading model...', end='')
    start_time = time.time()

    # LOAD SAVED MODEL AND BUILD DETECTION FUNCTION
    detect_fn = tf.saved_model.load(PATH_TO_SAVED_MODEL)

    end_time = time.time()
    elapsed_time = end_time - start_time
    print('Done! Took {} seconds'.format(elapsed_time))

    # LOAD LABEL MAP DATA FOR PLOTTING

    category_index = label_map_util.create_category_index_from_labelmap(PATH_TO_LABELS,
                                                                        use_display_name=True)

    print('\n')
    return detect_fn, category_index

#Performing the loading of model

# detect_fn , category_index = loadmodel()

# counter = 0


    
#   image_path = 'images/' + str(counter) + '.jpeg'
#   image = cv2.imread(image)
#   image = cv2.resize(image,(480,480))
#   image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
#   image_expanded = np.expand_dims(image_rgb, axis=0)

#   # The input needs to be a tensor, convert it using `tf.convert_to_tensor`.
#   input_tensor = tf.convert_to_tensor(image)
#   # The model expects a batch of images, so add an axis with `tf.newaxis`.
#   input_tensor = input_tensor[tf.newaxis, ...]

#   # input_tensor = np.expand_dims(image_np, 0)
#   detections = detect_fn(input_tensor)

#   # All outputs are batches tensors.
#   # Convert to numpy arrays, and take index [0] to remove the batch dimension.
#   # We're only interested in the first num_detections.
#   num_detections = int(detections.pop('num_detections'))
#   detections = {key: value[0, :num_detections].numpy()
#                 for key, value in detections.items()}
#   detections['num_detections'] = num_detections

#   # detection_classes should be ints.
#   detections['detection_classes'] = detections['detection_classes'].astype(np.int64)

#   image_with_detections = image.copy()

#   # SET MIN_SCORE_THRESH BASED ON YOU MINIMUM THRESHOLD FOR DETECTIONS
#   viz_utils.visualize_boxes_and_labels_on_image_array(
#         image_with_detections,
#         detections['detection_boxes'],
#         detections['detection_classes'],
#         detections['detection_scores'],
#         category_index,
#         use_normalized_coordinates=True,
#         max_boxes_to_draw=200,
#         min_score_thresh=MIN_CONF_THRESH,
#         agnostic_mode=False)

#   print('Done')

#   #Converting the image class to a string for passing out.

#   x = str(detections['detection_classes'][0])
#   print(type(x))
#   print(dictionary.get(x))


#   # # DISPLAYS OUTPUT IMAGE
#   cv2.imshow('Object Detector', image_with_detections)
#   # CLOSES WINDOW ONCE KEY IS PRESSED
#   cv2.waitKey(0)
#   # CLEANUP
#   cv2.destroyAllWindows()
#   counter = counter + 1


# # #Loading the image/images
# # image = cv2.imread(IMAGE_PATHS)
# # image = cv2.resize(image,(480,480))
# # image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
# # image_expanded = np.expand_dims(image_rgb, axis=0)

# # # The input needs to be a tensor, convert it using `tf.convert_to_tensor`.
# # input_tensor = tf.convert_to_tensor(image)
# # # The model expects a batch of images, so add an axis with `tf.newaxis`.
# # input_tensor = input_tensor[tf.newaxis, ...]

# # # input_tensor = np.expand_dims(image_np, 0)
# # detections = detect_fn(input_tensor)

# # # All outputs are batches tensors.
# # # Convert to numpy arrays, and take index [0] to remove the batch dimension.
# # # We're only interested in the first num_detections.
# # num_detections = int(detections.pop('num_detections'))
# # detections = {key: value[0, :num_detections].numpy()
# #                for key, value in detections.items()}
# # detections['num_detections'] = num_detections

# # # detection_classes should be ints.
# # detections['detection_classes'] = detections['detection_classes'].astype(np.int64)

# # image_with_detections = image.copy()

# # # SET MIN_SCORE_THRESH BASED ON YOU MINIMUM THRESHOLD FOR DETECTIONS
# # viz_utils.visualize_boxes_and_labels_on_image_array(
# #       image_with_detections,
# #       detections['detection_boxes'],
# #       detections['detection_classes'],
# #       detections['detection_scores'],
# #       category_index,
# #       use_normalized_coordinates=True,
# #       max_boxes_to_draw=200,
# #       min_score_thresh=MIN_CONF_THRESH,
# #       agnostic_mode=False)

# # print('Done')

# # #Converting the image class to a string for passing out.

# # x = str(detections['detection_classes'][0])
# # print(type(x))
# # print(dictionary.get(x))


# # # # DISPLAYS OUTPUT IMAGE
# # cv2.imshow('Object Detector', image_with_detections)
# # # CLOSES WINDOW ONCE KEY IS PRESSED
# # cv2.waitKey(0)
# # # CLEANUP
# # cv2.destroyAllWindows()
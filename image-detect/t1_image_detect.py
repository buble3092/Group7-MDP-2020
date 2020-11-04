import time
from PIL import Image
import argparse
import numpy as np
import os
import cv2
import tensorflow.compat.v1 as tf
from object_detection.utils import label_map_util
from object_detection.utils import visualization_utils as vis_util


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
  "15" : "5",
  "16" : "Stop"
}


parser = argparse.ArgumentParser()
parser.add_argument('--model', help='Folder that the Saved Model is Located In',
                    default='exported-models/fine_tuned_model/frozen_inference_graph.pb')
parser.add_argument('--labels', help='Where the Labelmap is Located',
                    default='exported-models/fine_tuned_model/saved_model/label_map.pbtxt')
parser.add_argument('--image', help='Name of the single image to perform detection on',
                    default='images/train/9.jpeg')
parser.add_argument('--threshold', help='Minimum confidence threshold for displaying detected objects',
                    default=0.50)

args = parser.parse_args()
# number of classes 
NUM_CLASSES = 16

# PROVIDE PATH TO IMAGE DIRECTORY
IMAGE_PATHS = args.image


# PROVIDE PATH TO MODEL DIRECTORY
PATH_TO_FROZEN_GRAPH = args.model

# PROVIDE PATH TO LABEL MAP
PATH_TO_LABEL_MAP = args.labels

# PROVIDE THE MINIMUM CONFIDENCE THRESHOLD
MIN_CONF_THRESH = float(args.threshold)

#reads the frozen graph

# print('Loading model...', end='')
# start_time = time.time()

detection_graph = tf.Graph()
with detection_graph.as_default():
    od_graph_def = tf.GraphDef()
    with tf.gfile.GFile(PATH_TO_FROZEN_GRAPH, 'rb') as fid:
        serialized_graph = fid.read()
        od_graph_def.ParseFromString(serialized_graph)
        tf.import_graph_def(od_graph_def, name='')



label_map = label_map_util.load_labelmap(PATH_TO_LABEL_MAP)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)

# end_time = time.time()
# elapsed_time = end_time - start_time
# print('Done! Took {} seconds'.format(elapsed_time))

def load_image_into_numpy_array(image):
    (im_width, im_height) = image.size
    return np.array(image.getdata()).reshape(
        (im_height, im_width, 3)).astype(np.uint8)

def run_inference_for_single_image(image, graph):
    with graph.as_default():
        with tf.Session() as sess:
            # Get handles to input and output tensors
            ops = tf.get_default_graph().get_operations()
            all_tensor_names = {
                output.name for op in ops for output in op.outputs}
            tensor_dict = {}
            for key in [
                'num_detections', 'detection_boxes', 'detection_scores',
                'detection_classes', 'detection_masks'
            ]:
                tensor_name = key + ':0'
                if tensor_name in all_tensor_names:
                    tensor_dict[key] = tf.get_default_graph().get_tensor_by_name(
                        tensor_name)
            if 'detection_masks' in tensor_dict:
                # The following processing is only for single image
                detection_boxes = tf.squeeze(
                    tensor_dict['detection_boxes'], [0])
                detection_masks = tf.squeeze(
                    tensor_dict['detection_masks'], [0])
                # Reframe is required to translate mask from box coordinates to image coordinates and fit the image size.
                real_num_detection = tf.cast(
                    tensor_dict['num_detections'][0], tf.int32)
                detection_boxes = tf.slice(detection_boxes, [0, 0], [
                                           real_num_detection, -1])
                detection_masks = tf.slice(detection_masks, [0, 0, 0], [
                                           real_num_detection, -1, -1])
                detection_masks_reframed = utils_ops.reframe_box_masks_to_image_masks(
                    detection_masks, detection_boxes, image.shape[0], image.shape[1])
                detection_masks_reframed = tf.cast(
                    tf.greater(detection_masks_reframed, 0.5), tf.uint8)
                # Follow the convention by adding back the batch dimension
                tensor_dict['detection_masks'] = tf.expand_dims(
                    detection_masks_reframed, 0)
            image_tensor = tf.get_default_graph().get_tensor_by_name('image_tensor:0')

            # Run inference
            output_dict = sess.run(tensor_dict,
                                   feed_dict={image_tensor: np.expand_dims(image, 0)})

            # all outputs are float32 numpy arrays, so convert types as appropriate
            output_dict['num_detections'] = int(
                output_dict['num_detections'][0])
            output_dict['detection_classes'] = output_dict[
                'detection_classes'][0].astype(np.uint8)
            output_dict['detection_boxes'] = output_dict['detection_boxes'][0]
            output_dict['detection_scores'] = output_dict['detection_scores'][0]
            if 'detection_masks' in output_dict:
                output_dict['detection_masks'] = output_dict['detection_masks'][0]
    return output_dict


# Detection

# print('taking time to check how long to detect image')
# start_time = time.time()

with detection_graph.as_default():
    with tf.Session(graph=detection_graph) as sess:
        image = Image.open(IMAGE_PATHS)
        image_np = load_image_into_numpy_array(image)
        # Expand dimensions since the model expects images to have shape: [1, None, None, 3]
        image_np_expanded = np.expand_dims(image_np, axis=0)
        #Actual detection.
        output_dict = run_inference_for_single_image(image_np, detection_graph)
        #Visualization of the results of a detection.
        vis_util.visualize_boxes_and_labels_on_image_array(
            image_np,
            output_dict['detection_boxes'],
            output_dict['detection_classes'],
            output_dict['detection_scores'],
            category_index,
            use_normalized_coordinates=True,
            line_thickness=10,
            max_boxes_to_draw=3,
            min_score_thresh=MIN_CONF_THRESH)

# end_time = time.time()
# elapsed_time = end_time - start_time
# print('Done! Took {} seconds'.format(elapsed_time))

#Converting the image class to a string for passing out.
if (output_dict['detection_scores'][0] < MIN_CONF_THRESH):
    print('Nothing is detected')
else:
    x = str(output_dict['detection_classes'][0])
    print(dictionary.get(x))


img_rgb = cv2.cvtColor(image_np, cv2.COLOR_BGR2RGB)
# Display output
cv2.imshow('Object', cv2.resize(img_rgb, (0,0), fx = 0.5, fy = 0.5))
# CLOSES WINDOW ONCE KEY IS PRESSED
cv2.waitKey(0)
# CLEANUP
cv2.destroyAllWindows()


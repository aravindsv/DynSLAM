import os
import sys
import random
import math
import numpy as np
import skimage.io
import matplotlib
import matplotlib.pyplot as plt
from typing import List
import time
import tensorflow as tf
from keras import backend

dirname = os.path.dirname(__file__)

# Root directory of the project
ROOT_DIR = os.path.abspath(os.path.join(dirname, "..", 'Mask_RCNN'))

# Import Mask RCNN
sys.path.append(ROOT_DIR)  # To find local version of the library
from mrcnn import utils
import mrcnn.model as modellib
from mrcnn import visualize

coco_path = os.path.join(ROOT_DIR, "samples", "coco")
print(coco_path)
sys.path.append(coco_path)  # To find local version
import coco


class InferenceConfig(coco.CocoConfig):
    # Set batch size to 1 since we'll be running inference on
    # one image at a time. Batch size = GPU_COUNT * IMAGES_PER_GPU
    GPU_COUNT = 1
    IMAGES_PER_GPU = 1

def make_segmenter():


    # Directory to save logs and trained model
    MODEL_DIR = os.path.join(ROOT_DIR, "logs")

    # Local path to trained weights file
    COCO_MODEL_PATH = os.path.join(ROOT_DIR, "mask_rcnn_coco.h5")
    # Download COCO trained weights from Releases if needed
    if not os.path.exists(COCO_MODEL_PATH):
        utils.download_trained_weights(COCO_MODEL_PATH)

    # Directory of images to run detection on
    # IMAGE_DIR = os.path.join(ROOT_DIR, "images")

    config = InferenceConfig()
    config.display()

    # Create model object in inference mode.
    model = modellib.MaskRCNN(mode="inference", model_dir=MODEL_DIR, config=config)

    # Load weights trained on MS-COCO
    model.load_weights(COCO_MODEL_PATH, by_name=True)

    graph = tf.get_default_graph()

    return Segmenter(model, graph)

class Segmentation:

    def __init__(self, result):

        # may want to only save parts of this...
        self.result = result

    def get_mask(self, x, y, ctx = 'no context'):

        mask_mat = self.result['masks']
        roi_mat = self.result['rois']
        class_id_array = self.result['class_ids']


        if mask_mat is None:
            print(f'masks not found. ctx = {ctx}')

        best_mask = None
        best_dist = np.inf

        for idx in range(mask_mat.shape[2]):

            # TODO: temporarily ignoring non-cars
            # if class_id_array[idx] not in [3, 8]:
            #     continue

            mask = mask_mat[:,:,idx]
            roi = roi_mat[idx,:]

            roi_y = (roi[0] + roi[2])/2
            roi_x = (roi[1] + roi[3])/2

            dist = np.sqrt((roi_x - x)**2 + (roi_y - y)**2)

            # TODO: would be better to do hungarian matching in some cases.
            if dist < best_dist:
                best_dist = dist
                best_mask = mask

            # maybe try this first.
            # if mask[int(y), int(x)] == True:
            #     return mask


        return best_mask

class Segmenter:

    model: modellib.MaskRCNN

    def __init__(self, model, graph):

        self.model = model
        self.graph = graph

    def segment(self, image):

        # backend.clear_session()

        with self.graph.as_default():
            result = self.model.detect([image], verbose=1)[0]

        print(f'result = {result}')

        return result

        seg_list = []
        for image_path in image_path_list:
            t1 = time.perf_counter()
            image = skimage.io.imread(image_path)
            t2 = time.perf_counter()

            result = self.model.detect([image], verbose=1)[0]

            # TODO: filter out non-car results to save memory

            t3 = time.perf_counter()
            seg = Segmentation(result)
            seg_list.append(seg)

            print(f'timers: imread = {t2 - t1:.1f}, detect = {t3 - t1:.1f}')

        #
        # image_list = []
        #
        # for image_path in image_path_list:
        #     image = skimage.io.imread(image_path)
        #     image_list.append(image)
        #
        # result_list = self.model.detect(image_list, verbose=1)
        #
        # seg_list = []
        #
        # for result in result_list:
        #     seg = Segmentation(result)
        #     seg_list.append(seg)

        return seg_list


def main():


    # COCO Class names
    # Index of the class in the list is its ID. For example, to get ID of
    # the teddy bear class, use: class_names.index('teddy bear')
    class_names = ['BG', 'person', 'bicycle', 'car', 'motorcycle', 'airplane',
                   'bus', 'train', 'truck', 'boat', 'traffic light',
                   'fire hydrant', 'stop sign', 'parking meter', 'bench', 'bird',
                   'cat', 'dog', 'horse', 'sheep', 'cow', 'elephant', 'bear',
                   'zebra', 'giraffe', 'backpack', 'umbrella', 'handbag', 'tie',
                   'suitcase', 'frisbee', 'skis', 'snowboard', 'sports ball',
                   'kite', 'baseball bat', 'baseball glove', 'skateboard',
                   'surfboard', 'tennis racket', 'bottle', 'wine glass', 'cup',
                   'fork', 'knife', 'spoon', 'bowl', 'banana', 'apple',
                   'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza',
                   'donut', 'cake', 'chair', 'couch', 'potted plant', 'bed',
                   'dining table', 'toilet', 'tv', 'laptop', 'mouse', 'remote',
                   'keyboard', 'cell phone', 'microwave', 'oven', 'toaster',
                   'sink', 'refrigerator', 'book', 'clock', 'vase', 'scissors',
                   'teddy bear', 'hair drier', 'toothbrush']

    # Load a random image from the images folder
    file_names = next(os.walk(IMAGE_DIR))[2]
    # image = skimage.io.imread(os.path.join(IMAGE_DIR, random.choice(file_names)))
    img_path = r"C:\Users\catph\data\kitti_raw\sync\kitti_raw_data\data\2011_09_26\2011_09_26_drive_0001_sync\image_02\data\0000000030.png"

    image = skimage.io.imread(img_path)
    # Run detection
    results = model.detect([image], verbose=1)

    # Visualize results
    r = results[0]
    visualize.display_instances(image, r['rois'], r['masks'], r['class_ids'],
                                class_names, r['scores'])



if __name__ == '__main__':
    main()
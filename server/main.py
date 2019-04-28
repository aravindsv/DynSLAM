from flask import Flask
from flask import request, make_response
import numpy as np
import time
import cv2
from flask_compress import Compress

import segment

import server_result_pb2

OCTET_STREAM = 'application/octet-stream'

app = Flask(__name__)

app.config['COMPRESS_MIMETYPES'] = [
    'text/html',
    'text/css',
    'text/xml',
    'application/json',
    'application/javascript',
    'application',
    OCTET_STREAM
]

Compress(app)


segmenter = segment.make_segmenter()

def make_server_mat(array):

    server_mat = server_result_pb2.ServerMat()

    server_mat.array = array.tobytes()
    server_mat.dim1 = array.shape[0]
    server_mat.dim2 = array.shape[1]

    return server_mat


def serialize_segments(segments):

    result = server_result_pb2.ServerResult()

    for idx in range(segments['masks'].shape[2]):
        mask = segments['masks'][:,:,idx]
        class_id = segments['class_ids'][idx]
        class_prob = segments['scores'][idx]

        # roi format is y0, x0 y1, x1
        roi = segments['rois'][idx]

        # extract region of interest of mask only.
        mask = mask[roi[0]:(roi[2]+1), roi[1]:(roi[3]+1)]

        detect = result.detections.add()
        detect.class_probability = class_prob
        detect.class_id = class_id
        detect.mask.mat.CopyFrom(make_server_mat(mask))
        detect.mask.bbox.y0 = roi[0]
        detect.mask.bbox.x0 = roi[1]
        detect.mask.bbox.y1 = roi[2]
        detect.mask.bbox.x1 = roi[3]

    result_bytes = result.SerializeToString()

    return result_bytes

@app.route('/post', methods=['POST'])
def hello_world():

    t1 = time.perf_counter()

    # print(f'len = {request.content_length}')
    # print(request.files)


    td1 = time.perf_counter()
    data = request.files['server_input'].read()
    td2 = time.perf_counter()

    print(f'read data runtime = {td2 - td1}')

    # img = request.form['image']

    # print(data[0:10])
    # print(type(data))
    # print(type(img))

    t7 = time.perf_counter()
    server_input = server_result_pb2.ServerInput()


    server_input.ParseFromString(data)

    image_array = np.frombuffer(server_input.image.array, dtype=np.uint8)

    image = cv2.imdecode(image_array, cv2.IMREAD_COLOR)

    # image = np.frombuffer(server_input.image.array, dtype=np.uint8).reshape(server_input.image.dim1,
    #                                                                        server_input.image.dim2, 3)
    t8 = time.perf_counter()

    print(f'deserialize runtime = {t8 - t7}')

    # print(f'image shape = {image.shape}, dtype={np.dtype}')


    t5 = time.perf_counter()
    segments = segmenter.segment(image)
    t6 = time.perf_counter()

    print(f'segment runtiem = {t6 - t5}')

    t3 = time.perf_counter()

    result_bytes = serialize_segments(segments)

    t4 = time.perf_counter()

    print(f'serialize rutime = {t4 - t3}')

    # print(f'len result = {len(result_bytes)}')

    tr1 = time.perf_counter()
    response = make_response(result_bytes)
    response.headers.set('Content-Type', OCTET_STREAM)

    tr2 = time.perf_counter()
    print(f'make response runtime = {tr2 - tr1}')

    t2 = time.perf_counter()

    print(f'/post runtime = {t2 - t1}')

    return response


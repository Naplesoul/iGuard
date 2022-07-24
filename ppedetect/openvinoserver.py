import json
import time
import socket
import cv2
import numpy as np
from openvino.runtime import Core, Layout, Type
from openvino.preprocess import PrePostProcessor, ColorFormat

import config
from augmentations import letterbox

model_path = "./model/ppe.xml"
class_names = ['glove', 'goggle', 'hand', 'head', 'helmet']
colors = [(34, 139, 34), (46, 84, 8), (84, 46, 8), (0, 69, 255), (0, 0, 139)]


def recv_img(recv: socket.socket):
    bytes, addr = recv.recvfrom(65536)
    bytes = np.asarray(bytearray(bytes), dtype="uint8")
    img = cv2.imdecode(bytes, cv2.IMWRITE_JPEG_QUALITY)
    return img


if __name__ == "__main__":
    core = Core()
    model = core.read_model(model_path)
    ppp = PrePostProcessor(model)

    ppp.input().tensor().set_color_format(ColorFormat.BGR).set_element_type(Type.u8).set_layout(Layout('NHWC'))
    ppp.input().model().set_layout(Layout('NCHW'))
    ppp.output().tensor().set_element_type(Type.f32)
    ppp.input().preprocess().convert_element_type(Type.f32).convert_color(ColorFormat.RGB).mean([0.0, 0.0, 0.0]).scale([255.0, 255.0, 255.0]) 

    print(f'Build preprocessor: {ppp}')
    model = ppp.build()
    net = core.compile_model(model, "AUTO")

    recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv.bind(("0.0.0.0", config.listen_port))
    print(f"openvino infer server listening on port {config.listen_port}")
    send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    while True:
        img = recv_img(recv)
        start_time = time.time()
        letterbox_img, ratio, (dw, dh) = letterbox(img, auto=False)
        input_tensor = np.expand_dims(letterbox_img, axis=0)
        predictions = net([input_tensor])[net.outputs[0]]

        class_ids = []
        confidences = []
        boxes = []

        for pred in predictions:
            for i, det in enumerate(pred):
                confidence = det[4]
                scores = det[5:]
                class_id = np.argmax(scores)
                if scores[class_id] > 0.25:
                    confidences.append(confidence)
                    class_ids.append(class_id)
                    x,y,w,h = det[0].item(),det[1].item(),det[2].item(),det[3].item()
                    left = int((x - 0.5*w -dw) / ratio[0])
                    top = int((y - 0.5*h - dh) / ratio[1])
                    width = int(w / ratio[0])
                    height = int(h / ratio[1])
                    box = np.array([left, top, width, height])
                    boxes.append(box)
        print("Detections: " + str(len(predictions)))

        indexes = cv2.dnn.NMSBoxes(boxes, confidences, 0.25, 0.45)

        filtered_ids = []
        filered_confidences = []
        filtered_boxes = []

        for i in indexes:
            filtered_ids.append(class_ids[i])
            filered_confidences.append(confidences[i])
            filtered_boxes.append(boxes[i])
        print("Detections: " + str(len(filtered_ids)))

        has_glove = False
        has_goggle = False
        has_helmet = False
        # Show bbox
        for (class_id, confidence, box) in zip(filtered_ids, filered_confidences, filtered_boxes):
            id = int(class_id)
            if id == 0:
                has_glove = True
            elif id == 1:
                has_goggle = True
            elif id == 4:
                has_helmet = True
            color = colors[id]
            cv2.rectangle(img, box, color, 3)
            cv2.rectangle(img, (box[0], box[1] - 30), (box[0] + box[2], box[1]), color, -1)
            cv2.putText(img, class_names[class_id], (box[0] + 20, box[1] - 5), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
            print(class_names[class_id], confidence, box)
            
        cv2.resize(img, (640, 360), img)
        success, buf = cv2.imencode(".jpg", img, [cv2.IMWRITE_JPEG_QUALITY, 10])

        send.sendto(json.dumps({ "has_helmet": has_helmet, "has_goggle": has_goggle, "has_glove": has_goggle }).encode(), (config.server_ip, config.server_detect_port))
        send.sendto(buf, (config.server_ip, config.server_image_port))

        end_time = time.time()
        print(f"Total process time: {int((end_time - start_time) * 1000)} ms.\n\n")
        # cv2.imwrite("resize.png", img)
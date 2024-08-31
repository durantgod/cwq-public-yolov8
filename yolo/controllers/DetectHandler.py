from controllers.BaseHandler import BaseHandler
import json
import numpy as np
import base64
from ultralytics import YOLO
import cv2
import torch

model = YOLO("../data/yolov8n.pt")
device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
print('device:', device)
model.to(device)

class DetectHandler(BaseHandler):
    async def post(self, *args, **kwargs):
        data = await self.do()
        self.response_json(data)

    async def do(self):
        request_params = self.request_post_params()

        # request_params_copy = request_params
        # del request_params_copy["image_base64"]
        # print(request_params.keys())

        happen = False
        happenScore = 0.0
        detects = []

        image_base64 = request_params.get("image_base64", None)  # 接收base64编码的图片并转换成cv2的图片格式
        if image_base64:
            encoded_image_byte = base64.b64decode(image_base64)
            image_array = np.frombuffer(encoded_image_byte, np.uint8)
            # image = tj.decode(image_array)  # turbojpeg 解码
            image = cv2.imdecode(image_array, cv2.COLOR_RGB2BGR)  # opencv 解码
            results = model.predict(source=[image],verbose=False)
            # results = model([image])
            if len(results) > 0:
                result = results[0]
                names = result.names
                cls_array = result.boxes.cls.cpu().numpy()
                xyxy_array = result.boxes.xyxy.cpu().numpy()
                conf_array = result.boxes.conf.cpu().numpy()
                for i in range(len(cls_array)):
                    class_index = int(cls_array[i])
                    class_name = names[class_index]
                    class_score = float("%.3f" % float(conf_array[i]))
                    box = xyxy_array[i]  # 左上角x,y + 右下角x,y

                    # print(i, class_index, class_name, class_score, box)
                    detect = {
                        "x1": int(box[0]),
                        "y1": int(box[1]),
                        "x2": int(box[2]),
                        "y2": int(box[3]),
                        "class_score": class_score,
                        "class_name": class_name
                    }
                    # print(detect)

                    detects.append(detect)

        if len(detects) > 0:
            happen = True
            happenScore = 1.0

        result = {
            "happen": happen,
            "happenScore": happenScore,
            "detects": detects
        }
        res = {
            "code": 1000,
            "msg": "success",
            "result": result
        }
        return res

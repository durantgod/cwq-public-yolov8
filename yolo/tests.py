from ultralytics import YOLO
from PIL import Image
import cv2
import torch
import time
model = YOLO("data/yolov8n.pt")

device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
print('device:', device)

model.to(device)

# accepts all formats - image/dir/Path/URL/video/PIL/ndarray. 0 for webcam
#results = model.predict(source="0")
#results = model.predict(source="folder", show=True) # Display preds. Accepts all YOLO predict arguments

# from PIL
# im1 = Image.open("../data/bus.jpg")
# results = model.predict(source=im1, save=True, save_txt=True,device='cpu')  # save plotted images

# from ndarray
im2 = cv2.imread("data/bus.jpg")

while True:
    t1 = time.time()
    results = model.predict(source=im2,device=device,verbose=False)
    t2 = time.time()
    # results = model(im2)

    # from list of PIL/ndarray
    # results = model.predict(source=[im1, im2])

    # print("----result----")
    # print(results)
    t_spend = t2 - t1
    print("推理耗时：",t_spend * 1000)
    for image_result in results:
        # 左上角x,y + 右下角x,y
        names = image_result.names
        cls_array = image_result.boxes.cls.cpu().numpy().astype("uint32")
        xyxy_array = image_result.boxes.xyxy.cpu().numpy().astype("uint32")
        conf_array = image_result.boxes.conf.cpu().numpy().astype("float")
        for i in range(len(cls_array)):
            class_index = cls_array[i]
            class_name = names[class_index]
            class_score = conf_array[i]
            box = xyxy_array[i]
            print(i,class_index,class_name,class_score,box)

        # print("------------------------------------")

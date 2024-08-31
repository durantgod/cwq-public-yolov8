# py_yolo8_pytorch_infer
* github代码地址：https://github.com/any12345com/py_yolo8_pytorch_infer

### 环境依赖

| 程序         | 版本      |
| ---------- | ------- |
| python     | 3.10+    |
| 依赖库      | requirements.txt |

### 创建虚拟环境
python -m venv venv

### 切换到虚拟环境
venv\Scripts\activate

### 安装 pytorch-cpu版本依赖库
* pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

### 安装 pytorch-gpu版本依赖库
* pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple
* pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
* 注意：安装pytorch-gpu训练环境，请根据自己的电脑硬件选择cuda版本，比如我上面选择的https://download.pytorch.org/whl/cu121，并非适用所有电脑设备，请根据自己的设备选择


### 介绍
* 基于python+pytorch开发的yolo8模型推理服务，直接运行yolo8的pt格式的模型文件，并对外提供图片分析的接口服务

### 启动
~~~
//启动服务
python main.py

默认端口: 9702
~~~

### 测试

~~~
//启动测试yolo8模型的脚本，默认加载data/yolov8n.pt模型文件，默认计算的图片是data/bus.jpg
python tests.py

~~~

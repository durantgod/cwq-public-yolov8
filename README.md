# hhd-link-ai-video

航鸿达科技-视频ai识别项目

### 整个YoloV8视频识别系统主要包括：admin管理端，media流媒体端，分析端analyzer(c++), 模型端yolo
![](https://wqknowledge.oss-cn-shenzhen.aliyuncs.com/LLM/yolov8.svg)

效果图：
![](https://wqknowledge.oss-cn-shenzhen.aliyuncs.com/LLM/xg1.png)

### 1、admin 启动
 进入到admin目录下执行：
```shell
python -m venv venv

### 切换到虚拟环境
venv\Scripts\activate

pip install -r .\requirements.txt

//启动后台管理服务
python manage.py runserver 0.0.0.0:9001

后台管理服务：9001
流媒体服务器：9002
分析器：9003

//管理员用户
admin/admin888
```

### 2、analyzer 启动
```shell
## 使用Visual stadio启动c++项目 打开

Analyzer.sln
```
启动后的界面（端口默认9703）：
![](https://wqknowledge.oss-cn-shenzhen.aliyuncs.com/LLM/vsruna.png)


### 3、mediaServer 启动
点击mediaServer下的exe启动即可

### 4、启动yolo
```shell
### 创建虚拟环境
python -m venv venv

### 切换到虚拟环境
venv\Scripts\activate

### 安装 pytorch-cpu版本依赖库
pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

## 启动服务
python main.py

默认端口: 9702
```

### 5、使用过程中的设置如何布控，如何选定识别算法等，如下：
![](https://wqknowledge.oss-cn-shenzhen.aliyuncs.com/LLM/sz1.png)


- 注意！！！ 以下文件夹中压缩文件需要解压：
  - analyzer\3rdparty\onnxruntime\lib
  - cwq-public-yolov8\demo\exe
  - analyzer/3rdparty/opencv/x64/vc15/bin/

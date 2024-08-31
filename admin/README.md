# 视频行为分析系统后台管理

如果要跑完整项目可以参考blog:
https://www.wqknowledge.top/posts/%E5%A4%A7%E6%A8%A1%E5%9E%8B/videoAnalyzer.html

![](https://wqknowledge.oss-cn-shenzhen.aliyuncs.com/LLM/yolo.svg)

![](https://wqknowledge.oss-cn-shenzhen.aliyuncs.com/LLM/videoadmin.png)


#### 安装
| 程序         | 版本              |
| ---------- |-----------------|
| python     | 3.10            |
| 依赖库      | requirements.txt |

#### 启动配置

~~~
### 创建虚拟环境
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

~~~


#### linux 创建python虚拟环境
~~~

# 创建虚拟环境
python -m venv venv

# 激活虚拟环境
source venv/bin/activate

# 更新虚拟环境的pip版本
python -m pip install --upgrade pip -i https://pypi.tuna.tsinghua.edu.cn/simple

# 在虚拟环境中安装依赖库
python -m pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

~~~

#### windows 创建python虚拟环境
~~~
# 创建虚拟环境
python -m venv venv

# 切换到虚拟环境
venv\Scripts\activate

# 更新虚拟环境的pip版本
python -m pip install --upgrade pip -i https://pypi.tuna.tsinghua.edu.cn/simple

# 在虚拟环境中安装依赖库
python -m pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

~~~


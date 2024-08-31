import json
import os
import time
from datetime import datetime
from app.utils.ZLMediaKit import ZLMediaKit
from app.utils.Analyzer import Analyzer
from app.utils.DjangoSql import DjangoSql
from app.utils.Config import Config
from app.models import *
from django.http import HttpResponse

g_config = Config()
g_media = ZLMediaKit(config=g_config)
g_analyzer = Analyzer(g_config.analyzerHost)
g_djangoSql = DjangoSql()
g_session_key_user = "user"


def getUser(request):
    user = request.session.get(g_session_key_user)
    return user

def get_algorithm_data():
    data = g_djangoSql.select("select * from av_algorithm")


    return data
def GetStream(app, name):
    __mediaInfo = g_media.getMediaInfo(app=app, name=name)
    is_online = 0
    video_codec_name = ""
    video_width = 0
    video_height = 0
    if __mediaInfo.get("ret"):
        is_online = 1
        video_codec_name = __mediaInfo.get("video_codec_name")  # 视频编码格式
        video_width = __mediaInfo.get("video_width")
        video_height = __mediaInfo.get("video_height")

    stream = {
        "is_online": is_online,
        # "code": code,
        "app": app,
        "name": name,
        # "produce_speed": produce_speed,
        # "video": video_str,
        "video_codec_name": video_codec_name,
        "video_width": video_width,
        "video_height": video_height,
        # "audio": audio_str,
        # "originUrl": d.get("originUrl"),  # 推流地址
        # "originType": d.get("originType"),  # 推流地址采用的推流协议类型
        # "originTypeStr": d.get("originTypeStr"),  # 推流地址采用的推流协议类型（字符串）
        # "clients": d.get("totalReaderCount"),  # 客户端总数量
        # "schemas_clients": schemas_clients,
        # "videoUrl": g_media.get_wsMp4Url(app, name),
        "wsHost": g_media.get_wsHost(),
        "wsMp4Url": g_media.get_wsMp4Url(app, name),
        "wsFlvUrl": g_media.get_wsFlvUrl(app, name),
        "httpMp4Url": g_media.get_httpMp4Url(app, name),
        "httpFlvUrl": g_media.get_httpFlvUrl(app, name),
        "rtspUrl": g_media.get_rtspUrl(app, name)
    }
    return stream

def readAllStreamData():
    data = g_djangoSql.select("select * from av_stream order by id desc")
    return data

def AllStreamStartForward():
    __ret = False
    __msg = "未知错误"

    try:
        online_data = g_media.getMediaList()
        online_dict = {}
        mediaServerState = g_media.mediaServerState
        if not mediaServerState:
            # 流媒体服务不在线，全部更新下线状态
            g_djangoSql.execute("update av_stream set forward_state=0")
            __msg = "流媒体服务不在线，无法开启转发！"
        else:
            for d in online_data:
                app_name = "{app}_{name}".format(app=d["app"], name=d["name"])
                online_dict[app_name] = d
            streams = Stream.objects.all()

            successCount = 0
            errorCount = 0
            for stream in streams:
                stream_app_name = "{app}_{name}".format(app=stream.app, name=stream.name)
                if online_dict.get(stream_app_name):  # 当前流已经在线，不用再次请求转发
                    successCount += 1
                else:
                    __media_ret = g_media.addStreamProxy(app=stream.app,
                                                         name=stream.name,
                                                         origin_url=stream.pull_stream_url)
                    if __media_ret:
                        stream.forward_state = 1
                        stream.save()
                        successCount += 1
                    else:
                        errorCount += 1

            if successCount > 0:
                __ret = True
            __msg = "转发成功%d条,转发失败%d条" % (successCount, errorCount)

    except Exception as e:
        __msg = "开启转发失败：" + str(e)

    return __ret, __msg

def parse_get_params(request):
    params = {}
    for k in request.GET:
        params.__setitem__(k, request.GET.get(k))

    return params


def parse_post_params(request):
    params = {}
    for k in request.POST:
        params.__setitem__(k, request.POST.get(k))

    # 接收json方式上传的参数
    if not params:
        params = request.body.decode('utf-8')
        params = json.loads(params)

    return params


def HttpResponseJson(res):
    def json_dumps_default(obj):
        if hasattr(obj, 'isoformat'):
            return obj.isoformat()
        else:
            raise TypeError

    return HttpResponse(json.dumps(res, default=json_dumps_default), content_type="application/json")

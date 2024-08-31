from app.models import *
from app.views.ViewsBase import *
from app.utils.OSInfo import OSInfo


def public_deleteAlarm(alarm_id):
    # 删除报警视频对应的数据库数据以及文件数据
    try:
        alarm = Alarm.objects.get(id=alarm_id)
        alarm.delete()

        try:
            absolute_video_path = os.path.join(g_config.uploadDir, alarm.video_path)
            absolute_image_path = os.path.join(g_config.uploadDir, alarm.image_path)
            if os.path.exists(absolute_video_path):
                os.remove(absolute_video_path)

            if os.path.exists(absolute_image_path):
                os.remove(absolute_image_path)

        except Exception as e:
            print("public_deleteAlarm error(file): %s" % str(e))

        return True
    except Exception as e:
        print("public_deleteAlarm error: %s" % str(e))

    return False


def api_getControls(request):
    code = 0
    msg = "error"
    mediaServerState = False
    ananyServerState = False

    atDBControls = []  # 数据库中存储的布控数据

    try:
        __online_streams_dict = {}  # 在线的视频流
        __online_controls_dict = {}  # 在线的布控数据

        __streams = g_media.getMediaList()
        mediaServerState = g_media.mediaServerState
        for d in __streams:
            if d.get("is_online"):
                __online_streams_dict[d.get("code")] = d

        if mediaServerState:
            __state, __msg, __controls = g_analyzer.controls()
            ananyServerState = g_analyzer.analyzerServerState
            for d in __controls:
                __online_controls_dict[d.get("code")] = d

        sql = "select ac.*,ab.name as algorithm_name from av_control ac left join av_algorithm as ab on ac.algorithm_code=ab.code order by ac.id desc"
        atDBControls = g_djangoSql.select(sql)  # 数据库中存储的布控数据
        atDBControlCodeSet = set()  # 数据库中所有布控code的set

        for atDBControl in atDBControls:
            atDBControlCodeSet.add(atDBControl.get("code"))

            atDBControl_stream_code = "%s_%s" % (atDBControl["stream_app"], atDBControl["stream_name"])
            atDBControl["create_time"] = atDBControl["create_time"].strftime("%Y-%m-%d %H:%M")

            if __online_streams_dict.get(atDBControl_stream_code):
                atDBControl["stream_active"] = 1  # 当前视频流在线
            else:
                atDBControl["stream_active"] = 0  # 当前视频流不在线

            __online_control = __online_controls_dict.get(atDBControl["code"])
            atDBControl["checkFps"] = "0"

            if __online_control:
                atDBControl["cur_state"] = 1  # 布控中
                atDBControl["checkFps"] = "%.2f" % float(__online_control.get("checkFps"))
            else:
                if 0 == int(atDBControl.get("state")):
                    atDBControl["cur_state"] = 0  # 未布控
                else:
                    atDBControl["cur_state"] = 5  # 布控中断

            if atDBControl.get("state") != atDBControl.get("cur_state"):
                # 数据表中的布控状态和最新布控状态不一致，需要更新至最新状态
                update_state_sql = "update av_control set state=%d where id=%d " % (
                atDBControl.get("cur_state"), atDBControl.get("id"))
                g_djangoSql.execute(update_state_sql)

        for code, control in __online_controls_dict.items():
            if code not in atDBControlCodeSet:
                # 布控数据在运行中，但却不存在本地数据表中，该数据为失控数据，需要关闭其运行状态
                print("api_getControls() 当前布控数据还在运行在，但却不存在本地数据表中，已启动停止布控", code, control)
                g_analyzer.control_cancel(code=code)

        code = 1000
        msg = "success"
    except Exception as e:
        msg = str(e)

    if mediaServerState and ananyServerState:
        serverState = "<span style='color:green;font-size:14px;'>流媒体运行中，视频分析器运行中</span>"
    elif mediaServerState and not ananyServerState:
        serverState = "<span style='color:green;font-size:14px;'>流媒体运行中</span> <span style='color:red;font-size:14px;'>视频分析器未运行<span>"
    else:
        serverState = "<span style='color:red;font-size:14px;'>流媒体未运行，视频分析器未运行<span>"

    res = {
        "code": code,
        "msg": msg,
        "ananyServerState": ananyServerState,
        "mediaServerState": mediaServerState,
        "serverState": serverState,
        "data": atDBControls
    }
    return HttpResponseJson(res)


def api_getStreams(request):
    code = 0
    msg = "error"
    mediaServerState = False
    data = []

    try:
        streams = g_media.getMediaList()
        mediaServerState = g_media.mediaServerState

        streams_in_camera_dict = {}
        cameras = g_djangoSql.select("select * from av_camera")
        cameras_dict = {}
        # 摄像头按照code生成字典
        for camera in cameras:
            push_stream_app = camera.get("push_stream_app")
            push_stream_name = camera.get("push_stream_name")
            code = "%s_%s" % (push_stream_app, push_stream_name)
            cameras_dict[code] = camera

        # 将所有在线的视频流分为用户推流的和摄像头推流的
        for stream in streams:
            stream_code = stream.get("code")
            if cameras_dict.get(stream_code):
                # 摄像头推流
                streams_in_camera_dict[stream_code] = stream
            else:
                # 用户推流
                stream["ori"] = "推流"
                data.append(stream)

        # 处理所有的摄像头，如果摄像头出现在在线视频流字典中，则更新到对应视频流的状态中
        for camera in cameras:
            push_stream_app = camera.get("push_stream_app")
            push_stream_name = camera.get("push_stream_name")
            code = "%s_%s" % (push_stream_app, push_stream_name)

            camera_stream = streams_in_camera_dict.get(code, None)

            stream = {
                "active": True if camera_stream else False,
                "code": code,
                "app": push_stream_app,
                "name": push_stream_name,
                "produce_speed": camera_stream.get("produce_speed") if camera_stream else "",
                "video": camera_stream.get("video") if camera_stream else "",
                "audio": camera_stream.get("audio") if camera_stream else "",
                "originUrl": camera_stream.get("originUrl") if camera_stream else "",  # 推流地址
                "originType": camera_stream.get("originType") if camera_stream else "",  # 推流地址采用的推流协议类型
                "originTypeStr": camera_stream.get("originTypeStr") if camera_stream else "",  # 推流地址采用的推流协议类型（字符串）
                "clients": camera_stream.get("clients") if camera_stream else 0,  # 客户端总数量
                "schemas_clients": camera_stream.get("schemas_clients") if camera_stream else [],
                "flvUrl": g_media.get_flvUrl(push_stream_app, push_stream_name),
                "hlsUrl": g_media.get_hlsUrl(push_stream_app, push_stream_name),
                "ori": camera.get("name")
            }
            data.append(stream)

        code = 1000
        msg = "success"

    except Exception as e:
        log = "内部异常，请检查流媒体服务：" + str(e)
        msg = log

    if mediaServerState:
        mediaServerState_msg = "<span style='color:green;font-size:14px;'>流媒体运行中</span>"
    else:
        mediaServerState_msg = "<span style='color:red;font-size:14px;'>流媒体未运行</span>"

    res = {
        "code": code,
        "msg": msg,
        "mediaServerState": mediaServerState,
        "mediaServerState_msg": mediaServerState_msg,
        "data": data
    }
    return HttpResponseJson(res)


def api_getIndex(request):
    # highcharts 例子 https://www.highcharts.com.cn/demo/highcharts/dynamic-update
    code = 0
    msg = "error"
    os_info = {}

    try:

        osSystem = OSInfo()
        os_info = osSystem.info()
        code = 1000
        msg = "success"

    except Exception as e:
        msg = str(e)

    res = {
        "code": code,
        "msg": msg,
        "os_info": os_info
    }
    return HttpResponseJson(res)


def api_postAddControl(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)
        try:
            controlCode = params.get("controlCode")
            algorithmCode = params.get("algorithmCode")
            objectCode = params.get("objectCode")
            polygon = params.get("polygon")
            pushStream = True if '1' == params.get("pushStream") else False
            minInterval = int(params.get("minInterval"))
            classThresh = float(params.get("classThresh"))
            overlapThresh = float(params.get("overlapThresh"))
            remark = params.get("remark")

            streamApp = params.get("streamApp")
            streamName = params.get("streamName")
            streamVideo = params.get("streamVideo")
            streamAudio = params.get("streamAudio")

            if controlCode and algorithmCode and streamApp and streamName and streamVideo:

                __save_state = False
                __save_msg = "error"

                control = None
                try:
                    control = Control.objects.get(code=controlCode)
                except:
                    pass

                if control:
                    # 编辑更新
                    control.stream_app = streamApp
                    control.stream_name = streamName
                    control.stream_video = streamVideo
                    control.stream_audio = streamAudio

                    control.algorithm_code = algorithmCode
                    control.object_code = objectCode
                    control.polygon = polygon
                    control.min_interval = minInterval
                    control.class_thresh = classThresh
                    control.overlap_thresh = overlapThresh
                    control.remark = remark
                    control.push_stream = pushStream
                    control.last_update_time = datetime.now()
                    control.save()

                    if control.id:
                        __save_state = True
                        __save_msg = "更新布控数据成功(a)"
                    else:
                        __save_msg = "更新布控数据失败(a)"

                else:
                    # 新增
                    control = Control()
                    control.user_id = getUser(request).get("id")
                    control.sort = 0
                    control.code = controlCode

                    control.stream_app = streamApp
                    control.stream_name = streamName
                    control.stream_video = streamVideo
                    control.stream_audio = streamAudio

                    control.algorithm_code = algorithmCode
                    control.object_code = objectCode
                    control.polygon = polygon
                    control.min_interval = minInterval
                    control.class_thresh = classThresh
                    control.overlap_thresh = overlapThresh
                    control.remark = remark

                    control.push_stream = pushStream
                    control.push_stream_app = g_media.default_push_stream_app
                    control.push_stream_name = controlCode

                    control.create_time = datetime.now()
                    control.last_update_time = datetime.now()

                    control.save()

                    if control.id:
                        __save_state = True
                        __save_msg = "添加布控数据成功"
                    else:
                        __save_msg = "添加布控数据失败"

                if __save_state:
                    code = 1000
                msg = __save_msg
            else:
                msg = "布控请求参数不完整！"
        except Exception as e:
            msg = "布控请求参数存在错误: %s" % str(e)
    else:
        msg = "请求方法不合法！"

    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)


def api_postEditControl(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)
        try:
            controlCode = params.get("controlCode")
            algorithmCode = params.get("algorithmCode")
            objectCode = params.get("objectCode")
            polygon = params.get("polygon")
            pushStream = True if '1' == params.get("pushStream") else False
            minInterval = int(params.get("minInterval"))
            classThresh = float(params.get("classThresh"))
            overlapThresh = float(params.get("overlapThresh"))
            remark = params.get("remark")

            if controlCode and algorithmCode and objectCode:
                try:
                    control = Control.objects.get(code=controlCode)

                    control.algorithm_code = algorithmCode
                    control.object_code = objectCode
                    control.polygon = polygon
                    control.min_interval = minInterval
                    control.class_thresh = classThresh
                    control.overlap_thresh = overlapThresh
                    control.remark = remark
                    control.push_stream = pushStream

                    control.last_update_time = datetime.now()
                    control.save()

                    if control.id:
                        code = 1000
                        msg = "更新布控数据成功"
                    else:
                        msg = "更新布控数据失败"

                except Exception as e:
                    msg = "更新布控数据失败：" + str(e)
            else:
                msg = "更新布控请求参数不完整！"
        except Exception as e:
            msg = "布控请求参数存在错误: %s" % str(e)
    else:
        msg = "请求方法不合法！"

    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)


def api_postDelControl(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)
        try:
            controlCode = params.get("controlCode")

            if controlCode:
                try:
                    control = Control.objects.get(code=controlCode)
                    g_analyzer.control_cancel(code=controlCode)  # 取消布控

                    if control.delete():
                        alarm_data = g_djangoSql.select(
                            "select id from av_alarm where control_code='%s' order by id asc" % controlCode)
                        for alarm in alarm_data:
                            alarm_id = alarm["id"]
                            public_deleteAlarm(alarm_id=alarm_id)

                        code = 1000
                        msg = "删除布控数据成功"
                    else:
                        msg = "删除布控数据失败"

                except Exception as e:
                    msg = "更新布控数据失败：" + str(e)
            else:
                msg = "删除布控请求参数不完整！"
        except Exception as e:
            msg = "删除布控请求参数存在错误: %s" % str(e)
    else:
        msg = "请求方法不合法！"

    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)


def api_postAddAnalyzer(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)

        controlCode = params.get("controlCode")

        if controlCode:

            try:
                control = Control.objects.get(code=controlCode)
            except:
                control = None
            if control:
                algorithm = g_djangoSql.select("select objects from av_algorithm where code='%s'" % control.algorithm_code)
                if len(algorithm) > 0:
                    algorithm = algorithm[0]
                else:
                    algorithm = None

                if algorithm:
                    __state, __msg = g_analyzer.control_add(
                        code=controlCode,
                        algorithmCode=control.algorithm_code,
                        objects=algorithm["objects"],
                        objectCode=control.object_code,
                        recognitionRegion=control.polygon,
                        minInterval=control.min_interval,
                        classThresh=control.class_thresh,
                        overlapThresh=control.overlap_thresh,
                        streamUrl=g_media.get_rtspUrl(control.stream_app, control.stream_name),  # 拉流地址
                        pushStream=control.push_stream,
                        pushStreamUrl=g_media.get_rtspUrl(control.push_stream_app, control.push_stream_name),  # 推流地址
                    )

                    msg = __msg
                    if __state:
                        control = Control.objects.get(code=controlCode)
                        control.state = 1
                        control.save()
                        code = 1000
                else:
                    msg = "该布控算法不存在"
            else:
                msg = "该布控不存在！"

        else:
            msg = "请求参数不合法"
    else:
        msg = "请求方法不支持"
    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)


def api_postCancelAnalyzer(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)

        controlCode = params.get("controlCode")
        if controlCode:
            control = None
            try:
                control = Control.objects.get(code=controlCode)
            except:
                pass

            if control:
                __state, __msg = g_analyzer.control_cancel(
                    code=controlCode
                )

                if __state:
                    control = Control.objects.get(code=controlCode)
                    control.state = 0
                    control.save()
                    msg = "取消布控成功"
                    code = 1000
                else:
                    msg = "取消布控失败：" + str(__msg)
            else:
                msg = "布控数据不能存在！"

        else:
            msg = "请求参数不合法"
    else:
        msg = "请求方法不支持"

    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)


def api_postHandleAlarm(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)

        alarm_ids_str = params.get("alarm_ids_str")
        handle = params.get("handle")
        if "read" == handle:
            sql = "update av_alarm set state=1 where id in (%s)" % alarm_ids_str
            if g_djangoSql.execute(sql=sql):
                msg = "已读操作成功"
                code = 1000
            else:
                msg = "已读操作失败"

        elif "delete" == handle:

            alarm_ids = alarm_ids_str.split(",")
            handle_success_count = 0
            handle_error_count = 0
            for alarm_id in alarm_ids:
                if public_deleteAlarm(alarm_id):
                    handle_success_count += 1
                else:
                    handle_error_count += 1

            msg = "删除成功%d条，删除失败%d条" % (handle_success_count, handle_error_count)
            code = 1000
        else:
            msg = "不支持的处理类型"

    else:
        msg = "请求方法不支持"
    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)


def api_postAddAlarm(request):
    code = 0
    msg = "error"

    if request.method == 'POST':
        params = parse_post_params(request)

        control_code = params.get("control_code")
        desc = params.get("desc")
        video_path = params.get("video_path")
        image_path = params.get("image_path")

        if control_code and desc and video_path and image_path:
            alarm = Alarm()
            alarm.sort = 0
            alarm.control_code = control_code
            alarm.desc = desc
            alarm.video_path = video_path
            alarm.image_path = image_path
            alarm.create_time = datetime.now()
            alarm.state = 0
            alarm.save()
            msg = "上传报警视频成功"
            code = 1000
        else:
            msg = "请求参数不合法"
    else:
        msg = "请求方法不支持"
    res = {
        "code": code,
        "msg": msg
    }
    return HttpResponseJson(res)

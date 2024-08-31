import requests
import inspect


class ZLMediaKit():
    def __init__(self, config):

        self.__config = config
        self.default_stream_app = "live"
        self.default_push_stream_app = "analyzer"
        self.default_user_agent = "Admin"

        self.timeout = 15
        self.mediaServerState = False  # 流媒体服务状态

    def __byteFormat(self, bytes, suffix="bps"):

        factor = 1024
        for unit in ["", "K", "M", "G"]:
            if bytes < factor:
                return f"{bytes:.2f}{unit}{suffix}"
            bytes /= factor

    def get_hlsUrl(self, app, name):

        return "%s/%s/%s.hls.m3u8" % (self.__config.mediaHttpHost, app, name)

    def get_httpFlvUrl(self, app, name):

        return "%s/%s/%s.live.flv" % (self.__config.mediaHttpHost, app, name)

    def get_rtspUrl(self, app, name):

        return "%s/%s/%s" % (self.__config.mediaRtspHost, app, name)

    def get_wsHost(self):
        return self.__config.mediaWsHost

    def get_wsMp4Url(self, app, name):

        return "%s/%s/%s.live.mp4" % (self.__config.mediaWsHost, app, name)

    def get_wsFlvUrl(self, app, name):

        return "%s/%s/%s.live.flv" % (self.__config.mediaWsHost, app, name)

    def get_httpMp4Url(self, app, name):

        return "%s/%s/%s.live.mp4" % (self.__config.mediaHttpHost, app, name)


    def addStreamProxy(self, app, name, origin_url, vhost="__defaultVhost__"):

        key = None  # 添加成功返回的 "key" : "__defaultVhost__/proxy/0"  流的唯一标识

        try:

            url = "{host}/index/api/addStreamProxy".format(host=self.__config.mediaHttpHost)
            params = {
                "secret": self.__config.mediaSecret,
                'vhost': vhost,
                'app': app,
                'stream': name,
                'url': origin_url
            }
            params["rtp_type"] = 0  # rtsp拉流时，拉流方式，0：tcp，1：udp，2：组播
            # params["timeout_sec"] = 1; #  拉流超时时间，单位秒，float类型
            params["enable_hls"] = 0  # 是否转换成hls协议
            params["enable_mp4"] = 0  # 是否允许mp4录制
            # params["enable_rtsp"] = 1  # 是否转rtsp协议
            params["enable_rtmp"] = 0  # 是否转rtmp / flv协议
            params["enable_ts"] = 0  # 是否转http - ts / ws - ts协议
            # params["enable_fmp4"] = 0  # 是否转http - fmp4 / ws - fmp4协议
            params["enable_audio"] = 0  # 转协议时是否开启音频
            params["add_mute_audio"] = 0  # 转协议时，无音频是否添加静音aac音频
            #  params["mp4_save_path"] = "" # mp4录制文件保存根目录，置空使用默认
            #  params["mp4_max_second"] = 1 #  mp4录制切片大小，单位秒
            #  params["hls_save_path"] = "" # hls文件保存保存根目录，置空使用默认

            res = requests.post(url, headers={
                "User-Agent": self.default_user_agent
            }, json=params, timeout=self.timeout)

            if res.status_code == 200:
                res_json = res.json()
                if 0 == res_json["code"]:
                    key = res_json["data"]["key"]
            self.mediaServerState = True
        except Exception as e:
            self.mediaServerState = False
            print("addStreamProxy() error: %s" % (str(e)))
        # print("addStreamProxy:",app,name,"key:",key)

        return key

    def delStreamProxy(self, app, name, vhost="__defaultVhost__"):

        key = "{vhost}/{app}/{name}".format(vhost=vhost, app=app, name=name)

        flag = False  # "flag" : true  成功与否
        try:
            url = "{host}/index/api/delStreamProxy?secret={secret}&key={key}".format(
                host=self.__config.mediaHttpHost,
                secret=self.__config.mediaSecret,
                key=key
            )
            res = requests.get(url, headers={
                "User-Agent": self.default_user_agent
            }, timeout=self.timeout)
            if res.status_code == 200:
                res_json = res.json()
                if 0 == res_json["code"]:
                    if True == res_json["data"]["flag"]:
                        flag = True
            self.mediaServerState = True
        except Exception as e:
            self.mediaServerState = False
            print("addStreamProxy() error: %s" % (str(e)))

        return flag

    def getMediaList(self):
        __data = []
        try:
            url = "{host}/index/api/getMediaList?secret={secret}".format(
                host=self.__config.mediaHttpHost,
                secret=self.__config.mediaSecret
            )
            # print(url)

            res = requests.get(url, headers={
                "User-Agent": self.default_user_agent
            }, timeout=self.timeout)

            if 200 == res.status_code:
                res_json = res.json()
                if 0 == res_json.get("code"):
                    data = res_json.get("data")
                    if data:
                        __data_group = {}  # 视频流按照流名称进行分组
                        for d in data:
                            app = d.get("app")  # 应用名
                            name = d.get("stream")  # 流id
                            schema = d.get("schema")  # 协议
                            code = "%s_%s" % (app, name)
                            v = __data_group.get(code)
                            if not v:
                                v = {}
                            v[schema] = d
                            __data_group[code] = v

                        for code, v in __data_group.items():
                            schema_clients = []
                            index = 0
                            d = None
                            for __schema, __d in v.items():
                                schema_clients.append({
                                    "schema": __schema,
                                    "readerCount": __d.get("readerCount")
                                })
                                if 0 == index:
                                    d = __d
                                index += 1
                            if d:
                                video_str = "无"
                                video_codec_name = None
                                video_width = 0
                                video_height = 0
                                audio_str = "无"
                                tracks = d.get("tracks", None)
                                if tracks:
                                    for track in tracks:
                                        # codec_id = track.get("codec_id","")
                                        codec_id_name = track.get("codec_id_name", "").lower()
                                        codec_type = track.get("codec_type", -1)  # Video = 0, Audio = 1
                                        # ready = track.get("ready","")

                                        if 0 == codec_type:  # 视频类型
                                            fps = track.get("fps")
                                            video_height = int(track.get("height",0))
                                            video_width = int(track.get("width",0))
                                            video_codec_name = codec_id_name

                                            video_str = "%s/%d/%dx%d" % (codec_id_name, fps, video_width, video_height)

                                        elif 1 == codec_type:  # 音频类型
                                            channels = track.get("channels")

                                            sample_bit = track.get("sample_bit")
                                            sample_rate = track.get("sample_rate")

                                            audio_str = "%s/%d/%d/%d" % (
                                                codec_id_name, channels, sample_rate, sample_bit)

                                produce_speed = self.__byteFormat(d.get("bytesSpeed"))  # 数据产生速度，单位byte/s

                                app = d.get("app")  # 应用名
                                name = d.get("stream")  # 流id

                                __data.append({
                                    "is_online": 1,
                                    "code": code,
                                    "app": app,
                                    "name": name,
                                    "produce_speed": produce_speed,
                                    "video": video_str,
                                    "video_codec_name": video_codec_name,
                                    "video_width": video_width,
                                    "video_height": video_height,
                                    "audio": audio_str,
                                    "originUrl": d.get("originUrl"),  # 推流地址
                                    "originType": d.get("originType"),  # 推流地址采用的推流协议类型
                                    "originTypeStr": d.get("originTypeStr"),  # 推流地址采用的推流协议类型（字符串）
                                    "clients": d.get("totalReaderCount"),  # 客户端总数量
                                    "schema_clients": schema_clients,
                                    "videoUrl": self.get_wsMp4Url(app, name),  # 默认播放地址(ws-fmp4)
                                    "wsHost": self.get_wsHost(),
                                    "wsMp4Url": self.get_wsMp4Url(app, name)
                                })

            self.mediaServerState = True
        except Exception as e:
            self.mediaServerState = False
            print("getMediaList() error: %s" % (str(e)))

        return __data

    def getMediaInfo(self, app, name, schema="rtsp", vhost="__defaultVhost__"):

        __ret = False
        __info = {
            "ret": False
        }
        try:
            url = "{host}/index/api/getMediaInfo?secret={secret}&schema={schema}&vhost={vhost}&app={app}&stream={name}".format(
                host=self.__config.mediaHttpHost,
                secret=self.__config.mediaSecret,
                schema=schema,
                vhost=vhost,
                app=app,
                name=name

            )

            res = requests.get(url, headers={
                "User-Agent": self.default_user_agent
            }, timeout=self.timeout)


            if 200 == res.status_code:
                res_json = res.json()
                if 0 == res_json["code"]:
                    tracks = res_json.get("tracks", None)
                    if tracks:
                        if len(tracks) > 0:
                            for track in tracks:
                                codec_id_name = track.get("codec_id_name", "").lower()
                                codec_type = int(track.get("codec_type", -1))  # Video = 0, Audio = 1
                                video_width = int(track.get("width",0))
                                video_height = int(track.get("height",0))

                                if 0 == codec_type:  # 视频类型
                                    __info["video_codec_name"] = codec_id_name
                                    __info["video_width"] = video_width
                                    __info["video_height"] = video_height
                                    __info["ret"] = True

            else:
                print("getMediaInfo() error: status=%d" % res.status_code)
            self.mediaServerState = True
        except Exception as e:
            self.mediaServerState = False
            print("getMediaInfo() error: %s" % (str(e)))

        return __info

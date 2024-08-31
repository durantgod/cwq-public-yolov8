import requests
import json
import time

class Analyzer():

    def __init__(self, analyzerHost):
        self.analyzerHost = analyzerHost
        self.timeout = 10
        self.analyzerServerState = False  # 流媒体服务状态

    def controls(self):
        """
        """
        __state = False
        __msg = "error"
        __data = []

        try:
            headers = {
                    "Content-Type": "application/json;"
                }

            data = {
            }

            data_json = json.dumps(data)

            res = requests.post(url='%s/api/controls' % self.analyzerHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:

                    res_result_data = res_result.get("data")
                    if res_result_data:
                        __data = res_result_data
                    __state = True
            else:
                __msg = "status_code=%d " % (res.status_code)
            self.analyzerServerState = True
        except Exception as e:
            self.analyzerServerState = False
            __msg = str(e)
        return __state, __msg, __data

    def control(self, code):
        """
        @code   布控编号    [str]  xxxxxxxxx
        """
        __state = False
        __msg = "error"
        __control = {}
        try:
            headers = {
                "Content-Type": "application/json;"
            }
            data = {
                "code": code,
            }

            data_json = json.dumps(data)
            res = requests.post(url='%s/api/control' % self.analyzerHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:
                    __control = res_result.get("control")
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)
            self.analyzerServerState = True
        except Exception as e:
            self.analyzerServerState = False
            __msg = str(e)

        return __state, __msg, __control

    def control_add(self, code, algorithmCode, objects, objectCode, recognitionRegion, minInterval,classThresh,overlapThresh,streamUrl,pushStream,pushStreamUrl):


        __state = False
        __msg = "error"

        try:
            headers = {
                "Content-Type": "application/json;"
            }

            data = {
                "code": code,
                "algorithmCode": algorithmCode,
                "objects": objects,
                "objectCode": objectCode,
                "recognitionRegion": recognitionRegion,
                "minInterval": str(minInterval),
                "classThresh": str(classThresh),
                "overlapThresh": str(overlapThresh),
                "streamUrl": streamUrl,
                "pushStream": pushStream,
                "pushStreamUrl": pushStreamUrl
            }

            data_json = json.dumps(data)

            print("Analyzer.control_add() data_json=",data_json)

            res = requests.post(url='%s/api/control/add' % self.analyzerHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)
            self.analyzerServerState = True
        except Exception as e:
            self.analyzerServerState = False
            __msg = str(e)

        return __state, __msg

    def control_cancel(self, code):
        """
        @code   布控编号    [str]  xxxxxxxxx
        """
        __state = False
        __msg = "error"

        try:
            headers = {
                "Content-Type": "application/json;"
            }
            data = {
                "code": code,
            }

            data_json = json.dumps(data)
            res = requests.post(url='%s/api/control/cancel' % self.analyzerHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)
            self.analyzerServerState = True
        except Exception as e:
            self.analyzerServerState = False
            __msg = str(e)

        return __state, __msg
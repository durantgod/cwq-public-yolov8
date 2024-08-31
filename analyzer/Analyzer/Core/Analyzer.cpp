#include "Analyzer.h"
#include "Algorithm.h"
#include <json/json.h>
#include "Scheduler.h"
#include "Config.h"
#include "Control.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Utils/Request.h"
#include "Utils/Base64.h"
#include "Utils/CalcuIOU.h"

namespace AVSAnalyzer {

    Analyzer::Analyzer(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(control)
    {
  
    }

    Analyzer::~Analyzer()
    {


    }

    bool Analyzer::handleVideoFrame(int64_t frameCount, cv::Mat& image, std::vector<DetectObject>& happenDetects, bool& happen, float& happenScore) {
        if (mControl->algorithmCode == "onnxruntime_yolo8") {
            //v3.41新增onnxruntime推理yolo8
            Algorithm* algorithm = mScheduler->gainAlgorithm();
            if (algorithm) {
                //推理onnxruntime_yolo8
                happenDetects.clear();
                happen = false;
                happenScore = 0;
                algorithm->objectDetect(image, happenDetects);

                mScheduler->giveBackAlgorithm(algorithm);

            }

            if(!happenDetects.empty()){

                cv::polylines(image, mControl->recognitionRegion_points, mControl->recognitionRegion_points.size(), cv::Scalar(0, 0, 255), 2, 8);//绘制多边形
                int x1, y1, x2, y2;
                int matchCount = 0;
                for (int i = 0; i < happenDetects.size(); i++)
                {
                    x1 = happenDetects[i].x1;
                    y1 = happenDetects[i].y1;
                    x2 = happenDetects[i].x2;
                    y2 = happenDetects[i].y2;

                    std::vector<double> object_d;
                    object_d.push_back(x1);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y2);

                    object_d.push_back(x1);
                    object_d.push_back(y2);


                    double iou = CalcuPolygonIOU(mControl->recognitionRegion_d, object_d);

                    if (iou >= 0.5) {
                        int class_id = happenDetects[i].class_id;
                        std::string class_name = "e";
                        if (class_id < mControl->objects_v1_len) {
                            class_name = mControl->objects_v1[class_id];
                        }
                        else {
                            LOGE("算法分类数量不正确");
                        }

                        happenDetects[i].class_name = class_name;
                        float class_score = happenDetects[i].score;
                        if (class_name == mControl->objectCode) {
                            ++matchCount;
                        }
                        
                    }
                }
                mControl->count = matchCount;//写入目标个数
                if (matchCount > 0) {//匹配数据大于0，则认为发生了报警事件
                    happen = true;
                    happenScore = 1.0;
                }

            }

            return true;
        }
        else if (mControl->algorithmCode == "API") {
            //v3.40新增，调用API类型的算法服务
            return this->postImage2Server(frameCount, image,happenDetects,happen,happenScore);

        }
        else if (mControl->algorithmCode == "DLIB_FACE") {
            //v3.40新增，基于dlib的人脸检测
            //v4.41移除，因为v3.41需要在rk3588等硬件编译，如果不去除还需要编译dlib，比较麻烦
            LOGE("该算法仅在v3.40版本支持：%s", mControl->algorithmCode.data());
            return false;
        }
        //新增一个其他检测的算法
        else if (mControl->algorithmCode == "orth_yolov8") {
            Algorithm* algorithm = mScheduler->gainAlgorithm();
            if (algorithm) {
                happenDetects.clear();
                happen = false;
                happenScore = 0;
                algorithm->orthDetect(image, happenDetects);
                mScheduler->giveBackAlgorithm(algorithm);
            }
            if (!happenDetects.empty()) {
                cv::polylines(image, mControl->recognitionRegion_points, mControl->recognitionRegion_points.size(), cv::Scalar(0, 255, 0), 2, 8);
                int x1, x2, y1, y2;
                int mathCount = 0;
                for (int i = 0; i < happenDetects.size(); i++) {
                    x1 = happenDetects[i].x1;
                    x2 = happenDetects[i].x2;
                    y1 = happenDetects[i].y1;
                    y2 = happenDetects[i].y2;

                    std::vector<double> object_d;
                    object_d.push_back(x1);
                    object_d.push_back(y1);

                    object_d.push_back(x2);
                    object_d.push_back(y1);

                    object_d.push_back(x1);
                    object_d.push_back(y2);


                    object_d.push_back(x2);
                    object_d.push_back(y2);

                    double iou = CalcuPolygonIOU(mControl->recognitionRegion_d, object_d);
                    if (iou >= 0.6) {                                                     //目标框与检测区域重合比例
                        int class_id = happenDetects[i].class_id;
                        std::string class_name = "e";
                        if (class_id < mControl->objects_v1_len) {
                            class_name = mControl->objects_v1[class_id];
                        }
                        else {
                            LOGE("算法分类数量不正确");
                        }
                        happenDetects[i].class_name = class_name;
                        float class_score = happenDetects[i].score;
                        if (class_name == mControl->objectCode) {
                            ++mathCount;
                        }
                    }
                }
                mControl->count = mathCount;
                if (mathCount > 0) {
                    happen = true;
                    happenScore = 1.0;
                }
            }
            return true;
        }
        else {
            LOGE("不支持的算法：%s",mControl->algorithmCode.data());
            return false;
        }

        return false;

    }
    bool Analyzer::postImage2Server(int64_t frameCount, cv::Mat& image, std::vector<DetectObject>& happenDetects, bool& happen, float& happenScore) {

        Config* config = mScheduler->getConfig();
        int height = mControl->videoHeight;
        int width = mControl->videoWidth;

        std::vector<int> JPEG_QUALITY = { 75 };
        std::vector<uchar> jpg;
        cv::imencode(".jpg", image, jpg, JPEG_QUALITY);
        int JPGBufSize = jpg.size();


        if (JPGBufSize > 0) {
            Base64 base64;
            std::string imageBase64;
            base64.encode(jpg.data(), JPGBufSize, imageBase64);

            std::string response;
            Json::Value param;
            param["image_base64"] = imageBase64;

            param["code"] = mControl->code;//布控编号
            param["objectCode"] = mControl->objectCode;
            param["objects"] = mControl->objects;
            param["recognitionRegion"] = mControl->recognitionRegion;
            param["min_interval"] = mControl->minInterval;

            std::string data = param.toStyledString();
            //int64_t t1 = Common::getCurTimestamp();
            Request request;
            if (request.post(config->algorithmApiUrl.data(), data.data(), response)) {
                Json::CharReaderBuilder builder;
                const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

                Json::Value root;
                JSONCPP_STRING errs;

                if (reader->parse(response.data(), response.data() + std::strlen(response.data()),
                    &root, &errs) && errs.empty()) {

                    if (root["code"].isInt() && root["msg"].isString()) {
                        int code = root["code"].asInt();
                        std::string msg = root["msg"].asCString();

                        if (1000 == code) {
                            happenDetects.clear();
                            happen = false;
                            happenScore = 0.0;

                            Json::Value result = root["result"];
                            if (result["happen"].isBool() && result["happenScore"].isDouble()) {
                                happen = result["happen"].asBool();
                                happenScore = result["happenScore"].asFloat();

                                int mathCount = 0;

                                Json::Value result_detects = result["detects"];
                                for (auto i : result_detects) {

                                    int x1 = i["x1"].asInt();
                                    int y1 = i["y1"].asInt();
                                    int x2 = i["x2"].asInt();
                                    int y2 = i["y2"].asInt();
                                    float class_score = i["class_score"].asFloat();
                                    std::string class_name = i["class_name"].asString();

                                    DetectObject detect;
                                    detect.x1 = x1;
                                    detect.y1 = y1;
                                    detect.x2 = x2;
                                    detect.y2 = y2;
                                    detect.class_name = class_name;
                                    detect.score = class_score;

                                    ++mathCount;

                                    happenDetects.push_back(detect);
                                }
                                mControl->count = mathCount;
                            }
                        }
                        else {
                            LOGE("code=%d,msg=%s", code, msg.data());
                        }

                    }
                    else {
                        LOGE("incorrect return parameter format");
                    }
                }
            }
            else {
                happenDetects.clear();
                happen = false;
                happenScore = 0.0;
            }
            //int64_t t2 = Common::getCurTimestamp();
        
        }

        return true;
    }

}
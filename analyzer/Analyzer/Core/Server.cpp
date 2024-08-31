#include "Server.h"


#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>
#include <json/json.h>
#include <json/value.h>
#include <thread>
#include "Control.h"
#include "Config.h"
#include "Scheduler.h"
#include "Utils/Log.h"
#include "Utils/Common.h"

using namespace AVSAnalyzer;

#define RECV_BUF_MAX_SIZE 1024*8


Server::Server() {
#ifdef WIN32
    WSADATA wdSockMsg;
    int s = WSAStartup(MAKEWORD(2, 2), &wdSockMsg);

    if (0 != s)
    {
        switch (s)
        {
        case WSASYSNOTREADY: printf("重启电脑，或者检查网络库");   break;
        case WSAVERNOTSUPPORTED: printf("请更新网络库");  break;
        case WSAEINPROGRESS: printf("请重新启动");  break;
        case WSAEPROCLIM:  printf("请关闭不必要的软件，以确保有足够的网络资源"); break;
        }
    }

    if (2 != HIBYTE(wdSockMsg.wVersion) || 2 != LOBYTE(wdSockMsg.wVersion))
    {
        LOGE("网络库版本错误");
        return;
    }
#endif

}
Server::~Server() {
    LOGE("");
#ifdef WIN32
    WSACleanup();
#endif

}

void Server::start(void* arg) {
    Scheduler* scheduler = (Scheduler*)arg;
    scheduler->setState(true);

    std::thread([](Scheduler *scheduler) {

        LOGI("启动分析器服务：http://0.0.0.0:%d", scheduler->getConfig()->analyzerPort);
        
        event_config* evt_config = event_config_new();
        struct event_base* base = event_base_new_with_config(evt_config);
        struct evhttp* http = evhttp_new(base);
        evhttp_set_default_content_type(http, "text/html; charset=utf-8");

        evhttp_set_timeout(http, 30);
        // 设置路由
        evhttp_set_cb(http, "/", api_index, nullptr);
        evhttp_set_cb(http, "/api/health", api_health, scheduler);
        evhttp_set_cb(http, "/api/controls", api_controls, scheduler);
        evhttp_set_cb(http, "/api/control", api_control, scheduler);
        evhttp_set_cb(http, "/api/control/add", api_control_add, scheduler);
        evhttp_set_cb(http, "/api/control/cancel", api_control_cancel, scheduler);

        evhttp_bind_socket(http, "0.0.0.0",
            scheduler->getConfig()->analyzerPort);
        event_base_dispatch(base);

        event_base_free(base);
        evhttp_free(http);
        event_config_free(evt_config);

        scheduler->setState(false);

    }, scheduler).detach();

}

void api_index(struct evhttp_request* req, void* arg) {
   
    Json::Value result_urls;
    result_urls["/api"] = "this api version 1.0";
    result_urls["/api/health"] = "check health";
    result_urls["/api/controls"] = "get all control being analyzed";
    result_urls["/api/control"] = "get control being analyzed";
    result_urls["/api/control/add"] = "add control";
    result_urls["/api/control/cancel"] = "cancel control";
    
    
    Json::Value result;
    result["urls"] = result_urls;

    struct evbuffer* buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", result.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);

}
void api_health(struct evhttp_request* req, void* arg) {
    int result_code = 0;
    std::string result_msg = "error";

    // 健康检测
    result_code = 1000;
    result_msg = "current service health";


    Json::Value result;
    result["msg"] = result_msg;
    result["code"] = result_code;

    struct evbuffer* buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", result.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);

}
void api_controls(struct evhttp_request* req, void* arg) {
    

    Scheduler* scheduler = (Scheduler*)arg;
    char buf[RECV_BUF_MAX_SIZE];
    parse_post(req, buf);

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errs;


    Json::Value result_data;
    Json::Value result_data_item;
    int result_code = 0;
    std::string result_msg = "error";
    Json::Value result;

    if (reader->parse(buf, buf + std::strlen(buf), &root, &errs) && errs.empty()) {

        std::vector<Control*> controls;
        int len = scheduler->apiControls(controls);

        if (len > 0) {
            int64_t curTimestamp = getCurTimestamp();
            int64_t startTimestamp = 0;
            for (int i = 0; i < controls.size(); i++)
            {
                startTimestamp = controls[i]->startTimestamp;

                result_data_item["code"] = controls[i]->code.data();
                result_data_item["streamUrl"] = controls[i]->streamUrl.data();
                result_data_item["pushStream"] = controls[i]->pushStream;
                result_data_item["pushStreamUrl"] = controls[i]->pushStreamUrl.data();
                result_data_item["algorithmCode"] = controls[i]->algorithmCode.data();
                result_data_item["objectCode"] = controls[i]->objectCode.data();
                result_data_item["recognitionRegion"] = controls[i]->recognitionRegion.data();

                result_data_item["mathcount"] = controls[i]->count;
                result_data_item["checkFps"] = controls[i]->checkFps;
                result_data_item["startTimestamp"] = startTimestamp;
                result_data_item["liveMilliseconds"] = curTimestamp - startTimestamp;


                result_data.append(result_data_item);
            }
            result["data"] = result_data;
            result_code = 1000;
            result_msg = "success";
        }
        else {
            result_msg = "the number of control exector is empty";
        }


    }
    else {
        result_msg = "invalid request parameter";
    }
    result["msg"] = result_msg;
    result["code"] = result_code;

    //LOGI("\n \t request:%s \n \t response:%s", root.toStyledString().data(), result.toStyledString().data());


    struct evbuffer* buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", result.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);

}
void api_control(struct evhttp_request* req, void* arg) {

    Scheduler* scheduler = (Scheduler*)arg;
    char buf[RECV_BUF_MAX_SIZE];
    parse_post(req, buf);

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errs;

    Json::Value result_control;
    int result_code = 0;
    std::string result_msg = "error";
    

    if (reader->parse(buf, buf + std::strlen(buf), &root, &errs) && errs.empty()) {

        Control* control = NULL;
        if (root["code"].isString()) {
            std::string code = root["code"].asCString();
            control = scheduler->apiControl(code);
        }

        if (control) {
            result_control["code"] = control->code;
            result_control["checkFps"] = control->checkFps;
            result_control["mathcount"] = control->count;

            result_code = 1000;
            result_msg = "success";

        }
        else {
            result_msg = "the control does not exist";
        }
    }
    else {
        result_msg = "invalid request parameter";
    }

    Json::Value result;
    result["control"] = result_control;
    result["msg"] = result_msg;
    result["code"] = result_code;

    LOGI("\n \t request:%s \n \t response:%s", root.toStyledString().data(), result.toStyledString().data());


    struct evbuffer* buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", result.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);

}
void api_control_add(struct evhttp_request* req, void* arg) {

    Scheduler* scheduler = (Scheduler*)arg;
    char buf[RECV_BUF_MAX_SIZE];
    parse_post(req, buf);

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errs;

    int result_code = 0;
    std::string result_msg = "error";


    if (reader->parse(buf, buf + std::strlen(buf), &root, &errs) && errs.empty()) {

        Control control;

        control.code = root["code"].asCString();
        control.streamUrl = root["streamUrl"].asString();
        control.pushStream = root["pushStream"].asBool();
        control.pushStreamUrl = root["pushStreamUrl"].asString();
        control.algorithmCode = root["algorithmCode"].asString();
        control.objects = root["objects"].asString();
        control.objects_v1 = split(control.objects, ",");
        control.objects_v1_len = control.objects_v1.size();
        control.objectCode = root["objectCode"].asString();
        control.recognitionRegion = root["recognitionRegion"].asString();

        if (root["minInterval"].isString()) {
            control.minInterval = stoi(root["minInterval"].asString()) * 1000;//客户端传递过的该参数单位是秒
        }
        //强制设置报警时间最大不能超过3分钟=180000毫秒
        if (control.minInterval < 180000) {
            control.minInterval = 180000;
        }
        if (control.validateAdd(result_msg)) {
            scheduler->apiControlAdd(&control, result_code, result_msg);
        }
    }
    else {
        result_msg = "invalid request parameter";
    }

    Json::Value result;
    result["msg"] = result_msg;
    result["code"] = result_code;

    LOGI("\n \t request:%s \n \t response:%s", root.toStyledString().data(), result.toStyledString().data());

    struct evbuffer* buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", result.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);


}
void api_control_cancel(struct evhttp_request* req, void* arg) {


    Scheduler* scheduler = (Scheduler*)arg;
    char buf[RECV_BUF_MAX_SIZE];
    parse_post(req, buf);

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    Json::Value root;
    JSONCPP_STRING errs;

    int result_code = 0;
    std::string result_msg = "error";

    if (reader->parse(buf, buf + std::strlen(buf), &root, &errs) && errs.empty()) {

        Control control;

        if (root["code"].isString()) {
            control.code = root["code"].asCString();
        }
        if (control.validateCancel(result_msg)) {
            scheduler->apiControlCancel(&control, result_code, result_msg);
        }

    }
    else {
        result_msg = "invalid request parameter";
    }

    Json::Value result;
    result["msg"] = result_msg;
    result["code"] = result_code;

    LOGI("\n \t request:%s \n \t response:%s", root.toStyledString().data(), result.toStyledString().data());

    struct evbuffer* buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", result.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);


}
void parse_get(struct evhttp_request* req, struct evkeyvalq* params) {
    if (req == nullptr) {
        return;
    }
    const char* url = evhttp_request_get_uri(req);
    if (url == nullptr) {
        return;
    }
    struct evhttp_uri* decoded = evhttp_uri_parse(url);
    if (!decoded) {
        return;
    }
    const char* path = evhttp_uri_get_path(decoded);
    if (path == nullptr) {
        path = "/";
    }
    char* query = (char*)evhttp_uri_get_query(decoded);
    if (query == nullptr) {
        return;
    }
    evhttp_parse_query_str(query, params);
}
void parse_post(struct evhttp_request* req, char* buf) {
    size_t post_size = 0;

    post_size = evbuffer_get_length(req->input_buffer);
    if (post_size <= 0) {
        //        printf("====line:%d,post msg is empty!\n",__LINE__);
        return;

    }
    else {
        size_t copy_len = post_size > RECV_BUF_MAX_SIZE ? RECV_BUF_MAX_SIZE : post_size;
        //        printf("====line:%d,post len:%d, copy_len:%d\n",__LINE__,post_size,copy_len);
        memcpy(buf, evbuffer_pullup(req->input_buffer, -1), copy_len);
        buf[post_size] = '\0';
        //        printf("====line:%d,post msg:%s\n",__LINE__,buf);
    }

}
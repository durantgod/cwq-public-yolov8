#include "Request.h"
#include <curl/curl.h>
#include "Log.h"

namespace AVSAnalyzer {
    inline size_t onWrite(void* buffer, size_t size, size_t nmemb, void* stream) {

        std::string* str = dynamic_cast<std::string*>((std::string*)stream);
        if (NULL == str || NULL == buffer)
        {
            return -1;
        }

        char* pData = (char*)buffer;
        str->append(pData, size * nmemb);
        return nmemb;
    }
    /*
    inline size_t onWrite(void* ptr, size_t size, size_t nmEmb, void* stream) {
        //    std::cout << "----->reply" << std::endl;
        std::string* str = (std::string*)stream;
        //    std::cout << *str << std::endl;
        (*str).append((char*)ptr, size * nmEmb);

        return size * nmEmb;
    }
    */
    Request::Request()
    {


    }

    Request::~Request()
    {
    }

    bool Request::get(const char* url, std::string& response) {

        CURL* curl = curl_easy_init();
        bool result;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);//0 or 1 当等于1时，会显示详细的调试信息,
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

            CURLcode code = curl_easy_perform(curl);


            if (code != CURLE_OK) {
                LOGE("curl_easy_strerror: %s", curl_easy_strerror(code));
                result = false;
            }
            else {
                result = true;
            }


        }
        else {
            LOGE("curl_easy_init error");
            result = false;
        }
        curl_easy_cleanup(curl);

        return result;
    }
    bool Request::post(const char* url, const char* data, std::string& response) {
        curl_global_init(CURL_GLOBAL_WIN32);

        CURL* curl = curl_easy_init();
        bool result;

        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "User-Agent: Analyzer;");
            headers = curl_slist_append(headers, "Content-Type:application/json;");
            headers = curl_slist_append(headers,
                "expect: ;");// libcurl请求慢解决方法 https://blog.csdn.net/feng964497595/article/details/86316861
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            //不接收响应头数据0代表不接收 1代表接收
            curl_easy_setopt(curl, CURLOPT_HEADER, 0);

            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POST, 1); // post type
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data); // post params

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false

            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);// 值为1时，会显示详细的调试信息
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            // curl_easy_setopt(curl, CURLOPT_HEADER, false);// 是否显示响应头信息
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

            CURLcode code = curl_easy_perform(curl);


            if (code != CURLE_OK) {
                LOGE("curl_easy_strerror: url=%s, %s", url, curl_easy_strerror(code));
                result = false;
            }
            else {
                result = true;
            }
            curl_slist_free_all(headers);//清理headers,防止内存泄漏


        }
        else {
            LOGE("curl_easy_init error: url=%s", url);
            result = false;
        }
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return result;

    }
}
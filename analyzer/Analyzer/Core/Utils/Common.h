#ifndef ANALYZER_COMMON_H
#define ANALYZER_COMMON_H

#include <string>
#include <vector>
#include <chrono>


namespace AVSAnalyzer {
    static int64_t getCurTime()// 获取当前系统启动以来的毫秒数
    {
#ifndef WIN32
        // Linux系统
        struct timespec now;// tv_sec (s) tv_nsec (ns-纳秒)
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
#else
        long long now = std::chrono::steady_clock::now().time_since_epoch().count();
        return now / 1000000;
#endif // !WIN32

    }
    static int64_t getCurTimestamp()// 获取毫秒级时间戳（13位）
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).
            count();

    }
    static std::string getCurFormatTimeStr(const char* format = "%Y-%m-%d %H:%M:%S") {

        time_t t = time(nullptr);

        char tc[64];
        strftime(tc, sizeof(tc), format, localtime(&t));

        std::string timeStr = tc;
        return timeStr;
    }
    static std::vector<std::string> split(const std::string& str, const std::string& sep) {

        std::vector<std::string> arr;
        int sepSize = sep.size();

        int lastPosition = 0, index = -1;
        while (-1 != (index = str.find(sep, lastPosition)))
        {
            arr.push_back(str.substr(lastPosition, index - lastPosition));
            lastPosition = index + sepSize;
        }
        std::string lastStr = str.substr(lastPosition);//截取最后一个分隔符后的内容

        if (!lastStr.empty()) {
            arr.push_back(lastStr);//如果最后一个分隔符后还有内容就入队
        }

        return arr;
    }

    static bool removeFile(const std::string& filename) {

        if (remove(filename.data()) == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    static int getRandomInt() {
        std::string numStr;
        numStr.append(std::to_string(std::rand() % 9 + 1));
        numStr.append(std::to_string(std::rand() % 10));
        numStr.append(std::to_string(std::rand() % 10));
        numStr.append(std::to_string(std::rand() % 10));
        numStr.append(std::to_string(std::rand() % 10));
        numStr.append(std::to_string(std::rand() % 10));
        numStr.append(std::to_string(std::rand() % 10));
        numStr.append(std::to_string(std::rand() % 10));
        int num = stoi(numStr);

        return num;
    }


};

#endif //ANALYZER_COMMON_H
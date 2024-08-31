#ifndef ANALYZER_CONTROL_H
#define ANALYZER_CONTROL_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Utils/Common.h"

namespace AVSAnalyzer {

	struct Control
	{
		// 布控请求必需参数
	public:
		std::string code;// 布控编号
		std::string streamUrl;//拉流地址
		bool        pushStream = false;//是否推流
		std::string pushStreamUrl;//推流地址
		std::string algorithmCode;//算法编号
		std::string objects;//算法支持所有分类
		std::vector<std::string> objects_v1;
		int objects_v1_len;

		std::string objectCode;//目标监测分类编号

		std::string		       recognitionRegion;   //算法识别区域坐标点 x1, y1, x2, y2, x3, y3, x4, y4
		std::vector<double>    recognitionRegion_d; //算法识别区域坐标点 x1, y1, x2, y2, x3, y3, x4, y4
		std::vector<cv::Point> recognitionRegion_points;//算法识别区域

		int64_t minInterval = 180000;// 布控最小的报警间隔时间（单位毫秒），3分钟=3*60*1000=180000毫秒

	public:
		// 通过计算获得的参数

		int64_t startTimestamp = 0;// 执行器启动时毫秒级时间戳（13位）
		float   checkFps = 0;// 算法检测的帧率（每秒检测的次数）
		int     videoWidth = 0;  // 布控视频流的像素宽
		int     videoHeight = 0; // 布控视频流的像素高
		int     videoChannel = 0;
		int     videoIndex = -1;
		int     videoFps = 0;
		int     count = 0;

	public:
		bool parseRecognitionRegion() {
			bool res = false;
			if (recognitionRegion_points.empty()) {
				if (videoWidth > 0 && videoHeight > 0) {

					std::vector<std::string> xys = split(recognitionRegion, ",");
					int lineCount = xys.size() / 2; //多边形边数
					if (lineCount > 3) {
						int x_index;
						int y_index;
						int x;
						int y;

						for (int i = 0; i < lineCount; i++)
						{
							x_index = i * 2;
							y_index = x_index + 1;
							x = stof(xys[x_index]) * videoWidth;
							y = stof(xys[y_index]) * videoHeight;

							recognitionRegion_d.push_back(x);
							recognitionRegion_d.push_back(y);

							cv::Point p(x, y);
							recognitionRegion_points.push_back(p);
						}
						res = true;
					}

				}

			}
			else {
				res = true;
			}
			return res;
		}
		bool validateAdd(std::string& result_msg) {
			if (code.empty() || streamUrl.empty() || algorithmCode.empty()|| objectCode.empty() || recognitionRegion.empty()) {
				result_msg = "validate parameter error";
				return false;
			}
			if (pushStream) {
				if (pushStreamUrl.empty()) {
					result_msg = "validate parameter pushStreamUrl is error: " + pushStreamUrl;
					return false;
				}

			}
			result_msg = "validate success";
			return true;
		}
		bool validateCancel(std::string& result_msg) {

			if (code.empty()) {
				result_msg = "validate parameter error";
				return false;
			}
			result_msg = "validate success";
			return true;
		}


	};
}
#endif //ANALYZER_CONTROL_H

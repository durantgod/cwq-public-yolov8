#ifndef ANALYZER_ALGORITHM_H
#define ANALYZER_ALGORITHM_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>    //opencv header file

namespace AVSAnalyzer {
    class Config;

    static std::vector<cv::Scalar> colors = {
        cv::Scalar(0, 0, 255) ,
        cv::Scalar(0, 255, 0) ,
        cv::Scalar(255, 0, 0) ,
        cv::Scalar(255, 100, 50) ,
        cv::Scalar(50, 100, 255) ,
        cv::Scalar(255, 50, 100)
     };

    cv::Mat static letterbox(const cv::Mat& source)
    {
        int col = source.cols;
        int row = source.rows;
        int _max = MAX(col, row);
        cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
        source.copyTo(result(cv::Rect(0, 0, col, row)));
        return result;
    };

	struct DetectObject
	{
		int x1;
		int y1;
		int x2;
		int y2;
		float score;
        int class_id;
		std::string class_name;
	};

    class Algorithm
    {
    public:
        Algorithm() = delete;
        Algorithm(Config* config);
        virtual ~Algorithm();
    public:
        //yolov8目标检测
        virtual bool objectDetect(cv::Mat &image, std::vector<DetectObject>& detects) = 0;
        //其他算法
        virtual bool orthDetect(cv::Mat& image, std::vector<DetectObject>& detects) = 0;
    protected:
        Config* mConfig;

    };

}
#endif //ANALYZER_ALGORITHM_H


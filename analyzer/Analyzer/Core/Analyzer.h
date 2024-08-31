#ifndef ANALYZER_ANALYZER_H
#define ANALYZER_ANALYZER_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

namespace AVSAnalyzer {
	struct Control;
	class Config;
	class Scheduler;
	class Algorithm;
	struct DetectObject;

	class Analyzer
	{
	public:
		explicit Analyzer(Scheduler* scheduler, Control* control);
		~Analyzer();
	public:
		bool handleVideoFrame(int64_t frameCount, cv::Mat &image, std::vector<DetectObject>& happenDetects, bool& happen, float& happenScore);
	private:
		bool postImage2Server(int64_t frameCount, cv::Mat& image, std::vector<DetectObject>& happenDetects, bool& happen, float& happenScore);

	private:
		Scheduler* mScheduler;
		Control*   mControl;
	};
}
#endif //ANALYZER_ANALYZER_H


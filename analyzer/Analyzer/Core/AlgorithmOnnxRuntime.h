#ifndef ANALYZER_ALGORITHMOPENVINOYOLO8_H
#define ANALYZER_ALGORITHMOPENVINOYOLO8_H

#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include "Algorithm.h"
#include <onnxruntime_cxx_api.h>

namespace AVSAnalyzer {
	class Config;
	class OnnxRuntimeEngine
	{
	public:
		explicit OnnxRuntimeEngine(std::string modelPath);
		~OnnxRuntimeEngine();
	public:
		bool runInference(cv::Mat& image, std::vector<DetectObject>& detects);

	private:
		std::string mModelPath;
		Ort::Env mEnv{ nullptr };
		Ort::SessionOptions mSessionOptions{ nullptr };
		Ort::Session mSession{ nullptr };

	};
	class AlgorithmOnnxRuntime : public Algorithm
	{
	public:
		AlgorithmOnnxRuntime(Config* config);
		virtual ~AlgorithmOnnxRuntime();
	public:
		//目标检测算法
		virtual bool objectDetect(cv::Mat& image, std::vector<DetectObject>& detects);
		//其他算法
		virtual bool orthDetect(cv::Mat& image, std::vector<DetectObject>& detects);
	private:
		OnnxRuntimeEngine* mEngine;
		OnnxRuntimeEngine* oEngine;//其他
	};
}
#endif //ANALYZER_ALGORITHMOPENVINOYOLO8_H


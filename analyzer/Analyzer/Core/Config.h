#ifndef ANALYZER_CONFIG_H
#define ANALYZER_CONFIG_H

#include <string>
#include <vector>

namespace AVSAnalyzer {
	class Config
	{
	public:
		Config(const char* file);
		~Config();
	public:

		bool mState = false;
		void show();
	public:
		const char* file = NULL;

		std::string host{};//主机IP地址 192.168.1.4
		std::string adminHost{};//后台管理服务地址 http://192.168.1.4:9001
		int adminPort;// 后台管理服务端口 9001
		int analyzerPort;// 分析服务端口 9002
		int mediaHttpPort;// 80
		int mediaRtspPort;// 554

		std::string uploadDir{};
		std::string videoFileNameFormat{};

		std::string algorithmApiUrl{};//算法api服务器url
		std::string onnxModelPath{};// onnx模型路径

		std::string orthModelPath{};//其他模型的路径

	};
}
#endif //ANALYZER_CONFIG_H
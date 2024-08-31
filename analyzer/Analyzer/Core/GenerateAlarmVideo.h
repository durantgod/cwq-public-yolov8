#ifndef ANALYZER_GENERATEALARMVIDEO_H
#define ANALYZER_GENERATEALARMVIDEO_H
#include <vector>
#include <queue>
#include <mutex>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

namespace AVSAnalyzer {

	class Config;
	struct Frame;

	struct Alarm
	{
	public:
		Alarm() = delete;
		Alarm(int height, int width, int fps, int64_t happenTimestamp,int happenImageIndex, const char* controlCode);
		~Alarm();
	public:
		int width = 0;
		int height = 0;
		int fps = 0;
		int64_t happenTimestamp = 0; //发生事件的时间戳（毫秒级）
		int		happenImageIndex = 0;//封面图index
		std::string controlCode;// 布控编号
		std::vector<Frame*> frames;//组成报警视频的图片帧
	};


	class GenerateAlarmVideo
	{
	public:
		GenerateAlarmVideo() = delete;
		GenerateAlarmVideo(Config* config, Alarm* alarm);
		~GenerateAlarmVideo();

	public:
		bool genAlarmVideo();
	private:
		Config* mConfig;
		Alarm* mAlarm;
		bool initCodecCtx(const char* url);
		void destoryCodecCtx();

		AVFormatContext* mFmtCtx = nullptr;
		//视频帧
		AVCodecContext* mVideoCodecCtx = nullptr;
		AVStream* mVideoStream = nullptr;
		int mVideoIndex = -1;
	};

}

#endif //ANALYZER_GENERATEALARMVIDEO_H
#ifndef ANALYZER_AVPUSHSTREAM_H
#define ANALYZER_AVPUSHSTREAM_H
#include <queue>
#include <mutex>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
namespace AVSAnalyzer {
	class Worker;
	struct Frame;

	class AvPushStream
	{
	public:
		AvPushStream(Worker* worker);
		~AvPushStream();
	public:
		bool connect();     // 连接流媒体服务
		bool reConnect();   // 重连流媒体服务
		void closeConnect();// 关闭流媒体服务的连接
		int mConnectCount = 0;

		AVFormatContext* mFmtCtx = nullptr;

		//视频帧
		AVCodecContext* mVideoCodecCtx = NULL;
		AVStream* mVideoStream = NULL;
		int mVideoIndex = -1;
		void addVideoFrame(Frame* frame);
		int getVideoFrameQSize();


	public:
		static void encodeVideoThread(void* arg); // 编码视频帧并推流
		void handleEncodeVideo();

	private:
		Worker* mWorker;

		//视频帧
		std::queue <Frame*> mVideoFrameQ;
		std::mutex          mVideoFrameQ_mtx;
		bool getVideoFrame(Frame*& frame);
		void clearVideoFrameQueue();

		// bgr24转yuv420p
		unsigned char clipValue(unsigned char x, unsigned char min_val, unsigned char  max_val);
		bool bgr24ToYuv420p(unsigned char* bgrBuf, int w, int h, unsigned char* yuvBuf);
	};

}
#endif //ANALYZER_AVPUSHSTREAM_H

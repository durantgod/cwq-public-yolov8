#ifndef ANALYZER_AVPULLSTREAM_H
#define ANALYZER_AVPULLSTREAM_H
#include <queue>
#include <mutex>
#include <condition_variable>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
namespace AVSAnalyzer {
	class Worker;

	class AvPullStream
	{
	public:
		AvPullStream(Worker* worker);
		~AvPullStream();

	public:
		bool connect();     // 连接流媒体服务
		bool reConnect();   // 重连流媒体服务
		void closeConnect();// 关闭流媒体服务的连接

		int mConnectCount = 0;

		AVFormatContext* mFmtCtx = NULL;

		// 视频帧
		AVCodecContext* mVideoCodecCtx = NULL;
		AVStream* mVideoStream = NULL;
		bool getVideoPkt(AVPacket& pkt, int& pktQSize);// 从队列获取的pkt，一定要主动释放!!!


	public:
		static void readThread(void* arg); // 拉流媒体流
		void handleRead();
	private:
		Worker* mWorker;

		bool pushVideoPkt(const AVPacket& pkt);
		void clearVideoPktQueue();
		std::queue <AVPacket>   mVideoPktQ;
		std::mutex              mVideoPktQ_mtx;

	};


}
#endif //ANALYZER_AVPULLSTREAM_H
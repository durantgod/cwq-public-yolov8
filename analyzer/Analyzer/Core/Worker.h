#ifndef ANALYZER_WORKER_H
#define ANALYZER_WORKER_H
#include <thread>
#include <queue>
#include <mutex>

namespace AVSAnalyzer {
	class Scheduler;
	class AvPullStream;
	class AvPushStream;
	class Analyzer;
	struct Control;
	struct Frame;
	class FramePool;

	class Worker
	{
	public:
		explicit Worker(Scheduler* scheduler, Control* control);
		~Worker();
	public:
		static void decodeVideoThread(void* arg);// （线程）解码视频帧和实时分析视频帧
		void handleDecodeVideo();
		static void generateAlarmThread(void* arg);//（线程）实时准备报警视频帧
		void handleGenerateAlarm();
	public:
		bool start(std::string& msg);

		bool getState();
		void remove();
	public:
		Control* mControl;
		Scheduler* mScheduler;
		AvPullStream* mPullStream;
		AvPushStream* mPushStream;
		Analyzer*  mAnalyzer;
		FramePool* mVideoFramePool;

	private:
		bool mState = false;
		std::vector<std::thread*> mThreads;


		//报警视频帧
		std::queue <Frame*> mAlarmVideoFrameQ;
		std::mutex          mAlarmVideoFrameQ_mtx;
		void addAlarmVideoFrameQ(Frame* frame);
		int  getAlarmVideoFrameQSize();
		bool getAlarmVideoFrame(Frame*& frame);
		void clearAlarmVideoFrameQ();

	};
}
#endif //ANALYZER_WORKER_H